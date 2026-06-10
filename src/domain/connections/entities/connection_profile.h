#pragma once

#include <string>

namespace redisforge::domain::connections
{
struct ConnectionProfile
{
    std::string id;
    std::string name;
    std::string host;
    std::uint16_t port{6379};
    std::string username;
    std::string password;
    std::int32_t databaseIndex{0};
    std::int32_t connectTimeoutMs{3000};
    bool useTls{false};
};
} // namespace redisforge::domain::connections
