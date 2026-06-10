#include "app/connections/services/active_connection_store.h"

namespace redisforge::app::connections
{
void ActiveConnectionStore::setActiveConnection(redisforge::domain::connections::ConnectionProfile profile)
{
    auto lock = std::scoped_lock{mutex_};
    activeConnection_ = std::move(profile);
}

void ActiveConnectionStore::clear()
{
    auto lock = std::scoped_lock{mutex_};
    activeConnection_.reset();
}

bool ActiveConnectionStore::hasActiveConnection() const
{
    auto lock = std::scoped_lock{mutex_};
    return activeConnection_.has_value();
}

std::optional<redisforge::domain::connections::ConnectionProfile> ActiveConnectionStore::activeConnection() const
{
    auto lock = std::scoped_lock{mutex_};
    return activeConnection_;
}
} // namespace redisforge::app::connections
