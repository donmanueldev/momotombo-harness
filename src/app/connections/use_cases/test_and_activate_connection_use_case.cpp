#include "app/connections/use_cases/test_and_activate_connection_use_case.h"

#include <sstream>

namespace redisforge::app::connections
{
TestAndActivateConnectionUseCase::TestAndActivateConnectionUseCase(
    const redisforge::domain::connections::IRedisConnectionGateway& gateway,
    ActiveConnectionStore& activeConnectionStore,
    redisforge::shared::logging::ILogger& logger)
    : gateway_(gateway), activeConnectionStore_(activeConnectionStore), logger_(logger)
{
}

TestAndActivateConnectionResult TestAndActivateConnectionUseCase::execute(
    const redisforge::domain::connections::ConnectionProfile& profile) const
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

        return TestAndActivateConnectionResult{
            redisforge::domain::connections::ConnectionTestResult{
                false,
                messageBuilder.str(),
                std::chrono::milliseconds{0},
                {},
            },
            std::nullopt,
        };
    }

    logger_.info("connections.test.started", {
        {"name", profile.name},
        {"host", profile.host},
        {"port", std::to_string(profile.port)},
    });

    auto result = redisforge::domain::connections::ConnectionTestResult{};
    try
    {
        result = gateway_.testConnection(profile);
    }
    catch (const std::exception& exception)
    {
        result = redisforge::domain::connections::ConnectionTestResult{
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

    if (!result.isSuccessful)
    {
        return TestAndActivateConnectionResult{result, std::nullopt};
    }

    activeConnectionStore_.setActiveConnection(profile);
    auto summary = buildConnectionSummary(profile);
    logger_.info("connections.active.updated", {
        {"name", profile.name},
        {"host", profile.host},
        {"database", std::to_string(profile.databaseIndex)},
    });

    return TestAndActivateConnectionResult{
        result,
        ActivatedConnection{profile, std::move(summary)},
    };
}

std::string TestAndActivateConnectionUseCase::buildConnectionSummary(
    const redisforge::domain::connections::ConnectionProfile& profile)
{
    auto summary = std::ostringstream{};
    summary << profile.name << " (" << profile.host << ':' << profile.port << " / db" << profile.databaseIndex << ')';
    return summary.str();
}
} // namespace redisforge::app::connections
