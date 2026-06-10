#pragma once

#include <mutex>
#include <optional>

#include "domain/connections/entities/connection_profile.h"

namespace redisforge::app::connections
{
class ActiveConnectionStore
{
public:
    void setActiveConnection(redisforge::domain::connections::ConnectionProfile profile);
    void clear();
    [[nodiscard]] bool hasActiveConnection() const;
    [[nodiscard]] std::optional<redisforge::domain::connections::ConnectionProfile> activeConnection() const;

private:
    mutable std::mutex mutex_;
    std::optional<redisforge::domain::connections::ConnectionProfile> activeConnection_;
};
} // namespace redisforge::app::connections
