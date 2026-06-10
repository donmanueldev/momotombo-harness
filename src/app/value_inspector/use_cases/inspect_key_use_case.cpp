#include "app/value_inspector/use_cases/inspect_key_use_case.h"

namespace redisforge::app::value_inspector
{
InspectKeyUseCase::InspectKeyUseCase(
    const redisforge::app::connections::ActiveConnectionStore& activeConnectionStore,
    const redisforge::domain::value_inspector::IValueInspectorGateway& valueInspectorGateway,
    redisforge::shared::logging::ILogger& logger)
    : activeConnectionStore_(activeConnectionStore), valueInspectorGateway_(valueInspectorGateway), logger_(logger)
{
}

redisforge::shared::result::OperationResult<redisforge::domain::value_inspector::KeyInspectionResult>
InspectKeyUseCase::execute(const std::string& key) const
{
    const auto profile = activeConnectionStore_.activeConnection();
    if (!profile.has_value())
    {
        return redisforge::shared::result::OperationResult<redisforge::domain::value_inspector::KeyInspectionResult>::failure(
            {"No active Redis connection. Test a connection first."});
    }

    logger_.info("value_inspector.inspect.started", {
        {"key", key},
        {"host", profile->host},
        {"database", std::to_string(profile->databaseIndex)},
    });

    auto inspection = valueInspectorGateway_.inspectKey(*profile, key);

    logger_.info("value_inspector.inspect.completed", {
        {"key", inspection.key},
        {"type", inspection.type},
        {"hasStringValue", inspection.hasStringValue ? "true" : "false"},
    });

    return redisforge::shared::result::OperationResult<redisforge::domain::value_inspector::KeyInspectionResult>::success(
        std::move(inspection));
}
} // namespace redisforge::app::value_inspector
