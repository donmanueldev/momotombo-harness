#include "infrastructure/connections/redis_connection_session.h"

namespace
{
using RedisReplyHandle = std::unique_ptr<redisReply, decltype(&freeReplyObject)>;
}

namespace redisforge::infrastructure::connections
{
RedisConnectionSession::RedisConnectionSession(const redisforge::domain::connections::ConnectionProfile& profile)
{
    auto timeout = timeval{
        static_cast<long>(profile.connectTimeoutMs / 1000),
        static_cast<int>((profile.connectTimeoutMs % 1000) * 1000),
    };

    auto options = redisOptions{};
    REDIS_OPTIONS_SET_TCP(&options, profile.host.c_str(), profile.port);
    options.connect_timeout = &timeout;
    options.command_timeout = &timeout;

    context_ = ContextHandle{redisConnectWithOptions(&options), &redisFree};
    if (context_ == nullptr)
    {
        errorMessage_ = "Failed to create hiredis context.";
        return;
    }

    if (context_->err != 0)
    {
        errorMessage_ = context_->errstr[0] != '\0' ? context_->errstr : "Unknown Redis connection error.";
        return;
    }

    if (const auto authError = authenticate(*context_, profile); !authError.empty())
    {
        errorMessage_ = authError;
        return;
    }

    if (const auto selectError = selectDatabase(*context_, profile); !selectError.empty())
    {
        errorMessage_ = selectError;
        return;
    }
}

bool RedisConnectionSession::isOpen() const
{
    return context_ != nullptr && errorMessage_.empty();
}

redisContext* RedisConnectionSession::context() const
{
    return context_.get();
}

const std::string& RedisConnectionSession::errorMessage() const
{
    return errorMessage_;
}

std::string RedisConnectionSession::authenticate(
    redisContext& context,
    const redisforge::domain::connections::ConnectionProfile& profile)
{
    if (profile.password.empty())
    {
        return {};
    }

    void* rawReply = nullptr;
    if (profile.username.empty())
    {
        rawReply = redisCommand(&context, "AUTH %s", profile.password.c_str());
    }
    else
    {
        rawReply = redisCommand(&context, "AUTH %s %s", profile.username.c_str(), profile.password.c_str());
    }

    auto reply = RedisReplyHandle{static_cast<redisReply*>(rawReply), &freeReplyObject};
    if (reply == nullptr)
    {
        return "Authentication failed: no reply received.";
    }

    if (reply->type == REDIS_REPLY_ERROR)
    {
        return std::string{"Authentication failed: "}
            + (reply->str != nullptr ? std::string{reply->str} : std::string{"Unknown AUTH error."});
    }

    return {};
}

std::string RedisConnectionSession::selectDatabase(
    redisContext& context,
    const redisforge::domain::connections::ConnectionProfile& profile)
{
    if (profile.databaseIndex == 0)
    {
        return {};
    }

    auto reply = RedisReplyHandle{
        static_cast<redisReply*>(redisCommand(&context, "SELECT %d", profile.databaseIndex)),
        &freeReplyObject,
    };

    if (reply == nullptr)
    {
        return "Database selection failed: no reply received.";
    }

    if (reply->type == REDIS_REPLY_ERROR)
    {
        return std::string{"Database selection failed: "}
            + (reply->str != nullptr ? std::string{reply->str} : std::string{"Unknown SELECT error."});
    }

    return {};
}
} // namespace redisforge::infrastructure::connections
