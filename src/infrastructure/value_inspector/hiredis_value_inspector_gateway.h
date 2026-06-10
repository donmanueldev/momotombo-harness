#pragma once

#include "domain/value_inspector/services/i_value_inspector_gateway.h"

namespace redisforge::infrastructure::value_inspector
{
class HiredisValueInspectorGateway final : public redisforge::domain::value_inspector::IValueInspectorGateway
{
public:
    [[nodiscard]] redisforge::domain::value_inspector::KeyInspectionResult inspectKey(
        const redisforge::domain::connections::ConnectionProfile& profile,
        const std::string& key,
        const redisforge::domain::value_inspector::ValueInspectionOptions& options) const override;
};
} // namespace redisforge::infrastructure::value_inspector
