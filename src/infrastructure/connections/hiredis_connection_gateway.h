#pragma once

#include "domain/connections/services/i_redis_connection_gateway.h"

namespace redisforge::infrastructure::connections
{
    class HiredisConnectionGateway final : public domain::connections::IRedisConnectionGateway
    {
    public:
        [[nodiscard]] domain::connections::ConnectionTestResult testConnection(
            const domain::connections::ConnectionProfile& profile) const override;
    };
} // namespace redisforge::infrastructure::connections
