#pragma once

#include "domain/value_inspector/entities/key_inspection_result.h"
#include "presentation/value_inspector/viewmodels/value_inspection_view_data.h"

namespace redisforge::presentation::value_inspector
{
class ValueInspectionViewDataMapper final
{
public:
    [[nodiscard]] static ValueInspectionViewData map(
        const redisforge::domain::value_inspector::KeyInspectionResult& inspection);
};
} // namespace redisforge::presentation::value_inspector
