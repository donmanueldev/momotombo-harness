#pragma once

#include <QObject>
#include <QString>
#include <QVector>

#include "app/connections/services/active_connection_store.h"
#include "app/connections/use_cases/list_connection_profiles_use_case.h"
#include "app/connections/use_cases/save_connection_profile_use_case.h"
#include "app/connections/use_cases/test_connection_use_case.h"
#include "shared/logging/logger.h"

namespace redisforge::presentation::connections
{
    struct ConnectionFormData
    {
        QString id;
        QString name;
        QString host;
        quint16 port{6379};
        QString username;
        QString password;
        int databaseIndex{0};
        int connectTimeoutMs{3000};
        bool useTls{false};
    };

    struct ConnectionListItem
    {
        QString id;
        QString title;
        QString subtitle;
    };

    class ConnectionManagementViewModel final : public QObject
    {
        Q_OBJECT

    public:
        ConnectionManagementViewModel(
            app::connections::ListConnectionProfilesUseCase& listProfilesUseCase,
            app::connections::SaveConnectionProfileUseCase& saveProfileUseCase,
            app::connections::TestConnectionUseCase& testConnectionUseCase,
            app::connections::ActiveConnectionStore& activeConnectionStore,
            shared::logging::ILogger& logger,
            QObject* parent = nullptr);

        void loadProfiles(std::size_t page = 1U);
        void saveConnection(const ConnectionFormData& formData);
        void testConnection(const ConnectionFormData& formData);
        [[nodiscard]] ConnectionFormData profileAt(int index) const;
        [[nodiscard]] bool isBusy() const;

    signals:
        void busyChanged(bool isBusy);
        void notificationChanged(const QString& message);
        void profilesChanged(const QVector<ConnectionListItem>& profiles);
        void paginationChanged(int currentPage, int totalPages);
        void connectionTestCompleted(bool isSuccessful, const QString& message, qint64 latencyMs,
                                     const QString& serverVersion);
        void activeConnectionChanged(const QString& connectionSummary);

    private:
        [[nodiscard]] static domain::connections::ConnectionProfile toDomain(const ConnectionFormData& formData);

        app::connections::ListConnectionProfilesUseCase& listProfilesUseCase_;
        app::connections::SaveConnectionProfileUseCase& saveProfileUseCase_;
        app::connections::TestConnectionUseCase& testConnectionUseCase_;
        app::connections::ActiveConnectionStore& activeConnectionStore_;
        shared::logging::ILogger& logger_;
        QVector<ConnectionFormData> currentProfiles_;
        std::size_t currentPage_{1U};
        std::size_t pageSize_{20U};
        bool busy_{false};
    };
} // namespace redisforge::presentation::connections
