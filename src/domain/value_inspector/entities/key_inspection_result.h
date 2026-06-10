#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace redisforge::domain::value_inspector
{
enum class RedisValueType
{
    None,
    String,
    Hash,
    List,
    Set,
    SortedSet,
    Stream,
    Unsupported,
};

[[nodiscard]] inline std::string redisValueTypeToString(const RedisValueType type)
{
    switch (type)
    {
    case RedisValueType::None:
        return "none";
    case RedisValueType::String:
        return "string";
    case RedisValueType::Hash:
        return "hash";
    case RedisValueType::List:
        return "list";
    case RedisValueType::Set:
        return "set";
    case RedisValueType::SortedSet:
        return "zset";
    case RedisValueType::Stream:
        return "stream";
    case RedisValueType::Unsupported:
        return "unsupported";
    }

    return "unsupported";
}

struct HashEntry
{
    std::string field;
    std::string value;
};

struct ListEntry
{
    std::size_t index{0};
    std::string value;
};

struct SortedSetEntry
{
    std::string member;
    double score{0.0};
    std::string scoreText;
};

struct StreamEntry
{
    std::string id;
    std::vector<HashEntry> fields;
};

struct KeyInspectionResult
{
    std::string key;
    RedisValueType type{RedisValueType::None};
    std::string stringValue;
    std::vector<HashEntry> hashEntries;
    std::vector<ListEntry> listEntries;
    std::vector<std::string> setMembers;
    std::vector<SortedSetEntry> sortedSetEntries;
    std::vector<StreamEntry> streamEntries;
};
} // namespace redisforge::domain::value_inspector
