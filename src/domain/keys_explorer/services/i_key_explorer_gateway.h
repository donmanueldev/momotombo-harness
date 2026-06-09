#pragma once

#include <cstddef>
#include <string>

#include "domain/connections/entities/connection_profile.h"
#include "domain/keys_explorer/entities/key_scan_page.h"

namespace redisforge::domain::keys_explorer
{
class IKeyExplorerGateway
{
public:
    virtual ~IKeyExplorerGateway() = default;

    virtual KeyScanPage scanKeys(
        const redisforge::domain::connections::ConnectionProfile& profile,
        const std::string& cursor,
        const std::string& pattern,
        std::size_t pageSize) const = 0;
};
} // namespace redisforge::domain::keys_explorer
