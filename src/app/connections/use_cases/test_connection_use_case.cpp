#include "app/connections/use_cases/test_connection_use_case.h"

#include <exception>
#include <sstream>

namespace redisforge::app::connections
{
TestConnectionUseCase::TestConnectionUseCase(
    const domain::connections::IRedisConnectionGateway& gateway,
    shared::logging::ILogger& logger)
    : gateway_(gateway), logger_(logger)
{
}

domain::connections::ConnectionTestResult TestConnectionUseCase::execute(
    const domain::connections::ConnectionProfile& profile) const
{
    const auto validationErrors = validator_.validate(profile);
    if (!validationErrors.empty())
    {
        auto messageBuilder = std::ostringstream{};
        for (std::size_t index = 0; index < validationErrors.size(); ++index)
        {
            if (index > 0)
            {
                messageBuilder << ' ';
            }
            messageBuilder << validationErrors[index];
        }

        logger_.warning("connections.test.validation_failed", {
            {"name", profile.name},
            {"host", profile.host},
        });

        return domain::connections::ConnectionTestResult{
            false,
            messageBuilder.str(),
            std::chrono::milliseconds{0},
            {},
        };
    }

    logger_.info("connections.test.started", {
        {"name", profile.name},
        {"host", profile.host},
        {"port", std::to_string(profile.port)},
    });

    auto result = domain::connections::ConnectionTestResult{};
    try
    {
        result = gateway_.testConnection(profile);
    }
    catch (const std::exception& exception)
    {
        result = domain::connections::ConnectionTestResult{
            false,
            exception.what(),
            std::chrono::milliseconds{0},
            {},
        };
    }

    logger_.info(
        result.isSuccessful ? "connections.test.succeeded" : "connections.test.failed",
        {
            {"name", profile.name},
            {"host", profile.host},
            {"latencyMs", std::to_string(result.latency.count())},
        });

    return result;
}
} // namespace redisforge::app::connections
