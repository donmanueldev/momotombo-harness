#pragma once

#include <string>
#include <vector>

#include "domain/value_inspector/entities/key_inspection_result.h"

namespace redisforge::presentation::value_inspector
{
struct ValueInspectionViewData
{
    std::string key;
    std::string typeLabel{"-"};
    redisforge::domain::value_inspector::RedisValueType type{
        redisforge::domain::value_inspector::RedisValueType::None};
    std::string stringValue;
    bool isStringJson{false};
    bool isStringTruncated{false};
    bool hasAdditionalEntries{false};
    std::size_t loadedEntryCount{0};
    std::vector<redisforge::domain::value_inspector::HashEntry> hashEntries;
    std::vector<redisforge::domain::value_inspector::ListEntry> listEntries;
    std::vector<std::string> setMembers;
    std::vector<redisforge::domain::value_inspector::SortedSetEntry> sortedSetEntries;
    std::vector<redisforge::domain::value_inspector::StreamEntry> streamEntries;
};
} // namespace redisforge::presentation::value_inspector
