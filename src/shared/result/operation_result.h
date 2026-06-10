#pragma once

#include <string>
#include <utility>
#include <vector>

namespace redisforge::shared::result
{
    template <typename TValue>
    struct OperationResult
    {
        bool isSuccessful{false};
        TValue value{};
        std::vector<std::string> errors;

        [[nodiscard]] static OperationResult success(TValue resultValue)
        {
            return OperationResult{true, std::move(resultValue), {}};
        }

        [[nodiscard]] static OperationResult failure(std::vector<std::string> errorsList)
        {
            return OperationResult{false, TValue{}, std::move(errorsList)};
        }
    };
} // namespace redisforge::shared::result
