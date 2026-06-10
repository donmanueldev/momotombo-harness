#pragma once

#include <string>
#include <vector>

#include "domain/connections/entities/connection_profile.h"
#include "domain/connections/repositories/i_connection_profile_repository.h"
#include "domain/connections/services/connection_profile_validator.h"
#include "shared/logging/logger.h"
#include "shared/result/operation_result.h"

namespace redisforge::app::connections
{
    class SaveConnectionProfileUseCase
    {
    public:
        SaveConnectionProfileUseCase(
            domain::connections::IConnectionProfileRepository& repository,
            shared::logging::ILogger& logger);

        [[nodiscard]] shared::result::OperationResult<std::string> execute(
            domain::connections::ConnectionProfile profile) const;

    private:
        [[nodiscard]] static std::string generateProfileId();

        domain::connections::IConnectionProfileRepository& repository_;
        shared::logging::ILogger& logger_;
        domain::connections::ConnectionProfileValidator validator_;
    };
} // namespace redisforge::app::connections
