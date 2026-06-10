#include <gtest/gtest.h>

#include <cstdlib>
#include <string>
#include <string_view>

#include "domain/connections/entities/connection_profile.h"
#include "infrastructure/connections/hiredis_connection_gateway.h"
#include "infrastructure/keys_explorer/hiredis_key_explorer_gateway.h"
#include "infrastructure/value_inspector/hiredis_value_inspector_gateway.h"

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

    TEST(HiredisConnectionGatewayIntegrationTests, ScansKeysFromRedisWhenIntegrationEnvironmentIsConfigured)
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

        if (const auto* database = std::getenv("REDISFORGE_TEST_REDIS_DATABASE"); database != nullptr)
        {
            profile.databaseIndex = std::stoi(database);
        }

        auto gateway = redisforge::infrastructure::keys_explorer::HiredisKeyExplorerGateway{};
        const auto page = gateway.scanKeys(profile, "0", "*", 100);

        EXPECT_FALSE(page.keys.empty());
    }

    TEST(HiredisConnectionGatewayIntegrationTests, InspectsKeyTypeFromRedisWhenIntegrationEnvironmentIsConfigured)
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

        if (const auto* database = std::getenv("REDISFORGE_TEST_REDIS_DATABASE"); database != nullptr)
        {
            profile.databaseIndex = std::stoi(database);
        }

        auto scanGateway = redisforge::infrastructure::keys_explorer::HiredisKeyExplorerGateway{};
        const auto page = scanGateway.scanKeys(profile, "0", "*", 100);
        ASSERT_FALSE(page.keys.empty());

        auto inspectorGateway = redisforge::infrastructure::value_inspector::HiredisValueInspectorGateway{};
        const auto inspection = inspectorGateway.inspectKey(profile, page.keys.front(), {});

        EXPECT_EQ(inspection.key, page.keys.front());
        EXPECT_NE(inspection.type, redisforge::domain::value_inspector::RedisValueType::Unsupported);
    }
} // namespace redisforge::infrastructure::connections
