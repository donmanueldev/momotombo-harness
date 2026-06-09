#include "app/connections/services/active_connection_store.h"

namespace redisforge::app::connections
{
void ActiveConnectionStore::setActiveConnection(redisforge::domain::connections::ConnectionProfile profile)
{
    activeConnection_ = std::move(profile);
}

void ActiveConnectionStore::clear()
{
    activeConnection_.reset();
}

bool ActiveConnectionStore::hasActiveConnection() const
{
    return activeConnection_.has_value();
}

std::optional<redisforge::domain::connections::ConnectionProfile> ActiveConnectionStore::activeConnection() const
{
    return activeConnection_;
}
} // namespace redisforge::app::connections
