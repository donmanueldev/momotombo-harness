#include <gtest/gtest.h>

#include "app/connections/services/active_connection_store.h"
#include "app/value_inspector/use_cases/save_string_value_use_case.h"
#include "domain/value_inspector/services/i_value_inspector_gateway.h"
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

    class FakeValueInspectorGateway final : public redisforge::domain::value_inspector::IValueInspectorGateway
    {
    public:
        redisforge::domain::value_inspector::KeyInspectionResult inspectKey(
            const redisforge::domain::connections::ConnectionProfile&,
            const std::string&,
            const redisforge::domain::value_inspector::ValueInspectionOptions&) const override
        {
            return {};
        }

        void saveStringValue(
            const redisforge::domain::connections::ConnectionProfile&,
            const std::string& key,
            const std::string& value) const override
        {
            lastStringKey = key;
            lastStringValue = value;
        }

        void saveHashField(
            const redisforge::domain::connections::ConnectionProfile&,
            const std::string&,
            const std::string&,
            const std::string&) const override
        {
        }

        mutable std::string lastStringKey;
        mutable std::string lastStringValue;
    };
}

namespace redisforge::app::value_inspector
{
    TEST(SaveStringValueUseCaseTests, FailsWhenNoActiveConnectionExists)
    {
        auto activeConnectionStore = redisforge::app::connections::ActiveConnectionStore{};
        auto gateway = FakeValueInspectorGateway{};
        auto logger = NullLogger{};
        auto useCase = SaveStringValueUseCase{activeConnectionStore, gateway, logger};

        const auto result = useCase.execute({"user:1", "hello"});

        EXPECT_FALSE(result.isSuccessful);
        ASSERT_FALSE(result.errors.empty());
    }

    TEST(SaveStringValueUseCaseTests, DelegatesSaveToGatewayUsingActiveConnection)
    {
        auto activeConnectionStore = redisforge::app::connections::ActiveConnectionStore{};
        activeConnectionStore.setActiveConnection(
            redisforge::domain::connections::ConnectionProfile{
                "", "Local Redis", "127.0.0.1", 6379, "", "", 0, 3000
            });
        auto gateway = FakeValueInspectorGateway{};
        auto logger = NullLogger{};
        auto useCase = SaveStringValueUseCase{activeConnectionStore, gateway, logger};

        const auto result = useCase.execute({"user:1", "{\"ok\":true}"});

        ASSERT_TRUE(result.isSuccessful);
        EXPECT_EQ(gateway.lastStringKey, "user:1");
        EXPECT_EQ(gateway.lastStringValue, "{\"ok\":true}");
    }
} // namespace redisforge::app::value_inspector
