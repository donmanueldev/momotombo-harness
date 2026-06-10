#include "infrastructure/value_inspector/hiredis_value_inspector_gateway.h"

#include <hiredis/hiredis.h>

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "infrastructure/connections/redis_connection_session.h"

namespace
{
    using RedisReplyHandle = std::unique_ptr<redisReply, decltype(&freeReplyObject)>;

    [[nodiscard]] RedisReplyHandle command(redisContext* context, const char* format, const std::string& key)
    {
        return RedisReplyHandle{
            static_cast<redisReply*>(redisCommand(context, format, key.data(), key.size())),
            &freeReplyObject,
        };
    }

    [[nodiscard]] RedisReplyHandle commandArgv(redisContext* context, const std::vector<std::string>& arguments)
    {
        auto argv = std::vector<const char*>{};
        auto argvlen = std::vector<std::size_t>{};
        argv.reserve(arguments.size());
        argvlen.reserve(arguments.size());

        for (const auto& argument : arguments)
        {
            argv.push_back(argument.data());
            argvlen.push_back(argument.size());
        }

        return RedisReplyHandle{
            static_cast<redisReply*>(
                redisCommandArgv(context, static_cast<int>(argv.size()), argv.data(), argvlen.data())),
            &freeReplyObject,
        };
    }

    [[nodiscard]] std::string readString(const redisReply* reply)
    {
        if (reply->str == nullptr)
        {
            return {};
        }

        return std::string{reply->str, reply->len};
    }

    void ensureReply(const RedisReplyHandle& reply, const std::string& operation)
    {
        if (reply == nullptr)
        {
            throw std::runtime_error(operation + " failed: no reply received.");
        }

        if (reply->type == REDIS_REPLY_ERROR)
        {
            throw std::runtime_error(
                operation + " failed: " + (reply->str != nullptr ? std::string{reply->str} : "Unknown Redis error."));
        }
    }

    [[nodiscard]] redisforge::domain::value_inspector::RedisValueType parseType(const std::string& type)
    {
        if (type == "none")
        {
            return redisforge::domain::value_inspector::RedisValueType::None;
        }

        if (type == "string")
        {
            return redisforge::domain::value_inspector::RedisValueType::String;
        }

        if (type == "hash")
        {
            return redisforge::domain::value_inspector::RedisValueType::Hash;
        }

        if (type == "list")
        {
            return redisforge::domain::value_inspector::RedisValueType::List;
        }

        if (type == "set")
        {
            return redisforge::domain::value_inspector::RedisValueType::Set;
        }

        if (type == "zset")
        {
            return redisforge::domain::value_inspector::RedisValueType::SortedSet;
        }

        if (type == "stream")
        {
            return redisforge::domain::value_inspector::RedisValueType::Stream;
        }

        return redisforge::domain::value_inspector::RedisValueType::Unsupported;
    }

    void inspectString(
        redisContext* context,
        const std::string& key,
        const redisforge::domain::value_inspector::ValueInspectionOptions& options,
        redisforge::domain::value_inspector::KeyInspectionResult& result)
    {
        auto lengthReply = command(context, "STRLEN %b", key);
        ensureReply(lengthReply, "STRLEN");
        if (lengthReply->type != REDIS_REPLY_INTEGER)
        {
            throw std::runtime_error("STRLEN failed: unexpected reply shape.");
        }

        const auto stringLength = static_cast<std::size_t>(lengthReply->integer);
        if (options.stringByteLimit == 0)
        {
            result.isStringTruncated = stringLength > 0;
            return;
        }

        const auto previewEnd = options.stringByteLimit - 1;
        auto valueReply =
            commandArgv(context, {"GETRANGE", key, "0", std::to_string(previewEnd)});
        ensureReply(valueReply, "GETRANGE");

        if (valueReply->type == REDIS_REPLY_NIL)
        {
            return;
        }

        if (valueReply->type != REDIS_REPLY_STRING)
        {
            throw std::runtime_error("GETRANGE failed: unexpected reply shape.");
        }

        result.stringValue = readString(valueReply.get());
        result.isStringTruncated = stringLength > options.stringByteLimit;
    }

    void inspectHash(
        redisContext* context,
        const std::string& key,
        const redisforge::domain::value_inspector::ValueInspectionOptions& options,
        redisforge::domain::value_inspector::KeyInspectionResult& result)
    {
        auto totalReply = command(context, "HLEN %b", key);
        ensureReply(totalReply, "HLEN");
        if (totalReply->type != REDIS_REPLY_INTEGER)
        {
            throw std::runtime_error("HLEN failed: unexpected reply shape.");
        }

        auto reply =
            commandArgv(context, {"HSCAN", key, "0", "COUNT", std::to_string(options.collectionEntryLimit)});
        ensureReply(reply, "HSCAN");
        if (reply->type != REDIS_REPLY_ARRAY || reply->elements != 2 || reply->element[1]->type != REDIS_REPLY_ARRAY)
        {
            throw std::runtime_error("HSCAN failed: unexpected reply shape.");
        }

        const auto* entriesReply = reply->element[1];
        result.hashEntries.clear();
        result.hashEntries.reserve(entriesReply->elements / 2);
        for (std::size_t index = 0; index + 1 < entriesReply->elements; index += 2)
        {
            result.hashEntries.push_back({
                readString(entriesReply->element[index]),
                readString(entriesReply->element[index + 1]),
            });
        }
        result.loadedEntryCount = result.hashEntries.size();
        result.hasAdditionalEntries = static_cast<std::size_t>(totalReply->integer) > result.loadedEntryCount
            || readString(reply->element[0]) != "0";
    }

    void inspectList(
        redisContext* context,
        const std::string& key,
        const redisforge::domain::value_inspector::ValueInspectionOptions& options,
        redisforge::domain::value_inspector::KeyInspectionResult& result)
    {
        auto totalReply = command(context, "LLEN %b", key);
        ensureReply(totalReply, "LLEN");
        if (totalReply->type != REDIS_REPLY_INTEGER)
        {
            throw std::runtime_error("LLEN failed: unexpected reply shape.");
        }

        const auto previewEnd =
            options.collectionEntryLimit == 0 ? static_cast<std::size_t>(0) : options.collectionEntryLimit - 1;
        auto reply = commandArgv(context, {"LRANGE", key, "0", std::to_string(previewEnd)});
        ensureReply(reply, "LRANGE");

        if (reply->type != REDIS_REPLY_ARRAY)
        {
            throw std::runtime_error("LRANGE failed: unexpected reply shape.");
        }

        result.listEntries.clear();
        result.listEntries.reserve(reply->elements);
        for (std::size_t index = 0; index < reply->elements; ++index)
        {
            result.listEntries.push_back({index, readString(reply->element[index])});
        }
        result.loadedEntryCount = result.listEntries.size();
        result.hasAdditionalEntries = static_cast<std::size_t>(totalReply->integer) > result.loadedEntryCount;
    }

    void inspectSet(
        redisContext* context,
        const std::string& key,
        const redisforge::domain::value_inspector::ValueInspectionOptions& options,
        redisforge::domain::value_inspector::KeyInspectionResult& result)
    {
        auto reply =
            commandArgv(context, {"SSCAN", key, "0", "COUNT", std::to_string(options.collectionEntryLimit)});
        ensureReply(reply, "SSCAN");
        if (reply->type != REDIS_REPLY_ARRAY || reply->elements != 2 || reply->element[1]->type != REDIS_REPLY_ARRAY)
        {
            throw std::runtime_error("SSCAN failed: unexpected reply shape.");
        }

        const auto* entriesReply = reply->element[1];
        result.setMembers.clear();
        result.setMembers.reserve(entriesReply->elements);
        for (std::size_t index = 0; index < entriesReply->elements; ++index)
        {
            result.setMembers.push_back(readString(entriesReply->element[index]));
        }
        result.loadedEntryCount = result.setMembers.size();
        result.hasAdditionalEntries = readString(reply->element[0]) != "0";
    }

    void inspectSortedSet(
        redisContext* context,
        const std::string& key,
        const redisforge::domain::value_inspector::ValueInspectionOptions& options,
        redisforge::domain::value_inspector::KeyInspectionResult& result)
    {
        auto reply =
            commandArgv(context, {"ZSCAN", key, "0", "COUNT", std::to_string(options.collectionEntryLimit)});
        ensureReply(reply, "ZSCAN");
        if (reply->type != REDIS_REPLY_ARRAY || reply->elements != 2 || reply->element[1]->type != REDIS_REPLY_ARRAY)
        {
            throw std::runtime_error("ZSCAN failed: unexpected reply shape.");
        }

        const auto* entriesReply = reply->element[1];
        result.sortedSetEntries.clear();
        result.sortedSetEntries.reserve(entriesReply->elements / 2);
        for (std::size_t index = 0; index + 1 < entriesReply->elements; index += 2)
        {
            const auto scoreText = readString(entriesReply->element[index + 1]);
            result.sortedSetEntries.push_back({
                readString(entriesReply->element[index]),
                std::strtod(scoreText.c_str(), nullptr),
                scoreText,
            });
        }
        result.loadedEntryCount = result.sortedSetEntries.size();
        result.hasAdditionalEntries = readString(reply->element[0]) != "0";
    }

    std::vector<redisforge::domain::value_inspector::HashEntry> parseStreamFields(const redisReply* fieldsReply)
    {
        if (fieldsReply->type != REDIS_REPLY_ARRAY)
        {
            throw std::runtime_error("XRANGE failed: unexpected field collection shape.");
        }

        auto fields = std::vector<redisforge::domain::value_inspector::HashEntry>{};
        fields.reserve(fieldsReply->elements / 2);
        for (std::size_t index = 0; index + 1 < fieldsReply->elements; index += 2)
        {
            fields.push_back({
                readString(fieldsReply->element[index]),
                readString(fieldsReply->element[index + 1]),
            });
        }

        return fields;
    }

    void inspectStream(
        redisContext* context,
        const std::string& key,
        const redisforge::domain::value_inspector::ValueInspectionOptions& options,
        redisforge::domain::value_inspector::KeyInspectionResult& result)
    {
        auto reply =
            commandArgv(context, {"XRANGE", key, "-", "+", "COUNT", std::to_string(options.collectionEntryLimit)});
        ensureReply(reply, "XRANGE");

        if (reply->type != REDIS_REPLY_ARRAY)
        {
            throw std::runtime_error("XRANGE failed: unexpected reply shape.");
        }

        result.streamEntries.clear();
        result.streamEntries.reserve(reply->elements);
        for (std::size_t index = 0; index < reply->elements; ++index)
        {
            const auto* entryReply = reply->element[index];
            if (entryReply->type != REDIS_REPLY_ARRAY || entryReply->elements != 2)
            {
                throw std::runtime_error("XRANGE failed: unexpected stream entry shape.");
            }

            result.streamEntries.push_back({
                readString(entryReply->element[0]),
                parseStreamFields(entryReply->element[1]),
            });
        }
        result.loadedEntryCount = result.streamEntries.size();
        result.hasAdditionalEntries = result.streamEntries.size() >= options.collectionEntryLimit;
    }
}

namespace redisforge::infrastructure::value_inspector
{
    void HiredisValueInspectorGateway::saveStringValue(
        const domain::connections::ConnectionProfile& profile,
        const std::string& key,
        const std::string& value) const
    {
        auto session = connections::RedisConnectionSession{profile};
        if (!session.isOpen())
        {
            throw std::runtime_error(session.errorMessage());
        }

        auto reply = commandArgv(session.context(), {"SET", key, value});
        ensureReply(reply, "SET");
        if (reply->type != REDIS_REPLY_STATUS)
        {
            throw std::runtime_error("SET failed: unexpected reply shape.");
        }
    }

    void HiredisValueInspectorGateway::saveHashField(
        const domain::connections::ConnectionProfile& profile,
        const std::string& key,
        const std::string& field,
        const std::string& value) const
    {
        auto session = connections::RedisConnectionSession{profile};
        if (!session.isOpen())
        {
            throw std::runtime_error(session.errorMessage());
        }

        auto reply = commandArgv(session.context(), {"HSET", key, field, value});
        ensureReply(reply, "HSET");
        if (reply->type != REDIS_REPLY_INTEGER)
        {
            throw std::runtime_error("HSET failed: unexpected reply shape.");
        }
    }

    domain::value_inspector::KeyInspectionResult HiredisValueInspectorGateway::inspectKey(
        const domain::connections::ConnectionProfile& profile,
        const std::string& key,
        const domain::value_inspector::ValueInspectionOptions& options) const
    {
        auto session = connections::RedisConnectionSession{profile};
        if (!session.isOpen())
        {
            throw std::runtime_error(session.errorMessage());
        }

        auto typeReply = command(session.context(), "TYPE %b", key);
        ensureReply(typeReply, "TYPE");

        if (typeReply->type != REDIS_REPLY_STATUS && typeReply->type != REDIS_REPLY_STRING)
        {
            throw std::runtime_error("TYPE failed: unexpected reply shape.");
        }

        auto result = domain::value_inspector::KeyInspectionResult{};
        result.key = key;
        result.type = parseType(readString(typeReply.get()));

        switch (result.type)
        {
        case domain::value_inspector::RedisValueType::String:
            inspectString(session.context(), key, options, result);
            break;
        case domain::value_inspector::RedisValueType::Hash:
            inspectHash(session.context(), key, options, result);
            break;
        case domain::value_inspector::RedisValueType::List:
            inspectList(session.context(), key, options, result);
            break;
        case domain::value_inspector::RedisValueType::Set:
            inspectSet(session.context(), key, options, result);
            break;
        case domain::value_inspector::RedisValueType::SortedSet:
            inspectSortedSet(session.context(), key, options, result);
            break;
        case domain::value_inspector::RedisValueType::Stream:
            inspectStream(session.context(), key, options, result);
            break;
        case domain::value_inspector::RedisValueType::None:
        case domain::value_inspector::RedisValueType::Unsupported:
            break;
        }

        return result;
    }
} // namespace redisforge::infrastructure::value_inspector
