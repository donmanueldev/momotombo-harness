#pragma once

#include "domain/value_inspector/services/i_value_inspector_gateway.h"

namespace redisforge::infrastructure::value_inspector
{
class HiredisValueInspectorGateway final : public domain::value_inspector::IValueInspectorGateway
{
public:
    [[nodiscard]] domain::value_inspector::KeyInspectionResult inspectKey(
        const domain::connections::ConnectionProfile& profile,
        const std::string& key,
        const domain::value_inspector::ValueInspectionOptions& options) const override;

    void saveStringValue(
        const domain::connections::ConnectionProfile& profile,
        const std::string& key,
        const std::string& value) const override;

    void saveHashField(
        const domain::connections::ConnectionProfile& profile,
        const std::string& key,
        const std::string& field,
        const std::string& value) const override;
};
} // namespace redisforge::infrastructure::value_inspector
