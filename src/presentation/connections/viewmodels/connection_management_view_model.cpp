#include "presentation/connections/viewmodels/connection_management_view_model.h"

#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrent>

#include <algorithm>

namespace redisforge::presentation::connections
{
namespace
{
ConnectionListItem toListItem(const ConnectionFormData& formData)
{
    return ConnectionListItem{
        formData.id,
        formData.name,
        QString("%1:%2 / db%3")
            .arg(formData.host)
            .arg(formData.port)
            .arg(formData.databaseIndex),
    };
}
} // namespace

ConnectionManagementViewModel::ConnectionManagementViewModel(
    app::connections::ListConnectionProfilesUseCase& listProfilesUseCase,
    app::connections::SaveConnectionProfileUseCase& saveProfileUseCase,
    app::connections::TestConnectionUseCase& testConnectionUseCase,
    app::connections::ActiveConnectionStore& activeConnectionStore,
    shared::logging::ILogger& logger,
    QObject* parent)
    : QObject(parent),
      listProfilesUseCase_(listProfilesUseCase),
      saveProfileUseCase_(saveProfileUseCase),
      testConnectionUseCase_(testConnectionUseCase),
      activeConnectionStore_(activeConnectionStore),
      logger_(logger)
{
}

void ConnectionManagementViewModel::loadProfiles(const std::size_t page)
{
    currentPage_ = std::max<std::size_t>(1U, page);
    const auto result = listProfilesUseCase_.execute(currentPage_, pageSize_);

    currentProfiles_.clear();
    currentProfiles_.reserve(static_cast<qsizetype>(result.items.size()));

    auto listItems = QVector<ConnectionListItem>{};
    listItems.reserve(static_cast<qsizetype>(result.items.size()));

    for (const auto& profile : result.items)
    {
        auto formData = ConnectionFormData{
            QString::fromStdString(profile.id),
            QString::fromStdString(profile.name),
            QString::fromStdString(profile.host),
            profile.port,
            QString::fromStdString(profile.username),
            QString::fromStdString(profile.password),
            profile.databaseIndex,
            profile.connectTimeoutMs,
            profile.useTls,
        };

        currentProfiles_.push_back(formData);
        listItems.push_back(toListItem(formData));
    }

    const auto totalPages = static_cast<int>(result.totalPages());

    emit profilesChanged(listItems);
    emit paginationChanged(static_cast<int>(result.page), totalPages);
}

void ConnectionManagementViewModel::saveConnection(const ConnectionFormData& formData)
{
    const auto result = saveProfileUseCase_.execute(toDomain(formData));
    if (!result.isSuccessful)
    {
        emit notificationChanged(QString::fromStdString(
            result.errors.empty() ? "Unable to save connection." : result.errors.front()));
        return;
    }

    emit notificationChanged("Connection profile saved.");
    loadProfiles(currentPage_);
}

void ConnectionManagementViewModel::testConnection(const ConnectionFormData& formData)
{
    if (busy_)
    {
        emit notificationChanged("A connection test is already running.");
        return;
    }

    busy_ = true;
    emit busyChanged(true);
    emit notificationChanged("Testing Redis connection...");

    const auto profile = toDomain(formData);
    auto* watcher = new QFutureWatcher<domain::connections::ConnectionTestResult>{this};
    connect(watcher, &QFutureWatcher<domain::connections::ConnectionTestResult>::finished, this, [this, watcher, profile]() {
        const auto result = watcher->result();
        busy_ = false;
        emit busyChanged(false);
        emit notificationChanged(QString::fromStdString(result.message));
        if (result.isSuccessful)
        {
            activeConnectionStore_.setActiveConnection(profile);
            emit activeConnectionChanged(
                QString("%1 (%2:%3 / db%4)")
                    .arg(QString::fromStdString(profile.name))
                    .arg(QString::fromStdString(profile.host))
                    .arg(profile.port)
                    .arg(profile.databaseIndex));
        }
        emit connectionTestCompleted(
            result.isSuccessful,
            QString::fromStdString(result.message),
            result.latency.count(),
            QString::fromStdString(result.serverVersion));
        watcher->deleteLater();
    });

    watcher->setFuture(QtConcurrent::run([this, profile]() { return testConnectionUseCase_.execute(profile); }));
}

ConnectionFormData ConnectionManagementViewModel::profileAt(const int index) const
{
    if (index < 0 || index >= currentProfiles_.size())
    {
        return {};
    }

    return currentProfiles_.at(index);
}

bool ConnectionManagementViewModel::isBusy() const
{
    return busy_;
}

domain::connections::ConnectionProfile ConnectionManagementViewModel::toDomain(const ConnectionFormData& formData)
{
    return domain::connections::ConnectionProfile{
        formData.id.toStdString(),
        formData.name.toStdString(),
        formData.host.toStdString(),
        static_cast<std::uint16_t>(formData.port),
        formData.username.toStdString(),
        formData.password.toStdString(),
        formData.databaseIndex,
        formData.connectTimeoutMs,
        formData.useTls,
    };
}
} // namespace redisforge::presentation::connections
