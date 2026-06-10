#include "domain/connections/services/connection_profile_validator.h"

namespace redisforge::domain::connections
{
    std::vector<std::string> ConnectionProfileValidator::validate(const ConnectionProfile& profile) const
    {
        auto errors = std::vector<std::string>{};

        if (profile.name.empty())
        {
            errors.emplace_back("Connection name is required.");
        }

        if (profile.host.empty())
        {
            errors.emplace_back("Host is required.");
        }

        if (profile.port == 0)
        {
            errors.emplace_back("Port must be between 1 and 65535.");
        }

        if (profile.connectTimeoutMs <= 0)
        {
            errors.emplace_back("Connection timeout must be greater than zero.");
        }

        if (profile.databaseIndex < 0)
        {
            errors.emplace_back("Database index cannot be negative.");
        }

        return errors;
    }
} // namespace redisforge::domain::connections
