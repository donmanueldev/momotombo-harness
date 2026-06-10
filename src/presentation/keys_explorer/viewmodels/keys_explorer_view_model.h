#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <vector>

#include "app/keys_explorer/use_cases/scan_keys_use_case.h"

namespace redisforge::presentation::keys_explorer
{
    class KeysExplorerViewModel final : public QObject
    {
        Q_OBJECT

    public:
        KeysExplorerViewModel(
            app::keys_explorer::ScanKeysUseCase& scanKeysUseCase,
            QObject* parent = nullptr);

        void refresh();
        void nextPage();
        void previousPage();
        void updatePattern(const QString& pattern);
        void updatePageSize(int pageSize);

    public slots:
        void onActiveConnectionChanged(const QString& connectionSummary);

    signals:
        void busyChanged(bool isBusy);
        void statusChanged(const QString& status);
        void keysChanged(const QStringList& keys);
        void paginationChanged(int currentPage, bool hasPreviousPage, bool hasNextPage);
        void activeConnectionChanged(const QString& connectionSummary);

    private:
        void loadCursor(const std::string& cursor, int pageNumber);

        app::keys_explorer::ScanKeysUseCase& scanKeysUseCase_;
        std::vector<std::string> pageCursors_{"0"};
        QString pattern_{"*"};
        int pageSize_{100};
        int currentPage_{1};
        bool busy_{false};
    };
} // namespace redisforge::presentation::keys_explorer
