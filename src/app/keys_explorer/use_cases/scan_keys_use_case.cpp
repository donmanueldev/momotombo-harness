#include "app/keys_explorer/use_cases/scan_keys_use_case.h"

#include <exception>

namespace redisforge::app::keys_explorer
{
    ScanKeysUseCase::ScanKeysUseCase(
        const connections::ActiveConnectionStore& activeConnectionStore,
        const domain::keys_explorer::IKeyExplorerGateway& keyExplorerGateway,
        shared::logging::ILogger& logger)
        : activeConnectionStore_(activeConnectionStore), keyExplorerGateway_(keyExplorerGateway), logger_(logger)
    {
    }

    shared::result::OperationResult<domain::keys_explorer::KeyScanPage>
    ScanKeysUseCase::execute(
        const ScanKeysQuery& query) const
    {
        const auto profile = activeConnectionStore_.activeConnection();
        if (!profile.has_value())
        {
            return shared::result::OperationResult<domain::keys_explorer::KeyScanPage>::failure(
                {"No active Redis connection. Test a connection first."});
        }

        logger_.info("keys.scan.started", {
                         {"cursor", query.cursor},
                         {"pattern", query.pattern},
                         {"pageSize", std::to_string(query.pageSize)},
                         {"host", profile->host},
                     });

        auto page = domain::keys_explorer::KeyScanPage{};
        try
        {
            page = keyExplorerGateway_.scanKeys(*profile, query.cursor, query.pattern, query.pageSize);
        }
        catch (const std::exception& exception)
        {
            logger_.error("keys.scan.failed", {
                              {"cursor", query.cursor},
                              {"pattern", query.pattern},
                              {"reason", exception.what()},
                          });
            return shared::result::OperationResult<domain::keys_explorer::KeyScanPage>::failure(
                {exception.what()});
        }

        logger_.info("keys.scan.completed", {
                         {"cursor", query.cursor},
                         {"nextCursor", page.nextCursor},
                         {"keyCount", std::to_string(page.entries.size())},
                         {"hasMore", page.hasMore ? "true" : "false"},
                     });

        return shared::result::OperationResult<domain::keys_explorer::KeyScanPage>::success(
            std::move(page));
    }
} // namespace redisforge::app::keys_explorer
