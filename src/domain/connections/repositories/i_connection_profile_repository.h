#pragma once

#include <cstddef>
#include <optional>
#include <vector>

#include "domain/connections/entities/connection_profile.h"

namespace redisforge::domain::connections
{
    class IConnectionProfileRepository
    {
    public:
        virtual ~IConnectionProfileRepository() = default;

        virtual void save(ConnectionProfile profile) = 0;
        virtual std::optional<ConnectionProfile> findById(const std::string& id) const = 0;
        virtual std::vector<ConnectionProfile> list(std::size_t offset, std::size_t limit) const = 0;
        virtual std::size_t count() const = 0;
    };
} // namespace redisforge::domain::connections
