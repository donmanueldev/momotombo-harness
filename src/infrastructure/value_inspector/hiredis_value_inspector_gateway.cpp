#include "infrastructure/value_inspector/hiredis_value_inspector_gateway.h"

#include <hiredis/hiredis.h>

#include <cstdlib>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

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
    redisforge::domain::value_inspector::KeyInspectionResult& result)
{
    auto valueReply = command(context, "GET %b", key);
    ensureReply(valueReply, "GET");

    if (valueReply->type == REDIS_REPLY_NIL)
    {
        return;
    }

    if (valueReply->type != REDIS_REPLY_STRING)
    {
        throw std::runtime_error("GET failed: unexpected reply shape.");
    }

    result.stringValue = readString(valueReply.get());
}

void inspectHash(
    redisContext* context,
    const std::string& key,
    redisforge::domain::value_inspector::KeyInspectionResult& result)
{
    auto reply = command(context, "HGETALL %b", key);
    ensureReply(reply, "HGETALL");

    if (reply->type != REDIS_REPLY_ARRAY)
    {
        throw std::runtime_error("HGETALL failed: unexpected reply shape.");
    }

    result.hashEntries.clear();
    result.hashEntries.reserve(static_cast<std::size_t>(reply->elements / 2));
    for (std::size_t index = 0; index + 1 < reply->elements; index += 2)
    {
        result.hashEntries.push_back({
            readString(reply->element[index]),
            readString(reply->element[index + 1]),
        });
    }
}

void inspectList(
    redisContext* context,
    const std::string& key,
    redisforge::domain::value_inspector::KeyInspectionResult& result)
{
    auto reply = command(context, "LRANGE %b 0 -1", key);
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
}

void inspectSet(
    redisContext* context,
    const std::string& key,
    redisforge::domain::value_inspector::KeyInspectionResult& result)
{
    auto reply = command(context, "SMEMBERS %b", key);
    ensureReply(reply, "SMEMBERS");

    if (reply->type != REDIS_REPLY_ARRAY)
    {
        throw std::runtime_error("SMEMBERS failed: unexpected reply shape.");
    }

    result.setMembers.clear();
    result.setMembers.reserve(reply->elements);
    for (std::size_t index = 0; index < reply->elements; ++index)
    {
        result.setMembers.push_back(readString(reply->element[index]));
    }
}

void inspectSortedSet(
    redisContext* context,
    const std::string& key,
    redisforge::domain::value_inspector::KeyInspectionResult& result)
{
    auto reply = command(context, "ZRANGE %b 0 -1 WITHSCORES", key);
    ensureReply(reply, "ZRANGE");

    if (reply->type != REDIS_REPLY_ARRAY)
    {
        throw std::runtime_error("ZRANGE failed: unexpected reply shape.");
    }

    result.sortedSetEntries.clear();
    result.sortedSetEntries.reserve(static_cast<std::size_t>(reply->elements / 2));
    for (std::size_t index = 0; index + 1 < reply->elements; index += 2)
    {
        const auto scoreText = readString(reply->element[index + 1]);
        result.sortedSetEntries.push_back({
            readString(reply->element[index]),
            std::strtod(scoreText.c_str(), nullptr),
            scoreText,
        });
    }
}

std::vector<redisforge::domain::value_inspector::HashEntry> parseStreamFields(const redisReply* fieldsReply)
{
    if (fieldsReply->type != REDIS_REPLY_ARRAY)
    {
        throw std::runtime_error("XRANGE failed: unexpected field collection shape.");
    }

    auto fields = std::vector<redisforge::domain::value_inspector::HashEntry>{};
    fields.reserve(static_cast<std::size_t>(fieldsReply->elements / 2));
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
    redisforge::domain::value_inspector::KeyInspectionResult& result)
{
    auto reply = command(context, "XRANGE %b - +", key);
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
}
}

namespace redisforge::infrastructure::value_inspector
{
redisforge::domain::value_inspector::KeyInspectionResult HiredisValueInspectorGateway::inspectKey(
    const redisforge::domain::connections::ConnectionProfile& profile,
    const std::string& key) const
{
    auto session = redisforge::infrastructure::connections::RedisConnectionSession{profile};
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

    auto result = redisforge::domain::value_inspector::KeyInspectionResult{};
    result.key = key;
    result.type = parseType(readString(typeReply.get()));

    switch (result.type)
    {
    case redisforge::domain::value_inspector::RedisValueType::String:
        inspectString(session.context(), key, result);
        break;
    case redisforge::domain::value_inspector::RedisValueType::Hash:
        inspectHash(session.context(), key, result);
        break;
    case redisforge::domain::value_inspector::RedisValueType::List:
        inspectList(session.context(), key, result);
        break;
    case redisforge::domain::value_inspector::RedisValueType::Set:
        inspectSet(session.context(), key, result);
        break;
    case redisforge::domain::value_inspector::RedisValueType::SortedSet:
        inspectSortedSet(session.context(), key, result);
        break;
    case redisforge::domain::value_inspector::RedisValueType::Stream:
        inspectStream(session.context(), key, result);
        break;
    case domain::value_inspector::RedisValueType::None:
    case domain::value_inspector::RedisValueType::Unsupported:
        break;
    }

    return result;
}
} // namespace redisforge::infrastructure::value_inspector
