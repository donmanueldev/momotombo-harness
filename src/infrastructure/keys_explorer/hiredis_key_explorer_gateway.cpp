#include "infrastructure/keys_explorer/hiredis_key_explorer_gateway.h"

#include <hiredis/hiredis.h>

#include <memory>
#include <stdexcept>

#include "infrastructure/connections/redis_connection_session.h"

namespace
{
using RedisReplyHandle = std::unique_ptr<redisReply, decltype(&freeReplyObject)>;
}

namespace redisforge::infrastructure::keys_explorer
{
redisforge::domain::keys_explorer::KeyScanPage HiredisKeyExplorerGateway::scanKeys(
    const redisforge::domain::connections::ConnectionProfile& profile,
    const std::string& cursor,
    const std::string& pattern,
    const std::size_t pageSize) const
{
    auto session = redisforge::infrastructure::connections::RedisConnectionSession{profile};
    if (!session.isOpen())
    {
        throw std::runtime_error(session.errorMessage());
    }

    const auto effectivePattern = pattern.empty() ? "*" : pattern;
    auto reply = RedisReplyHandle{
        static_cast<redisReply*>(redisCommand(
            session.context(),
            "SCAN %s MATCH %s COUNT %zu",
            cursor.c_str(),
            effectivePattern.c_str(),
            pageSize)),
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

    auto page = redisforge::domain::keys_explorer::KeyScanPage{};
    page.nextCursor = cursorReply->str != nullptr ? cursorReply->str : "0";
    page.hasMore = page.nextCursor != "0";
    page.keys.reserve(keysReply->elements);

    for (size_t index = 0; index < keysReply->elements; ++index)
    {
        const auto* keyReply = keysReply->element[index];
        if (keyReply != nullptr && keyReply->str != nullptr)
        {
            page.keys.emplace_back(keyReply->str);
        }
    }

    return page;
}
} // namespace redisforge::infrastructure::keys_explorer
