#include <gtest/gtest.h>

#include "app/connections/services/active_connection_store.h"
#include "app/connections/use_cases/test_and_activate_connection_use_case.h"
#include "domain/connections/services/i_redis_connection_gateway.h"
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

    class FakeRedisConnectionGateway final : public redisforge::domain::connections::IRedisConnectionGateway
    {
    public:
        redisforge::domain::connections::ConnectionTestResult testConnection(
            const redisforge::domain::connections::ConnectionProfile& profile) const override
        {
            lastProfileName = profile.name;
            return redisforge::domain::connections::ConnectionTestResult{
                true,
                "ok",
                std::chrono::milliseconds{5},
                "7.2.0",
            };
        }

        mutable std::string lastProfileName;
    };
} // namespace

namespace redisforge::app::connections
{
    TEST(TestAndActivateConnectionUseCaseTests, ActivatesSuccessfulConnection)
    {
        auto gateway = FakeRedisConnectionGateway{};
        auto store = ActiveConnectionStore{};
        auto logger = NullLogger{};
        auto useCase = TestAndActivateConnectionUseCase{gateway, store, logger};

        const auto profile = redisforge::domain::connections::ConnectionProfile{
            "",
            "Local Redis",
            "127.0.0.1",
            6379,
            "",
            "",
            0,
            3000,
            false,
        };

        const auto result = useCase.execute(profile);

        ASSERT_TRUE(result.testResult.isSuccessful);
        ASSERT_TRUE(result.activeConnection.has_value());
        ASSERT_TRUE(store.activeConnection().has_value());
        EXPECT_EQ(result.activeConnection->profile.name, "Local Redis");
        EXPECT_EQ(gateway.lastProfileName, "Local Redis");
    }
} // namespace redisforge::app::connections
