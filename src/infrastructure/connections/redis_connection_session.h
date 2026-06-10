#pragma once

#include <hiredis/hiredis.h>

#include <memory>
#include <string>

#include "domain/connections/entities/connection_profile.h"

namespace redisforge::infrastructure::connections
{
    class RedisConnectionSession
    {
    public:
        using ContextHandle = std::unique_ptr<redisContext, decltype(&redisFree)>;

        explicit RedisConnectionSession(const domain::connections::ConnectionProfile& profile);

        [[nodiscard]] bool isOpen() const;
        [[nodiscard]] redisContext* context() const;
        [[nodiscard]] const std::string& errorMessage() const;

    private:
        static std::string authenticate(redisContext& context,
                                        const domain::connections::ConnectionProfile& profile);
        static std::string selectDatabase(redisContext& context,
                                          const domain::connections::ConnectionProfile& profile);

        ContextHandle context_{nullptr, &redisFree};
        std::string errorMessage_;
    };
} // namespace redisforge::infrastructure::connections
