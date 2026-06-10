#include "presentation/value_inspector/widgets/value_inspection_details_widget.h"

#include <QHeaderView>
#include <QLabel>
#include <QPlainTextEdit>
#include <QStackedWidget>
#include <QTableView>
#include <QVBoxLayout>

#include "presentation/value_inspector/models/value_entries_table_model.h"
#include "presentation/value_inspector/viewmodels/value_inspector_view_model.h"

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
    tableView_ = new QTableView(this);
    tableModel_ = new ValueEntriesTableModel(this);
    tableView_->setModel(tableModel_);
    tableView_->horizontalHeader()->setStretchLastSection(true);
    tableView_->verticalHeader()->setVisible(false);
    tableView_->setSelectionMode(QAbstractItemView::NoSelection);
    tableView_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableView_->setAlternatingRowColors(true);

    contentStack_->addWidget(emptyStateLabel_);
    contentStack_->addWidget(stringValueTextEdit_);
    contentStack_->addWidget(tableView_);

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
    auto typeLabel = data.key.empty() ? QString("Type: -") : QString("Type: %1").arg(QString::fromStdString(data.typeLabel));
    if (data.hasAdditionalEntries)
    {
        typeLabel.append(QString(" (preview: %1 entries)").arg(static_cast<qulonglong>(data.loadedEntryCount)));
    }
    selectedTypeLabel_->setText(typeLabel);

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
    if (data.isStringTruncated)
    {
        stringValueTextEdit_->appendPlainText("\n\n[preview truncated]");
    }
    contentStack_->setCurrentWidget(stringValueTextEdit_);
}

void ValueInspectionDetailsWidget::renderHash()
{
    const auto& data = viewModel_.viewData();
    tableModel_->setHashEntries(data.hashEntries);
    contentStack_->setCurrentWidget(tableView_);
}

void ValueInspectionDetailsWidget::renderList()
{
    const auto& data = viewModel_.viewData();
    tableModel_->setListEntries(data.listEntries);
    contentStack_->setCurrentWidget(tableView_);
}

void ValueInspectionDetailsWidget::renderSet()
{
    const auto& data = viewModel_.viewData();
    tableModel_->setSetMembers(data.setMembers);
    contentStack_->setCurrentWidget(tableView_);
}

void ValueInspectionDetailsWidget::renderSortedSet()
{
    const auto& data = viewModel_.viewData();
    tableModel_->setSortedSetEntries(data.sortedSetEntries);
    contentStack_->setCurrentWidget(tableView_);
}

void ValueInspectionDetailsWidget::renderStream()
{
    const auto& data = viewModel_.viewData();
    tableModel_->setStreamEntries(data.streamEntries);
    contentStack_->setCurrentWidget(tableView_);
}

} // namespace redisforge::presentation::value_inspector
