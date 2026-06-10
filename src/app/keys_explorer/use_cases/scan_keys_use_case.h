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
            const connections::ActiveConnectionStore& activeConnectionStore,
            const domain::keys_explorer::IKeyExplorerGateway& keyExplorerGateway,
            shared::logging::ILogger& logger);

        [[nodiscard]] shared::result::OperationResult<domain::keys_explorer::KeyScanPage>
        execute(
            const ScanKeysQuery& query) const;

    private:
        const connections::ActiveConnectionStore& activeConnectionStore_;
        const domain::keys_explorer::IKeyExplorerGateway& keyExplorerGateway_;
        shared::logging::ILogger& logger_;
    };
} // namespace redisforge::app::keys_explorer
