#pragma once

#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace redisforge::shared::logging
{
    using LogField = std::pair<std::string, std::string>;
    using LogContext = std::vector<LogField>;

    class ILogger
    {
    public:
        virtual ~ILogger() = default;

        virtual void info(std::string_view message, LogContext context = {}) = 0;
        virtual void warning(std::string_view message, LogContext context = {}) = 0;
        virtual void error(std::string_view message, LogContext context = {}) = 0;
    };
} // namespace redisforge::shared::logging
