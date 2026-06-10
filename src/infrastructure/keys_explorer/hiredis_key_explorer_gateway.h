#pragma once

#include "domain/keys_explorer/services/i_key_explorer_gateway.h"

namespace redisforge::infrastructure::keys_explorer
{
    class HiredisKeyExplorerGateway final : public domain::keys_explorer::IKeyExplorerGateway
    {
    public:
        [[nodiscard]] domain::keys_explorer::KeyScanPage scanKeys(
            const domain::connections::ConnectionProfile& profile,
            const std::string& cursor,
            const std::string& pattern,
            std::size_t pageSize) const override;
    };
} // namespace redisforge::infrastructure::keys_explorer
