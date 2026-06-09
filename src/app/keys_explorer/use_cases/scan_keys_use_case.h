#pragma once

#include <cstddef>
#include <string>

#include "app/connections/services/active_connection_store.h"
#include "domain/keys_explorer/services/i_key_explorer_gateway.h"
#include "shared/logging/logger.h"
#include "shared/result/operation_result.h"

namespace redisforge::app::keys_explorer
{
struct ScanKeysQuery
{
    std::string cursor{"0"};
    std::string pattern{"*"};
    std::size_t pageSize{100};
};

class ScanKeysUseCase
{
public:
    ScanKeysUseCase(
        const redisforge::app::connections::ActiveConnectionStore& activeConnectionStore,
        const redisforge::domain::keys_explorer::IKeyExplorerGateway& keyExplorerGateway,
        redisforge::shared::logging::ILogger& logger);

    [[nodiscard]] redisforge::shared::result::OperationResult<redisforge::domain::keys_explorer::KeyScanPage> execute(
        const ScanKeysQuery& query) const;

private:
    const redisforge::app::connections::ActiveConnectionStore& activeConnectionStore_;
    const redisforge::domain::keys_explorer::IKeyExplorerGateway& keyExplorerGateway_;
    redisforge::shared::logging::ILogger& logger_;
};
} // namespace redisforge::app::keys_explorer
