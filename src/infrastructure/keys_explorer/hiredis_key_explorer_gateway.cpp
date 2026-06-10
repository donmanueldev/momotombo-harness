#include "infrastructure/keys_explorer/hiredis_key_explorer_gateway.h"

#include <hiredis/hiredis.h>

#include <array>
#include <memory>
#include <stdexcept>
#include <string>

#include "infrastructure/connections/redis_connection_session.h"

namespace
{
    using RedisReplyHandle = std::unique_ptr<redisReply, decltype(&freeReplyObject)>;

    [[nodiscard]] std::string readReplyString(const redisReply* reply)
    {
        if (reply == nullptr || reply->str == nullptr)
        {
            return {};
        }

        return std::string{reply->str, reply->len};
    }
}

namespace redisforge::infrastructure::keys_explorer
{
    domain::keys_explorer::KeyScanPage HiredisKeyExplorerGateway::scanKeys(
        const domain::connections::ConnectionProfile& profile,
        const std::string& cursor,
        const std::string& pattern,
        const std::size_t pageSize) const
    {
        auto session = connections::RedisConnectionSession{profile};
        if (!session.isOpen())
        {
            throw std::runtime_error(session.errorMessage());
        }

        const auto effectivePattern = pattern.empty() ? "*" : pattern;
        const auto count = std::to_string(pageSize);
        const char* arguments[] = {
            "SCAN",
            cursor.c_str(),
            "MATCH",
            effectivePattern.c_str(),
            "COUNT",
            count.c_str(),
        };
        const std::array<std::size_t, 6> argumentLengths{
            4,
            cursor.size(),
            5,
            effectivePattern.size(),
            5,
            count.size(),
        };
        auto reply = RedisReplyHandle{
            static_cast<redisReply*>(
                redisCommandArgv(
                    session.context(),
                    std::size(arguments),
                    arguments,
                    argumentLengths.data())),
            &freeReplyObject,
        };

        if (reply == nullptr)
        {
            throw std::runtime_error("SCAN failed: no reply received.");
        }

        if (reply->type == REDIS_REPLY_ERROR)
        {
            throw std::runtime_error(
                std::string{"SCAN failed: "} + (reply->str != nullptr ? reply->str : "Unknown SCAN error."));
        }

        if (reply->type != REDIS_REPLY_ARRAY || reply->elements != 2)
        {
            throw std::runtime_error("SCAN failed: unexpected reply shape.");
        }

        const auto* cursorReply = reply->element[0];
        const auto* keysReply = reply->element[1];

        if (cursorReply == nullptr || keysReply == nullptr || cursorReply->type != REDIS_REPLY_STRING
            || keysReply->type != REDIS_REPLY_ARRAY)
        {
            throw std::runtime_error("SCAN failed: invalid cursor or keys payload.");
        }

        auto page = domain::keys_explorer::KeyScanPage{};
        page.nextCursor = cursorReply->str != nullptr ? cursorReply->str : "0";
        page.hasMore = page.nextCursor != "0";
        page.entries.reserve(keysReply->elements);

        for (size_t index = 0; index < keysReply->elements; ++index)
        {
            const auto* keyReply = keysReply->element[index];
            if (keyReply == nullptr || keyReply->str == nullptr)
            {
                continue;
            }

            const auto key = std::string{keyReply->str, keyReply->len};
            auto typeReply = RedisReplyHandle{
                static_cast<redisReply*>(redisCommand(session.context(), "TYPE %b", key.data(), key.size())),
                &freeReplyObject,
            };

            if (typeReply == nullptr)
            {
                throw std::runtime_error("TYPE failed: no reply received while classifying scanned keys.");
            }

            if (typeReply->type == REDIS_REPLY_ERROR)
            {
                throw std::runtime_error(
                    std::string{"TYPE failed: "}
                    + (typeReply->str != nullptr ? typeReply->str : "Unknown TYPE error."));
            }

            if (typeReply->type != REDIS_REPLY_STATUS && typeReply->type != REDIS_REPLY_STRING)
            {
                throw std::runtime_error("TYPE failed: unexpected reply shape while classifying scanned keys.");
            }

            page.entries.push_back({key, readReplyString(typeReply.get())});
        }

        return page;
    }
} // namespace redisforge::infrastructure::keys_explorer
