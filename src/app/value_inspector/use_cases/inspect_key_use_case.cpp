#include "app/value_inspector/use_cases/inspect_key_use_case.h"

#include <exception>

namespace redisforge::app::value_inspector
{
    InspectKeyUseCase::InspectKeyUseCase(
        const connections::ActiveConnectionStore& activeConnectionStore,
        const domain::value_inspector::IValueInspectorGateway& valueInspectorGateway,
        shared::logging::ILogger& logger)
        : activeConnectionStore_(activeConnectionStore), valueInspectorGateway_(valueInspectorGateway), logger_(logger)
    {
    }

    shared::result::OperationResult<domain::value_inspector::KeyInspectionResult>
    InspectKeyUseCase::execute(const InspectKeyQuery& query) const
    {
        const auto& key = query.key;
        const auto profile = activeConnectionStore_.activeConnection();
        if (!profile.has_value())
        {
            return shared::result::OperationResult<redisforge::domain::value_inspector::KeyInspectionResult>::failure(
                {"No active Redis connection. Test a connection first."});
        }

        logger_.info("value_inspector.inspect.started", {
                         {"key", key},
                         {"host", profile->host},
                         {"database", std::to_string(profile->databaseIndex)},
                     });

        auto inspection = domain::value_inspector::KeyInspectionResult{};
        try
        {
            inspection = valueInspectorGateway_.inspectKey(*profile, key, query.options);
        }
        catch (const std::exception& exception)
        {
            logger_.error("value_inspector.inspect.failed", {
                {"key", key},
                {"reason", exception.what()},
            });
            return shared::result::OperationResult<domain::value_inspector::KeyInspectionResult>::failure(
                {exception.what()});
        }

        logger_.info("value_inspector.inspect.completed", {
                         {"key", inspection.key},
                         {"type", domain::value_inspector::redisValueTypeToString(inspection.type)},
                     });

        return shared::result::OperationResult<domain::value_inspector::KeyInspectionResult>::success(
            std::move(inspection));
    }
} // namespace redisforge::app::value_inspector
