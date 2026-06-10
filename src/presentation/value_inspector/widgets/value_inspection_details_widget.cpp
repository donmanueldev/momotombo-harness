#include "presentation/value_inspector/widgets/value_inspection_details_widget.h"

#include <QHeaderView>
#include <QLabel>
#include <QPlainTextEdit>
#include <QStackedWidget>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

#include "presentation/value_inspector/viewmodels/value_inspector_view_model.h"

namespace
{
QString joinStreamFields(const std::vector<redisforge::domain::value_inspector::HashEntry>& fields)
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
ValueInspectionDetailsWidget::ValueInspectionDetailsWidget(ValueInspectorViewModel& viewModel, QWidget* parent)
    : QWidget(parent), viewModel_(viewModel)
{
    auto* rootLayout = new QVBoxLayout(this);

    selectedKeyLabel_ = new QLabel("Selected key: none", this);
    selectedTypeLabel_ = new QLabel("Type: -", this);

    contentStack_ = new QStackedWidget(this);
    emptyStateLabel_ = new QLabel("Select a key to inspect.", this);
    emptyStateLabel_->setWordWrap(true);
    stringValueTextEdit_ = new QPlainTextEdit(this);
    stringValueTextEdit_->setReadOnly(true);
    tableWidget_ = new QTableWidget(this);
    tableWidget_->horizontalHeader()->setStretchLastSection(true);
    tableWidget_->verticalHeader()->setVisible(false);
    tableWidget_->setSelectionMode(QAbstractItemView::NoSelection);
    tableWidget_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableWidget_->setAlternatingRowColors(true);

    contentStack_->addWidget(emptyStateLabel_);
    contentStack_->addWidget(stringValueTextEdit_);
    contentStack_->addWidget(tableWidget_);

    rootLayout->addWidget(selectedKeyLabel_);
    rootLayout->addWidget(selectedTypeLabel_);
    rootLayout->addWidget(contentStack_);

    connect(&viewModel_, &ValueInspectorViewModel::inspectionChanged, this, [this]() {
        render();
    });

    render();
}

void ValueInspectionDetailsWidget::render()
{
    const auto& data = viewModel_.viewData();

    selectedKeyLabel_->setText(
        data.key.empty() ? "Selected key: none" : QString("Selected key: %1").arg(QString::fromStdString(data.key)));
    selectedTypeLabel_->setText(
        data.key.empty() ? "Type: -" : QString("Type: %1").arg(QString::fromStdString(data.typeLabel)));

    if (data.key.empty())
    {
        emptyStateLabel_->setText("Select a key to inspect.");
        contentStack_->setCurrentWidget(emptyStateLabel_);
        return;
    }

    if (data.typeLabel == "Loading...")
    {
        emptyStateLabel_->setText("Loading value...");
        contentStack_->setCurrentWidget(emptyStateLabel_);
        return;
    }

    switch (data.type)
    {
    case redisforge::domain::value_inspector::RedisValueType::String:
        renderString();
        return;
    case redisforge::domain::value_inspector::RedisValueType::Hash:
        renderHash();
        return;
    case redisforge::domain::value_inspector::RedisValueType::List:
        renderList();
        return;
    case redisforge::domain::value_inspector::RedisValueType::Set:
        renderSet();
        return;
    case redisforge::domain::value_inspector::RedisValueType::SortedSet:
        renderSortedSet();
        return;
    case redisforge::domain::value_inspector::RedisValueType::Stream:
        renderStream();
        return;
    case redisforge::domain::value_inspector::RedisValueType::None:
        emptyStateLabel_->setText("The selected key no longer exists.");
        contentStack_->setCurrentWidget(emptyStateLabel_);
        return;
    case redisforge::domain::value_inspector::RedisValueType::Unsupported:
        emptyStateLabel_->setText("RedisForge does not render this Redis type yet.");
        contentStack_->setCurrentWidget(emptyStateLabel_);
        return;
    }
}

void ValueInspectionDetailsWidget::renderString()
{
    const auto& data = viewModel_.viewData();
    stringValueTextEdit_->setPlainText(QString::fromStdString(data.stringValue));
    contentStack_->setCurrentWidget(stringValueTextEdit_);
}

void ValueInspectionDetailsWidget::renderHash()
{
    const auto& data = viewModel_.viewData();
    clearTable(2, {"Field", "Value"});
    tableWidget_->setRowCount(static_cast<int>(data.hashEntries.size()));
    for (int row = 0; row < static_cast<int>(data.hashEntries.size()); ++row)
    {
        tableWidget_->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(data.hashEntries[static_cast<std::size_t>(row)].field)));
        tableWidget_->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(data.hashEntries[static_cast<std::size_t>(row)].value)));
    }
    contentStack_->setCurrentWidget(tableWidget_);
}

void ValueInspectionDetailsWidget::renderList()
{
    const auto& data = viewModel_.viewData();
    clearTable(2, {"Index", "Value"});
    tableWidget_->setRowCount(static_cast<int>(data.listEntries.size()));
    for (int row = 0; row < static_cast<int>(data.listEntries.size()); ++row)
    {
        const auto& entry = data.listEntries[static_cast<std::size_t>(row)];
        tableWidget_->setItem(row, 0, new QTableWidgetItem(QString::number(static_cast<qulonglong>(entry.index))));
        tableWidget_->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(entry.value)));
    }
    contentStack_->setCurrentWidget(tableWidget_);
}

void ValueInspectionDetailsWidget::renderSet()
{
    const auto& data = viewModel_.viewData();
    clearTable(1, {"Member"});
    tableWidget_->setRowCount(static_cast<int>(data.setMembers.size()));
    for (int row = 0; row < static_cast<int>(data.setMembers.size()); ++row)
    {
        tableWidget_->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(data.setMembers[static_cast<std::size_t>(row)])));
    }
    contentStack_->setCurrentWidget(tableWidget_);
}

void ValueInspectionDetailsWidget::renderSortedSet()
{
    const auto& data = viewModel_.viewData();
    clearTable(2, {"Member", "Score"});
    tableWidget_->setRowCount(static_cast<int>(data.sortedSetEntries.size()));
    for (int row = 0; row < static_cast<int>(data.sortedSetEntries.size()); ++row)
    {
        const auto& entry = data.sortedSetEntries[static_cast<std::size_t>(row)];
        tableWidget_->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(entry.member)));
        tableWidget_->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(entry.scoreText)));
    }
    contentStack_->setCurrentWidget(tableWidget_);
}

void ValueInspectionDetailsWidget::renderStream()
{
    const auto& data = viewModel_.viewData();
    clearTable(2, {"ID", "Fields"});
    tableWidget_->setRowCount(static_cast<int>(data.streamEntries.size()));
    for (int row = 0; row < static_cast<int>(data.streamEntries.size()); ++row)
    {
        const auto& entry = data.streamEntries[static_cast<std::size_t>(row)];
        tableWidget_->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(entry.id)));
        tableWidget_->setItem(row, 1, new QTableWidgetItem(joinStreamFields(entry.fields)));
    }
    contentStack_->setCurrentWidget(tableWidget_);
}

void ValueInspectionDetailsWidget::clearTable(const int columnCount, const QStringList& headers)
{
    tableWidget_->clearContents();
    tableWidget_->setColumnCount(columnCount);
    tableWidget_->setHorizontalHeaderLabels(headers);
    tableWidget_->setRowCount(0);
}
} // namespace redisforge::presentation::value_inspector
