#pragma once

#include "shared/logging/logger.h"

namespace redisforge::infrastructure::logging
{
    class QtStructuredLogger final : public shared::logging::ILogger
    {
    public:
        void info(std::string_view message, shared::logging::LogContext context = {}) override;
        void warning(std::string_view message, shared::logging::LogContext context = {}) override;
        void error(std::string_view message, shared::logging::LogContext context = {}) override;
    };
} // namespace redisforge::infrastructure::logging
