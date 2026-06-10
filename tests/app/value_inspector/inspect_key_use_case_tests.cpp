#include <gtest/gtest.h>

#include "app/connections/services/active_connection_store.h"
#include "app/value_inspector/use_cases/inspect_key_use_case.h"
#include "domain/value_inspector/services/i_value_inspector_gateway.h"
#include "shared/logging/logger.h"

namespace
{
class NullLogger final : public redisforge::shared::logging::ILogger
{
public:
    void info(std::string_view, redisforge::shared::logging::LogContext) override {}
    void warning(std::string_view, redisforge::shared::logging::LogContext) override {}
    void error(std::string_view, redisforge::shared::logging::LogContext) override {}
};

class FakeValueInspectorGateway final : public redisforge::domain::value_inspector::IValueInspectorGateway
{
public:
    redisforge::domain::value_inspector::KeyInspectionResult inspectKey(
        const redisforge::domain::connections::ConnectionProfile&,
        const std::string& key) const override
    {
        lastKey = key;
        return redisforge::domain::value_inspector::KeyInspectionResult{
            key,
            redisforge::domain::value_inspector::RedisValueType::String,
            "hello",
            {},
            {},
            {},
            {},
            {},
        };
    }

    mutable std::string lastKey;
};
} // namespace

namespace redisforge::app::value_inspector
{
TEST(InspectKeyUseCaseTests, FailsWhenNoActiveConnectionExists)
{
    auto activeConnectionStore = redisforge::app::connections::ActiveConnectionStore{};
    auto gateway = FakeValueInspectorGateway{};
    auto logger = NullLogger{};
    auto useCase = InspectKeyUseCase{activeConnectionStore, gateway, logger};

    const auto result = useCase.execute("app:name");

    EXPECT_FALSE(result.isSuccessful);
    ASSERT_FALSE(result.errors.empty());
}

TEST(InspectKeyUseCaseTests, DelegatesInspectionUsingActiveConnection)
{
    auto activeConnectionStore = redisforge::app::connections::ActiveConnectionStore{};
    activeConnectionStore.setActiveConnection(
        redisforge::domain::connections::ConnectionProfile{"", "Local Redis", "127.0.0.1", 6379, "", "", 0, 3000, false});
    auto gateway = FakeValueInspectorGateway{};
    auto logger = NullLogger{};
    auto useCase = InspectKeyUseCase{activeConnectionStore, gateway, logger};

    const auto result = useCase.execute("app:name");

    ASSERT_TRUE(result.isSuccessful);
    EXPECT_EQ(gateway.lastKey, "app:name");
    EXPECT_EQ(result.value.type, redisforge::domain::value_inspector::RedisValueType::String);
    EXPECT_EQ(result.value.stringValue, "hello");
}
} // namespace redisforge::app::value_inspector
