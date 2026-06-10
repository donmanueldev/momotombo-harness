#pragma once

#include <QAbstractTableModel>
#include <QStringList>

#include "presentation/value_inspector/viewmodels/value_inspection_view_data.h"

namespace redisforge::presentation::value_inspector
{
class ValueEntriesTableModel final : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit ValueEntriesTableModel(QObject* parent = nullptr);

    [[nodiscard]] int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    [[nodiscard]] int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    [[nodiscard]] QVariant data(const QModelIndex& index, int role) const override;
    [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    void clear();
    void setHashEntries(const std::vector<redisforge::domain::value_inspector::HashEntry>& entries);
    void setListEntries(const std::vector<redisforge::domain::value_inspector::ListEntry>& entries);
    void setSetMembers(const std::vector<std::string>& members);
    void setSortedSetEntries(const std::vector<redisforge::domain::value_inspector::SortedSetEntry>& entries);
    void setStreamEntries(const std::vector<redisforge::domain::value_inspector::StreamEntry>& entries);

private:
    QStringList headers_;
    QList<QStringList> rows_;
};
} // namespace redisforge::presentation::value_inspector
