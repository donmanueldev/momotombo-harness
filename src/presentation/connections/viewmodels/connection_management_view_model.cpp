#include "presentation/connections/viewmodels/connection_management_view_model.h"

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
        app::connections::TestAndActivateConnectionUseCase& testAndActivateConnectionUseCase,
        shared::logging::ILogger& logger,
        QObject* parent)
        : QObject(parent),
          listProfilesUseCase_(listProfilesUseCase),
          saveProfileUseCase_(saveProfileUseCase),
          testAndActivateConnectionUseCase_(testAndActivateConnectionUseCase),
          logger_(logger)
    {
    }

    void ConnectionManagementViewModel::loadProfiles(const std::size_t page)
    {
        if (busy_)
        {
            return;
        }

        ++activeRequestId_;
        const auto requestId = activeRequestId_;
        currentPage_ = std::max<std::size_t>(1U, page);
        busy_ = true;
        emit busyChanged(true);
        emit notificationChanged("Loading connection profiles...");

        auto* watcher =
            new QFutureWatcher<shared::pagination::Page<domain::connections::ConnectionProfile>>{this};
        connect(
            watcher,
            &QFutureWatcher<shared::pagination::Page<domain::connections::ConnectionProfile>>::finished,
            this,
            [this, watcher, requestId]()
            {
                if (requestId != activeRequestId_)
                {
                    watcher->deleteLater();
                    return;
                }

                busy_ = false;
                emit busyChanged(false);

                auto result = watcher->result();
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
                    };

                    currentProfiles_.push_back(formData);
                    listItems.push_back(toListItem(formData));
                }

                emit profilesChanged(listItems);
                emit paginationChanged(static_cast<int>(result.page), static_cast<int>(result.totalPages()));
                emit notificationChanged(
                    listItems.isEmpty()
                        ? "No saved connection profiles."
                        : QString("Loaded %1 connection profile(s).").arg(listItems.size()));
                watcher->deleteLater();
            });

        const auto pageToLoad = currentPage_;
        const auto pageSize = pageSize_;
        auto* useCase = &listProfilesUseCase_;
        watcher->setFuture(QtConcurrent::run([useCase, pageToLoad, pageSize]()
        {
            return useCase->execute(pageToLoad, pageSize);
        }));
    }

    void ConnectionManagementViewModel::saveConnection(const ConnectionFormData& formData)
    {
        if (busy_)
        {
            emit notificationChanged("An operation is already running.");
            return;
        }

        ++activeRequestId_;
        const auto requestId = activeRequestId_;
        busy_ = true;
        emit busyChanged(true);
        emit notificationChanged("Saving connection profile...");

        const auto profile = toDomain(formData);
        auto* watcher = new QFutureWatcher<shared::result::OperationResult<std::string>>{this};
        connect(
            watcher,
            &QFutureWatcher<shared::result::OperationResult<std::string>>::finished,
            this,
            [this, watcher, requestId]()
            {
                if (requestId != activeRequestId_)
                {
                    watcher->deleteLater();
                    return;
                }

                busy_ = false;
                emit busyChanged(false);

                const auto result = watcher->result();
                if (!result.isSuccessful)
                {
                    emit notificationChanged(QString::fromStdString(
                        result.errors.empty() ? "Unable to save connection." : result.errors.front()));
                    watcher->deleteLater();
                    return;
                }

                emit notificationChanged("Connection profile saved.");
                watcher->deleteLater();
                loadProfiles(currentPage_);
            });

        auto* useCase = &saveProfileUseCase_;
        watcher->setFuture(QtConcurrent::run([useCase, profile]()
        {
            return useCase->execute(profile);
        }));
    }

    void ConnectionManagementViewModel::testConnection(const ConnectionFormData& formData)
    {
        if (busy_)
        {
            emit notificationChanged("A connection test is already running.");
            return;
        }

        ++activeRequestId_;
        const auto requestId = activeRequestId_;
        busy_ = true;
        emit busyChanged(true);
        emit notificationChanged("Testing Redis connection...");

        const auto profile = toDomain(formData);
        auto* watcher = new QFutureWatcher<app::connections::TestAndActivateConnectionResult>{this};
        connect(
            watcher,
            &QFutureWatcher<app::connections::TestAndActivateConnectionResult>::finished,
            this,
            [this, watcher, requestId]()
            {
                if (requestId != activeRequestId_)
                {
                    watcher->deleteLater();
                    return;
                }

                busy_ = false;
                emit busyChanged(false);

                const auto result = watcher->result();
                emit notificationChanged(QString::fromStdString(result.testResult.message));
                if (result.activeConnection.has_value())
                {
                    emit activeConnectionChanged(QString::fromStdString(result.activeConnection->summary));
                }

                emit connectionTestCompleted(
                    result.testResult.isSuccessful,
                    QString::fromStdString(result.testResult.message),
                    result.testResult.latency.count(),
                    QString::fromStdString(result.testResult.serverVersion));
                watcher->deleteLater();
            });

        auto* useCase = &testAndActivateConnectionUseCase_;
        watcher->setFuture(QtConcurrent::run([useCase, profile]() { return useCase->execute(profile); }));
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
        };
    }
} // namespace redisforge::presentation::connections
