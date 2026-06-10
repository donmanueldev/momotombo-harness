#pragma once

#include <QAbstractTableModel>

#include "domain/keys_explorer/entities/key_scan_page.h"

namespace redisforge::presentation::keys_explorer
{
    class KeysTableModel final : public QAbstractTableModel
    {
        Q_OBJECT

    public:
        explicit KeysTableModel(QObject* parent = nullptr);

        [[nodiscard]] int rowCount(const QModelIndex& parent = QModelIndex()) const override;
        [[nodiscard]] int columnCount(const QModelIndex& parent = QModelIndex()) const override;
        [[nodiscard]] QVariant data(const QModelIndex& index, int role) const override;
        [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

        void setEntries(const std::vector<redisforge::domain::keys_explorer::KeyListEntry>& entries);
        void setTypeForKey(const QString& key, const QString& type);
        [[nodiscard]] QString keyAt(int row) const;

    private:
        struct Row
        {
            QString key;
            QString type{"-"};
        };

        QList<Row> rows_;
    };
} // namespace redisforge::presentation::keys_explorer
