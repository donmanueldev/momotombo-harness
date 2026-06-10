#include <gtest/gtest.h>

#include "presentation/value_inspector/viewmodels/value_inspection_view_data_mapper.h"

namespace redisforge::presentation::value_inspector
{
TEST(ValueInspectionViewDataMapperTests, PrettyPrintsJsonStrings)
{
    auto inspection = redisforge::domain::value_inspector::KeyInspectionResult{};
    inspection.key = "user:1";
    inspection.type = redisforge::domain::value_inspector::RedisValueType::String;
    inspection.stringValue = R"({"name":"Ada","active":true})";

    const auto data = ValueInspectionViewDataMapper::map(inspection);

    EXPECT_TRUE(data.isStringJson);
    EXPECT_EQ(data.typeLabel, "string");
    EXPECT_NE(data.stringValue.find("\n"), std::string::npos);
    EXPECT_NE(data.stringValue.find("  \"name\""), std::string::npos);
}

TEST(ValueInspectionViewDataMapperTests, PreservesTypedCollections)
{
    auto inspection = redisforge::domain::value_inspector::KeyInspectionResult{};
    inspection.key = "scores";
    inspection.type = redisforge::domain::value_inspector::RedisValueType::SortedSet;
    inspection.sortedSetEntries.push_back({"alice", 42.5, "42.5"});

    const auto data = ValueInspectionViewDataMapper::map(inspection);

    EXPECT_EQ(data.type, redisforge::domain::value_inspector::RedisValueType::SortedSet);
    ASSERT_EQ(data.sortedSetEntries.size(), 1U);
    EXPECT_EQ(data.sortedSetEntries.front().member, "alice");
    EXPECT_EQ(data.sortedSetEntries.front().scoreText, "42.5");
}
} // namespace redisforge::presentation::value_inspector
