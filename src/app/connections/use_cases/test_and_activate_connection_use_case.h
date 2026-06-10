#pragma once

#include "app/connections/services/active_connection_store.h"
#include "domain/connections/entities/connection_profile.h"
#include "domain/connections/entities/connection_test_result.h"
#include "domain/connections/services/connection_profile_validator.h"
#include "domain/connections/services/i_redis_connection_gateway.h"
#include "shared/logging/logger.h"

namespace redisforge::app::connections
{
struct ActivatedConnection
{
    redisforge::domain::connections::ConnectionProfile profile;
    std::string summary;
};

struct TestAndActivateConnectionResult
{
    redisforge::domain::connections::ConnectionTestResult testResult;
    std::optional<ActivatedConnection> activeConnection;
};

class TestAndActivateConnectionUseCase final
{
public:
    TestAndActivateConnectionUseCase(
        const redisforge::domain::connections::IRedisConnectionGateway& gateway,
        ActiveConnectionStore& activeConnectionStore,
        redisforge::shared::logging::ILogger& logger);

    [[nodiscard]] TestAndActivateConnectionResult execute(
        const redisforge::domain::connections::ConnectionProfile& profile) const;

private:
    [[nodiscard]] static std::string buildConnectionSummary(
        const redisforge::domain::connections::ConnectionProfile& profile);

    const redisforge::domain::connections::IRedisConnectionGateway& gateway_;
    ActiveConnectionStore& activeConnectionStore_;
    redisforge::shared::logging::ILogger& logger_;
    redisforge::domain::connections::ConnectionProfileValidator validator_;
};
} // namespace redisforge::app::connections
