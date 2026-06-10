#pragma once

#include <string>

namespace redisforge::domain::value_inspector
{
struct KeyInspectionResult
{
    std::string key;
    std::string type;
    std::string stringValue;
    bool hasStringValue{false};
};
} // namespace redisforge::domain::value_inspector
