#include <gtest/gtest.h>

#include "app/connections/use_cases/save_connection_profile_use_case.h"
#include "domain/connections/entities/connection_profile.h"
#include "infrastructure/connections/in_memory_connection_profile_repository.h"
#include "shared/logging/logger.h"

namespace
{
    class NullLogger final : public redisforge::shared::logging::ILogger
    {
    public:
        void info(std::string_view, redisforge::shared::logging::LogContext) override
        {
        }

        void warning(std::string_view, redisforge::shared::logging::LogContext) override
        {
        }

        void error(std::string_view, redisforge::shared::logging::LogContext) override
        {
        }
    };
} // namespace

namespace redisforge::app::connections
{
    TEST(SaveConnectionProfileUseCaseTests, GeneratesIdAndPersistsConnectionProfile)
    {
        auto repository = infrastructure::connections::InMemoryConnectionProfileRepository{};
        auto logger = NullLogger{};
        auto useCase = SaveConnectionProfileUseCase{repository, logger};

        const auto result = useCase.execute(domain::connections::ConnectionProfile{
            "",
            "Staging Redis",
            "redis.internal",
            6379,
            "",
            "",
            0,
            3000,
            false,
        });

        ASSERT_TRUE(result.isSuccessful);
        ASSERT_FALSE(result.value.empty());
        EXPECT_EQ(repository.count(), 1U);
    }
} // namespace redisforge::app::connections
