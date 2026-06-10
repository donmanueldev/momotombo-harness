#pragma once

#include <algorithm>
#include <cstddef>
#include <vector>

namespace redisforge::shared::pagination
{
    template <typename T>
    struct Page
    {
        std::vector<T> items;
        std::size_t page{1};
        std::size_t pageSize{20};
        std::size_t totalCount{0};

        [[nodiscard]] std::size_t totalPages() const
        {
            if (pageSize == 0)
            {
                return 0;
            }

            return std::max<std::size_t>(1, (totalCount + pageSize - 1) / pageSize);
        }
    };
} // namespace redisforge::shared::pagination
