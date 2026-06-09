#include <gtest/gtest.h>

#include <cstdlib>
#include <string>
#include <string_view>

#include "domain/connections/entities/connection_profile.h"
#include "infrastructure/connections/hiredis_connection_gateway.h"

namespace redisforge::infrastructure::connections
{
TEST(HiredisConnectionGatewayIntegrationTests, ConnectsToRedisWhenIntegrationEnvironmentIsConfigured)
{
    const auto* enabled = std::getenv("REDISFORGE_RUN_REDIS_INTEGRATION_TESTS");
    if (enabled == nullptr || std::string_view{enabled} != "1")
    {
        GTEST_SKIP() << "Set REDISFORGE_RUN_REDIS_INTEGRATION_TESTS=1 to enable Redis integration tests.";
    }

    const auto* host = std::getenv("REDISFORGE_TEST_REDIS_HOST");
    const auto* port = std::getenv("REDISFORGE_TEST_REDIS_PORT");
    if (host == nullptr || port == nullptr)
    {
        GTEST_SKIP() << "Set REDISFORGE_TEST_REDIS_HOST and REDISFORGE_TEST_REDIS_PORT.";
    }

    auto profile = domain::connections::ConnectionProfile{};
    profile.name = "Integration Test Redis";
    profile.host = host;
    profile.port = static_cast<std::uint16_t>(std::stoi(port));
    profile.connectTimeoutMs = 3000;

    if (const auto* password = std::getenv("REDISFORGE_TEST_REDIS_PASSWORD"); password != nullptr)
    {
        profile.password = password;
    }

    if (const auto* username = std::getenv("REDISFORGE_TEST_REDIS_USERNAME"); username != nullptr)
    {
        profile.username = username;
    }

    auto gateway = HiredisConnectionGateway{};
    const auto result = gateway.testConnection(profile);

    EXPECT_TRUE(result.isSuccessful) << result.message;
}
} // namespace redisforge::infrastructure::connections
