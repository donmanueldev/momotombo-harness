#pragma once

#include <string>

#include "app/connections/services/active_connection_store.h"
#include "domain/value_inspector/services/i_value_inspector_gateway.h"
#include "shared/logging/logger.h"
#include "shared/result/operation_result.h"

namespace redisforge::app::value_inspector
{
class InspectKeyUseCase final
{
public:
    InspectKeyUseCase(
        const redisforge::app::connections::ActiveConnectionStore& activeConnectionStore,
        const redisforge::domain::value_inspector::IValueInspectorGateway& valueInspectorGateway,
        redisforge::shared::logging::ILogger& logger);

    [[nodiscard]] redisforge::shared::result::OperationResult<redisforge::domain::value_inspector::KeyInspectionResult>
    execute(const std::string& key) const;

private:
    const redisforge::app::connections::ActiveConnectionStore& activeConnectionStore_;
    const redisforge::domain::value_inspector::IValueInspectorGateway& valueInspectorGateway_;
    redisforge::shared::logging::ILogger& logger_;
};
} // namespace redisforge::app::value_inspector
