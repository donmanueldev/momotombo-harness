#include "infrastructure/connections/in_memory_connection_profile_repository.h"

#include <algorithm>

namespace redisforge::infrastructure::connections
{
void InMemoryConnectionProfileRepository::save(domain::connections::ConnectionProfile profile)
{
    auto lock = std::scoped_lock{mutex_};
    const auto existing = profilesById_.find(profile.id);

    if (existing == profilesById_.end())
    {
        orderedIds_.push_back(profile.id);
    }

    profilesById_[profile.id] = std::move(profile);
}

std::optional<domain::connections::ConnectionProfile> InMemoryConnectionProfileRepository::findById(
    const std::string& id) const
{
    auto lock = std::scoped_lock{mutex_};
    const auto iterator = profilesById_.find(id);
    if (iterator == profilesById_.end())
    {
        return std::nullopt;
    }

    return iterator->second;
}

std::vector<domain::connections::ConnectionProfile> InMemoryConnectionProfileRepository::list(
    const std::size_t offset,
    const std::size_t limit) const
{
    auto lock = std::scoped_lock{mutex_};
    auto page = std::vector<domain::connections::ConnectionProfile>{};

    if (offset >= orderedIds_.size())
    {
        return page;
    }

    const auto endIndex = std::min(orderedIds_.size(), offset + limit);
    page.reserve(endIndex - offset);

    for (auto index = offset; index < endIndex; ++index)
    {
        page.push_back(profilesById_.at(orderedIds_[index]));
    }

    return page;
}

std::size_t InMemoryConnectionProfileRepository::count() const
{
    auto lock = std::scoped_lock{mutex_};
    return orderedIds_.size();
}
} // namespace redisforge::infrastructure::connections
