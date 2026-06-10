#include "app/value_inspector/use_cases/save_hash_field_use_case.h"

#include <exception>

namespace redisforge::app::value_inspector
{
SaveHashFieldUseCase::SaveHashFieldUseCase(
    const connections::ActiveConnectionStore& activeConnectionStore,
    const domain::value_inspector::IValueInspectorGateway& valueInspectorGateway,
    shared::logging::ILogger& logger)
    : activeConnectionStore_(activeConnectionStore), valueInspectorGateway_(valueInspectorGateway), logger_(logger)
{
}

shared::result::OperationResult<std::monostate> SaveHashFieldUseCase::execute(const SaveHashFieldCommand& command) const
{
    const auto profile = activeConnectionStore_.activeConnection();
    if (!profile.has_value())
    {
        return shared::result::OperationResult<std::monostate>::failure(
            {"No active Redis connection. Test a connection first."});
    }

    if (command.field.empty())
    {
        return shared::result::OperationResult<std::monostate>::failure({"Hash field name is required."});
    }

    try
    {
        valueInspectorGateway_.saveHashField(*profile, command.key, command.field, command.value);
    }
    catch (const std::exception& exception)
    {
        logger_.error("value_inspector.save_hash_field.failed", {
            {"key", command.key},
            {"field", command.field},
            {"reason", exception.what()},
        });
        return shared::result::OperationResult<std::monostate>::failure({exception.what()});
    }

    logger_.info("value_inspector.save_hash_field.completed", {
        {"key", command.key},
        {"field", command.field},
        {"host", profile->host},
    });
    return shared::result::OperationResult<std::monostate>::success({});
}
} // namespace redisforge::app::value_inspector
