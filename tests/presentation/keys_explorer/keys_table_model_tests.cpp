#include <gtest/gtest.h>

#include "presentation/keys_explorer/models/keys_table_model.h"

namespace redisforge::presentation::keys_explorer
{
    TEST(KeysTableModelTests, ReturnsEmptyDataForOutOfRangeIndexes)
    {
        auto model = KeysTableModel{};
        model.setEntries({redisforge::domain::keys_explorer::KeyListEntry{"user:1", "string"}});

        EXPECT_FALSE(model.data(model.index(-1, 0), Qt::DisplayRole).isValid());
        EXPECT_FALSE(model.data(model.index(10, 0), Qt::DisplayRole).isValid());
        EXPECT_FALSE(model.data(model.index(0, 5), Qt::DisplayRole).isValid());
    }
} // namespace redisforge::presentation::keys_explorer
