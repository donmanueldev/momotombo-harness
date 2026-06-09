#include <gtest/gtest.h>

#include "app/connections/services/active_connection_store.h"
#include "app/keys_explorer/use_cases/scan_keys_use_case.h"
#include "domain/keys_explorer/services/i_key_explorer_gateway.h"
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

class FakeKeyExplorerGateway final : public redisforge::domain::keys_explorer::IKeyExplorerGateway
{
public:
    redisforge::domain::keys_explorer::KeyScanPage scanKeys(
        const redisforge::domain::connections::ConnectionProfile&,
        const std::string& cursor,
        const std::string& pattern,
        const std::size_t pageSize) const override
    {
        lastCursor = cursor;
        lastPattern = pattern;
        lastPageSize = pageSize;

        return redisforge::domain::keys_explorer::KeyScanPage{{"user:1", "user:2"}, "17", true};
    }

    mutable std::string lastCursor;
    mutable std::string lastPattern;
    mutable std::size_t lastPageSize{0};
};
} // namespace

namespace redisforge::app::keys_explorer
{
TEST(ScanKeysUseCaseTests, FailsWhenNoActiveConnectionExists)
{
    auto activeConnectionStore = redisforge::app::connections::ActiveConnectionStore{};
    auto gateway = FakeKeyExplorerGateway{};
    auto logger = NullLogger{};
    auto useCase = ScanKeysUseCase{activeConnectionStore, gateway, logger};

    const auto result = useCase.execute(ScanKeysQuery{});

    EXPECT_FALSE(result.isSuccessful);
    ASSERT_FALSE(result.errors.empty());
}

TEST(ScanKeysUseCaseTests, DelegatesScanToGatewayUsingActiveConnection)
{
    auto activeConnectionStore = redisforge::app::connections::ActiveConnectionStore{};
    activeConnectionStore.setActiveConnection(
        redisforge::domain::connections::ConnectionProfile{"", "Local Redis", "127.0.0.1", 6379, "", "", 0, 3000, false});
    auto gateway = FakeKeyExplorerGateway{};
    auto logger = NullLogger{};
    auto useCase = ScanKeysUseCase{activeConnectionStore, gateway, logger};

    const auto result = useCase.execute(ScanKeysQuery{"0", "user:*", 50});

    ASSERT_TRUE(result.isSuccessful);
    EXPECT_EQ(gateway.lastCursor, "0");
    EXPECT_EQ(gateway.lastPattern, "user:*");
    EXPECT_EQ(gateway.lastPageSize, 50U);
    EXPECT_EQ(result.value.keys.size(), 2U);
    EXPECT_EQ(result.value.nextCursor, "17");
}
} // namespace redisforge::app::keys_explorer
