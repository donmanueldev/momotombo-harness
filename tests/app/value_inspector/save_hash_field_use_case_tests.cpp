#include <gtest/gtest.h>

#include "app/connections/services/active_connection_store.h"
#include "app/value_inspector/use_cases/save_hash_field_use_case.h"
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
            const std::string&,
            const std::string&) const override
        {
        }

        void saveHashField(
            const redisforge::domain::connections::ConnectionProfile&,
            const std::string& key,
            const std::string& field,
            const std::string& value) const override
        {
            lastHashKey = key;
            lastHashField = field;
            lastHashValue = value;
        }

        mutable std::string lastHashKey;
        mutable std::string lastHashField;
        mutable std::string lastHashValue;
    };
}

namespace redisforge::app::value_inspector
{
    TEST(SaveHashFieldUseCaseTests, RejectsEmptyFieldName)
    {
        auto activeConnectionStore = redisforge::app::connections::ActiveConnectionStore{};
        activeConnectionStore.setActiveConnection(
            redisforge::domain::connections::ConnectionProfile{
                "", "Local Redis", "127.0.0.1", 6379, "", "", 0, 3000
            });
        auto gateway = FakeValueInspectorGateway{};
        auto logger = NullLogger{};
        auto useCase = SaveHashFieldUseCase{activeConnectionStore, gateway, logger};

        const auto result = useCase.execute({"session:1", "", "active"});

        EXPECT_FALSE(result.isSuccessful);
        ASSERT_FALSE(result.errors.empty());
    }

    TEST(SaveHashFieldUseCaseTests, DelegatesSaveToGatewayUsingActiveConnection)
    {
        auto activeConnectionStore = redisforge::app::connections::ActiveConnectionStore{};
        activeConnectionStore.setActiveConnection(
            redisforge::domain::connections::ConnectionProfile{
                "", "Local Redis", "127.0.0.1", 6379, "", "", 0, 3000
            });
        auto gateway = FakeValueInspectorGateway{};
        auto logger = NullLogger{};
        auto useCase = SaveHashFieldUseCase{activeConnectionStore, gateway, logger};

        const auto result = useCase.execute({"session:1", "status", "active"});

        ASSERT_TRUE(result.isSuccessful);
        EXPECT_EQ(gateway.lastHashKey, "session:1");
        EXPECT_EQ(gateway.lastHashField, "status");
        EXPECT_EQ(gateway.lastHashValue, "active");
    }
} // namespace redisforge::app::value_inspector
