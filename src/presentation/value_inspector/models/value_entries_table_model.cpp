#include "presentation/value_inspector/models/value_entries_table_model.h"

namespace
{
QString joinFields(const std::vector<redisforge::domain::value_inspector::HashEntry>& fields)
{
    auto lines = QStringList{};
    lines.reserve(static_cast<qsizetype>(fields.size()));
    for (const auto& field : fields)
    {
        lines.push_back(QString("%1: %2").arg(QString::fromStdString(field.field), QString::fromStdString(field.value)));
    }

    return lines.join('\n');
}
} // namespace

namespace redisforge::presentation::value_inspector
{
ValueEntriesTableModel::ValueEntriesTableModel(QObject* parent)
    : QAbstractTableModel(parent)
{
}

int ValueEntriesTableModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : rows_.size();
}

int ValueEntriesTableModel::columnCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : headers_.size();
}

QVariant ValueEntriesTableModel::data(const QModelIndex& index, const int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
    {
        return {};
    }

    if (index.row() < 0 || index.row() >= rows_.size())
    {
        return {};
    }

    return rows_.at(index.row()).value(index.column());
}

QVariant ValueEntriesTableModel::headerData(const int section, const Qt::Orientation orientation, const int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
    {
        return {};
    }

    return headers_.value(section);
}

void ValueEntriesTableModel::clear()
{
    beginResetModel();
    headers_.clear();
    rows_.clear();
    endResetModel();
}

void ValueEntriesTableModel::setHashEntries(const std::vector<redisforge::domain::value_inspector::HashEntry>& entries)
{
    beginResetModel();
    headers_ = {"Field", "Value"};
    rows_.clear();
    rows_.reserve(static_cast<qsizetype>(entries.size()));
    for (const auto& entry : entries)
    {
        rows_.push_back({QString::fromStdString(entry.field), QString::fromStdString(entry.value)});
    }
    endResetModel();
}

void ValueEntriesTableModel::setListEntries(const std::vector<redisforge::domain::value_inspector::ListEntry>& entries)
{
    beginResetModel();
    headers_ = {"Index", "Value"};
    rows_.clear();
    rows_.reserve(static_cast<qsizetype>(entries.size()));
    for (const auto& entry : entries)
    {
        rows_.push_back({QString::number(static_cast<qulonglong>(entry.index)), QString::fromStdString(entry.value)});
    }
    endResetModel();
}

void ValueEntriesTableModel::setSetMembers(const std::vector<std::string>& members)
{
    beginResetModel();
    headers_ = {"Member"};
    rows_.clear();
    rows_.reserve(static_cast<qsizetype>(members.size()));
    for (const auto& member : members)
    {
        rows_.push_back({QString::fromStdString(member)});
    }
    endResetModel();
}

void ValueEntriesTableModel::setSortedSetEntries(
    const std::vector<redisforge::domain::value_inspector::SortedSetEntry>& entries)
{
    beginResetModel();
    headers_ = {"Member", "Score"};
    rows_.clear();
    rows_.reserve(static_cast<qsizetype>(entries.size()));
    for (const auto& entry : entries)
    {
        rows_.push_back({QString::fromStdString(entry.member), QString::fromStdString(entry.scoreText)});
    }
    endResetModel();
}

void ValueEntriesTableModel::setStreamEntries(
    const std::vector<redisforge::domain::value_inspector::StreamEntry>& entries)
{
    beginResetModel();
    headers_ = {"ID", "Fields"};
    rows_.clear();
    rows_.reserve(static_cast<qsizetype>(entries.size()));
    for (const auto& entry : entries)
    {
        rows_.push_back({QString::fromStdString(entry.id), joinFields(entry.fields)});
    }
    endResetModel();
}

QStringList ValueEntriesTableModel::rowValuesAt(const int row) const
{
    if (row < 0 || row >= rows_.size())
    {
        return {};
    }

    return rows_.at(row);
}
} // namespace redisforge::presentation::value_inspector
