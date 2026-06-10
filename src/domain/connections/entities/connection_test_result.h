#pragma once

#include <chrono>
#include <string>

namespace redisforge::domain::connections
{
    struct ConnectionTestResult
    {
        bool isSuccessful{false};
        std::string message;
        std::chrono::milliseconds latency{0};
        std::string serverVersion;
    };
} // namespace redisforge::domain::connections
