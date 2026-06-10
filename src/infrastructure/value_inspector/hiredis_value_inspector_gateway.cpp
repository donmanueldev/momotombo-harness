#include "infrastructure/value_inspector/hiredis_value_inspector_gateway.h"

#include <hiredis/hiredis.h>

#include <memory>
#include <stdexcept>
#include <string>

#include "infrastructure/connections/redis_connection_session.h"

namespace
{
using RedisReplyHandle = std::unique_ptr<redisReply, decltype(&freeReplyObject)>;
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

    auto typeReply = RedisReplyHandle{
        static_cast<redisReply*>(redisCommand(session.context(), "TYPE %b", key.data(), key.size())),
        &freeReplyObject,
    };

    if (typeReply == nullptr)
    {
        throw std::runtime_error("TYPE failed: no reply received.");
    }

    if (typeReply->type == REDIS_REPLY_ERROR)
    {
        throw std::runtime_error(
            std::string{"TYPE failed: "} + (typeReply->str != nullptr ? typeReply->str : "Unknown TYPE error."));
    }

    if (typeReply->type != REDIS_REPLY_STATUS && typeReply->type != REDIS_REPLY_STRING)
    {
        throw std::runtime_error("TYPE failed: unexpected reply shape.");
    }

    auto result = redisforge::domain::value_inspector::KeyInspectionResult{};
    result.key = key;
    result.type = typeReply->str != nullptr ? typeReply->str : "unknown";

    if (result.type != "string")
    {
        return result;
    }

    auto valueReply = RedisReplyHandle{
        static_cast<redisReply*>(redisCommand(session.context(), "GET %b", key.data(), key.size())),
        &freeReplyObject,
    };

    if (valueReply == nullptr)
    {
        throw std::runtime_error("GET failed: no reply received.");
    }

    if (valueReply->type == REDIS_REPLY_ERROR)
    {
        throw std::runtime_error(
            std::string{"GET failed: "} + (valueReply->str != nullptr ? valueReply->str : "Unknown GET error."));
    }

    if (valueReply->type == REDIS_REPLY_NIL)
    {
        return result;
    }

    if (valueReply->type != REDIS_REPLY_STRING)
    {
        throw std::runtime_error("GET failed: unexpected reply shape.");
    }

    result.stringValue = valueReply->str != nullptr ? std::string{valueReply->str, valueReply->len} : "";
    result.hasStringValue = true;

    return result;
}
} // namespace redisforge::infrastructure::value_inspector
