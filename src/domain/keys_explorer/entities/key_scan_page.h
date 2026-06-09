#pragma once

#include <string>
#include <vector>

namespace redisforge::domain::keys_explorer
{
struct KeyScanPage
{
    std::vector<std::string> keys;
    std::string nextCursor{"0"};
    bool hasMore{false};
};
} // namespace redisforge::domain::keys_explorer
