#pragma once

#include <mutex>
#include <unordered_map>
#include <vector>

#include "domain/connections/repositories/i_connection_profile_repository.h"

namespace redisforge::infrastructure::connections
{
class InMemoryConnectionProfileRepository final : public domain::connections::IConnectionProfileRepository
{
public:
    void save(domain::connections::ConnectionProfile profile) override;
    [[nodiscard]] std::optional<domain::connections::ConnectionProfile> findById(const std::string& id) const override;
    [[nodiscard]] std::vector<domain::connections::ConnectionProfile> list(
        std::size_t offset,
        std::size_t limit) const override;
    [[nodiscard]] std::size_t count() const override;

private:
    mutable std::mutex mutex_;
    std::vector<std::string> orderedIds_;
    std::unordered_map<std::string, domain::connections::ConnectionProfile> profilesById_;
};
} // namespace redisforge::infrastructure::connections
