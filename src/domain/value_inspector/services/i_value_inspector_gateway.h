#pragma once

#include <string>

#include "domain/connections/entities/connection_profile.h"
#include "domain/value_inspector/entities/key_inspection_result.h"

namespace redisforge::domain::value_inspector
{
class IValueInspectorGateway
{
public:
    virtual ~IValueInspectorGateway() = default;

    virtual KeyInspectionResult inspectKey(
        const redisforge::domain::connections::ConnectionProfile& profile,
        const std::string& key,
        const redisforge::domain::value_inspector::ValueInspectionOptions& options) const = 0;
};
} // namespace redisforge::domain::value_inspector
