#pragma once

#include <string>
#include <variant>

#include "app/connections/services/active_connection_store.h"
#include "domain/value_inspector/services/i_value_inspector_gateway.h"
#include "shared/logging/logger.h"
#include "shared/result/operation_result.h"

namespace redisforge::app::value_inspector
{
struct SaveStringValueCommand
{
    std::string key;
    std::string value;
};

class SaveStringValueUseCase final
{
public:
    SaveStringValueUseCase(
        const connections::ActiveConnectionStore& activeConnectionStore,
        const domain::value_inspector::IValueInspectorGateway& valueInspectorGateway,
        shared::logging::ILogger& logger);

    [[nodiscard]] shared::result::OperationResult<std::monostate> execute(const SaveStringValueCommand& command) const;

private:
    const connections::ActiveConnectionStore& activeConnectionStore_;
    const domain::value_inspector::IValueInspectorGateway& valueInspectorGateway_;
    shared::logging::ILogger& logger_;
};
} // namespace redisforge::app::value_inspector
