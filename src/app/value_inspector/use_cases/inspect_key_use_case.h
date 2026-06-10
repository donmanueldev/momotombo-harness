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
            const connections::ActiveConnectionStore& activeConnectionStore,
            const domain::value_inspector::IValueInspectorGateway& valueInspectorGateway,
            shared::logging::ILogger& logger);

        [[nodiscard]] shared::result::OperationResult<domain::value_inspector::KeyInspectionResult>
        execute(const std::string& key) const;

    private:
        const connections::ActiveConnectionStore& activeConnectionStore_;
        const domain::value_inspector::IValueInspectorGateway& valueInspectorGateway_;
        shared::logging::ILogger& logger_;
    };
} // namespace redisforge::app::value_inspector
