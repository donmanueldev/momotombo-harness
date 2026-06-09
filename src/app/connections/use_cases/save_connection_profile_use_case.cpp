#include "app/connections/use_cases/save_connection_profile_use_case.h"

#include <atomic>
#include <chrono>

namespace redisforge::app::connections
{
SaveConnectionProfileUseCase::SaveConnectionProfileUseCase(
    domain::connections::IConnectionProfileRepository& repository,
    shared::logging::ILogger& logger)
    : repository_(repository), logger_(logger)
{
}

shared::result::OperationResult<std::string> SaveConnectionProfileUseCase::execute(
    domain::connections::ConnectionProfile profile) const
{
    const auto validationErrors = validator_.validate(profile);
    if (!validationErrors.empty())
    {
        logger_.warning("connections.profile.save_validation_failed", {
            {"errorCount", std::to_string(validationErrors.size())},
        });
        return shared::result::OperationResult<std::string>::failure(validationErrors);
    }

    if (profile.id.empty())
    {
        profile.id = generateProfileId();
    }

    repository_.save(profile);

    logger_.info("connections.profile.saved", {
        {"profileId", profile.id},
        {"name", profile.name},
        {"host", profile.host},
        {"port", std::to_string(profile.port)},
    });

    return shared::result::OperationResult<std::string>::success(profile.id);
}

std::string SaveConnectionProfileUseCase::generateProfileId()
{
    static auto sequence = std::atomic_uint64_t{0};
    const auto now = std::chrono::steady_clock::now().time_since_epoch().count();
    return "conn-" + std::to_string(now) + "-" + std::to_string(sequence.fetch_add(1));
}
} // namespace redisforge::app::connections
