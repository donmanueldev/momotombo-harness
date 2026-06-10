#include "infrastructure/connections/hiredis_connection_gateway.h"

#include <hiredis/hiredis.h>

#include <chrono>
#include <memory>
#include <sstream>
#include <string_view>

#include "infrastructure/connections/redis_connection_session.h"

namespace
{
    using RedisReplyHandle = std::unique_ptr<redisReply, decltype(&freeReplyObject)>;

    [[nodiscard]] std::string parseServerVersion(std::string_view infoResponse)
    {
        constexpr auto prefix = std::string_view{"redis_version:"};
        const auto start = infoResponse.find(prefix);
        if (start == std::string_view::npos)
        {
            return {};
        }

        const auto versionStart = start + prefix.size();
        const auto versionEnd = infoResponse.find('\n', versionStart);
        return std::string{infoResponse.substr(versionStart, versionEnd - versionStart)};
    }
} // namespace

namespace redisforge::infrastructure::connections
{
    domain::connections::ConnectionTestResult HiredisConnectionGateway::testConnection(
        const domain::connections::ConnectionProfile& profile) const
    {
        const auto startedAt = std::chrono::steady_clock::now();
        auto session = RedisConnectionSession{profile};

        if (!session.isOpen())
        {
            return domain::connections::ConnectionTestResult{
                false,
                session.errorMessage(),
                std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - startedAt),
                {},
            };
        }

        auto pingReply = RedisReplyHandle{
            static_cast<redisReply*>(redisCommand(session.context(), "PING")),
            &freeReplyObject,
        };

        if (pingReply == nullptr || pingReply->type == REDIS_REPLY_ERROR)
        {
            return domain::connections::ConnectionTestResult{
                false,
                (pingReply != nullptr && pingReply->str != nullptr) ? pingReply->str : "PING command failed.",
                std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - startedAt),
                {},
            };
        }

        auto infoReply = RedisReplyHandle{
            static_cast<redisReply*>(redisCommand(session.context(), "INFO server")),
            &freeReplyObject,
        };

        const auto latency =
            std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - startedAt);
        const auto serverVersion =
            (infoReply != nullptr && infoReply->type == REDIS_REPLY_STRING) ? parseServerVersion(infoReply->str) : "";

        auto messageBuilder = std::ostringstream{};
        messageBuilder << "Connection successful. PING latency " << latency.count() << " ms.";
        if (!serverVersion.empty())
        {
            messageBuilder << " Server version " << serverVersion << '.';
        }

        return domain::connections::ConnectionTestResult{
            true,
            messageBuilder.str(),
            latency,
            serverVersion,
        };
    }
} // namespace redisforge::infrastructure::connections
