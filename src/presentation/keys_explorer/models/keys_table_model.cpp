#include "presentation/keys_explorer/models/keys_table_model.h"

namespace redisforge::presentation::keys_explorer
{
    KeysTableModel::KeysTableModel(QObject* parent)
        : QAbstractTableModel(parent)
    {
    }

    int KeysTableModel::rowCount(const QModelIndex& parent) const
    {
        return parent.isValid() ? 0 : rows_.size();
    }

    int KeysTableModel::columnCount(const QModelIndex& parent) const
    {
        return parent.isValid() ? 0 : 2;
    }

    QVariant KeysTableModel::data(const QModelIndex& index, const int role) const
    {
        if (!index.isValid() || role != Qt::DisplayRole)
        {
            return {};
        }

        const auto& row = rows_.at(index.row());
        return index.column() == 0 ? QVariant{row.key} : QVariant{row.type};
    }

    QVariant KeysTableModel::headerData(const int section, const Qt::Orientation orientation, const int role) const
    {
        if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        {
            return {};
        }

        return section == 0 ? QVariant{"Key"} : QVariant{"Type"};
    }

    void KeysTableModel::setKeys(const QStringList& keys)
    {
        beginResetModel();
        rows_.clear();
        rows_.reserve(keys.size());
        for (const auto& key : keys)
        {
            rows_.push_back({key, "-"});
        }
        endResetModel();
    }

    void KeysTableModel::setTypeForKey(const QString& key, const QString& type)
    {
        for (auto index = 0; index < rows_.size(); ++index)
        {
            if (rows_[index].key != key)
            {
                continue;
            }

            rows_[index].type = type;
            emit dataChanged(this->index(index, 1), this->index(index, 1), {Qt::DisplayRole});
            return;
        }
    }

    QString KeysTableModel::keyAt(const int row) const
    {
        if (row < 0 || row >= rows_.size())
        {
            return {};
        }

        return rows_.at(row).key;
    }
} // namespace redisforge::presentation::keys_explorer
