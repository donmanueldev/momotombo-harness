#pragma once

#include "domain/connections/entities/connection_profile.h"
#include "domain/connections/entities/connection_test_result.h"

namespace redisforge::domain::connections
{
    class IRedisConnectionGateway
    {
    public:
        virtual ~IRedisConnectionGateway() = default;

        virtual ConnectionTestResult testConnection(const ConnectionProfile& profile) const = 0;
    };
} // namespace redisforge::domain::connections
