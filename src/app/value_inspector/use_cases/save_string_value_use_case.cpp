#include "app/value_inspector/use_cases/save_string_value_use_case.h"

#include <exception>

namespace redisforge::app::value_inspector
{
SaveStringValueUseCase::SaveStringValueUseCase(
    const connections::ActiveConnectionStore& activeConnectionStore,
    const domain::value_inspector::IValueInspectorGateway& valueInspectorGateway,
    shared::logging::ILogger& logger)
    : activeConnectionStore_(activeConnectionStore), valueInspectorGateway_(valueInspectorGateway), logger_(logger)
{
}

shared::result::OperationResult<std::monostate> SaveStringValueUseCase::execute(const SaveStringValueCommand& command) const
{
    const auto profile = activeConnectionStore_.activeConnection();
    if (!profile.has_value())
    {
        return shared::result::OperationResult<std::monostate>::failure(
            {"No active Redis connection. Test a connection first."});
    }

    try
    {
        valueInspectorGateway_.saveStringValue(*profile, command.key, command.value);
    }
    catch (const std::exception& exception)
    {
        logger_.error("value_inspector.save_string.failed", {
            {"key", command.key},
            {"reason", exception.what()},
        });
        return shared::result::OperationResult<std::monostate>::failure({exception.what()});
    }

    logger_.info("value_inspector.save_string.completed", {
        {"key", command.key},
        {"host", profile->host},
    });
    return shared::result::OperationResult<std::monostate>::success({});
}
} // namespace redisforge::app::value_inspector
