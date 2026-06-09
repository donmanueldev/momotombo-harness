#include "app/connections/use_cases/list_connection_profiles_use_case.h"

#include <algorithm>

namespace redisforge::app::connections
{
ListConnectionProfilesUseCase::ListConnectionProfilesUseCase(
    const domain::connections::IConnectionProfileRepository& repository,
    shared::logging::ILogger& logger)
    : repository_(repository), logger_(logger)
{
}

shared::pagination::Page<domain::connections::ConnectionProfile> ListConnectionProfilesUseCase::execute(
    const std::size_t page,
    const std::size_t pageSize) const
{
    const auto sanitizedPage = std::max<std::size_t>(1, page);
    const auto sanitizedPageSize = std::max<std::size_t>(1, pageSize);
    const auto offset = (sanitizedPage - 1U) * sanitizedPageSize;
    const auto totalCount = repository_.count();

    logger_.info("connections.profiles.listed", {
        {"page", std::to_string(sanitizedPage)},
        {"pageSize", std::to_string(sanitizedPageSize)},
        {"totalCount", std::to_string(totalCount)},
    });

    return shared::pagination::Page<domain::connections::ConnectionProfile>{
        repository_.list(offset, sanitizedPageSize),
        sanitizedPage,
        sanitizedPageSize,
        totalCount,
    };
}
} // namespace redisforge::app::connections
