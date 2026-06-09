#include <gtest/gtest.h>

#include "domain/connections/entities/connection_profile.h"
#include "domain/connections/services/connection_profile_validator.h"

namespace redisforge::domain::connections
{
TEST(ConnectionProfileValidatorTests, ReturnsErrorsWhenRequiredFieldsAreMissing)
{
    auto validator = ConnectionProfileValidator{};
    auto profile = ConnectionProfile{};
    profile.port = 0;
    profile.connectTimeoutMs = 0;
    profile.databaseIndex = -1;

    const auto errors = validator.validate(profile);

    EXPECT_EQ(errors.size(), 5U);
}

TEST(ConnectionProfileValidatorTests, AcceptsValidConnectionProfile)
{
    auto validator = ConnectionProfileValidator{};
    auto profile = ConnectionProfile{
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

    EXPECT_TRUE(validator.validate(profile).empty());
}
} // namespace redisforge::domain::connections
