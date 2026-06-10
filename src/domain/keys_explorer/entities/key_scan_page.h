#pragma once

#include <string>
#include <vector>

namespace redisforge::domain::keys_explorer
{
struct KeyListEntry
{
    std::string key;
    std::string type{"-"};
};

struct KeyScanPage
{
    std::vector<KeyListEntry> entries;
    std::string nextCursor{"0"};
    bool hasMore{false};
};
} // namespace redisforge::domain::keys_explorer
