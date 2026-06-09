#pragma once

#include <cstddef>

#include "domain/connections/entities/connection_profile.h"
#include "domain/connections/repositories/i_connection_profile_repository.h"
#include "shared/logging/logger.h"
#include "shared/pagination/page.h"

namespace redisforge::app::connections
{
class ListConnectionProfilesUseCase
{
public:
    ListConnectionProfilesUseCase(
        const domain::connections::IConnectionProfileRepository& repository,
        shared::logging::ILogger& logger);

    [[nodiscard]] shared::pagination::Page<domain::connections::ConnectionProfile> execute(
        std::size_t page,
        std::size_t pageSize) const;

private:
    const domain::connections::IConnectionProfileRepository& repository_;
    shared::logging::ILogger& logger_;
};
} // namespace redisforge::app::connections
