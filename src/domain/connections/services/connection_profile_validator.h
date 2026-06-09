#pragma once

#include <string>
#include <vector>

#include "domain/connections/entities/connection_profile.h"

namespace redisforge::domain::connections
{
class ConnectionProfileValidator
{
public:
    [[nodiscard]] std::vector<std::string> validate(const ConnectionProfile& profile) const;
};
} // namespace redisforge::domain::connections
