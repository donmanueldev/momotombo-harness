#pragma once

#include "domain/connections/entities/connection_profile.h"
#include "domain/connections/entities/connection_test_result.h"
#include "domain/connections/services/connection_profile_validator.h"
#include "domain/connections/services/i_redis_connection_gateway.h"
#include "shared/logging/logger.h"

namespace redisforge::app::connections
{
class TestConnectionUseCase
{
public:
    TestConnectionUseCase(
        const domain::connections::IRedisConnectionGateway& gateway,
        shared::logging::ILogger& logger);

    [[nodiscard]] domain::connections::ConnectionTestResult execute(
        const domain::connections::ConnectionProfile& profile) const;

private:
    const domain::connections::IRedisConnectionGateway& gateway_;
    shared::logging::ILogger& logger_;
    domain::connections::ConnectionProfileValidator validator_;
};
} // namespace redisforge::app::connections
