#include "presentation/value_inspector/widgets/value_inspection_details_widget.h"

#include <QFontDatabase>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QItemSelectionModel>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
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
    setObjectName("inspectorDetails");
    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(12);

    auto* headerLayout = new QHBoxLayout();
    auto* headerTextLayout = new QVBoxLayout();
    auto* headerEyebrow = new QLabel("Inspector", this);
    headerEyebrow->setObjectName("panelEyebrow");
    selectedKeyLabel_ = new QLabel("Selected key: none", this);
    selectedKeyLabel_->setObjectName("panelTitle");
    selectedTypeLabel_ = new QLabel("Type: -", this);
    selectedTypeLabel_->setObjectName("metaLabel");
    refreshButton_ = new QPushButton("Reload value", this);
    refreshButton_->setObjectName("ghostButton");
    headerTextLayout->addWidget(headerEyebrow);
    headerTextLayout->addWidget(selectedKeyLabel_);
    headerTextLayout->addWidget(selectedTypeLabel_);
    headerLayout->addLayout(headerTextLayout, 1);
    headerLayout->addWidget(refreshButton_);

    contentStack_ = new QStackedWidget(this);
    emptyStateLabel_ = new QLabel("Select a key to inspect.", this);
    emptyStateLabel_->setObjectName("emptyStateBody");
    emptyStateLabel_->setWordWrap(true);
    stringValueTextEdit_ = new QPlainTextEdit(this);
    stringValueTextEdit_->setMinimumHeight(280);
    stringValueTextEdit_->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    tableView_ = new QTableView(this);
    tableModel_ = new ValueEntriesTableModel(this);
    tableView_->setModel(tableModel_);
    tableView_->horizontalHeader()->setStretchLastSection(true);
    tableView_->verticalHeader()->setVisible(false);
    tableView_->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView_->setSelectionMode(QAbstractItemView::SingleSelection);
    tableView_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableView_->setAlternatingRowColors(true);
    tableView_->setShowGrid(false);

    contentStack_->addWidget(emptyStateLabel_);
    contentStack_->addWidget(stringValueTextEdit_);
    contentStack_->addWidget(tableView_);

    editorStack_ = new QStackedWidget(this);
    emptyEditorWidget_ = new QWidget(this);
    stringEditorWidget_ = new QWidget(this);
    auto* stringEditorLayout = new QHBoxLayout(stringEditorWidget_);
    stringEditorLayout->setContentsMargins(0, 0, 0, 0);
    saveStringButton_ = new QPushButton("Save string", stringEditorWidget_);
    saveStringButton_->setObjectName("primaryButton");
    stringEditorLayout->addStretch(1);
    stringEditorLayout->addWidget(saveStringButton_);

    hashEditorWidget_ = new QWidget(this);
    auto* hashEditorLayout = new QHBoxLayout(hashEditorWidget_);
    hashEditorLayout->setContentsMargins(0, 0, 0, 0);
    hashFieldEdit_ = new QLineEdit(hashEditorWidget_);
    hashFieldEdit_->setPlaceholderText("Field");
    hashValueEdit_ = new QLineEdit(hashEditorWidget_);
    hashValueEdit_->setPlaceholderText("Value");
    saveHashFieldButton_ = new QPushButton("Save field", hashEditorWidget_);
    saveHashFieldButton_->setObjectName("primaryButton");
    clearHashEditorButton_ = new QPushButton("Clear", hashEditorWidget_);
    clearHashEditorButton_->setObjectName("ghostButton");
    hashEditorLayout->addWidget(hashFieldEdit_, 1);
    hashEditorLayout->addWidget(hashValueEdit_, 2);
    hashEditorLayout->addWidget(saveHashFieldButton_);
    hashEditorLayout->addWidget(clearHashEditorButton_);

    editorStack_->addWidget(emptyEditorWidget_);
    editorStack_->addWidget(stringEditorWidget_);
    editorStack_->addWidget(hashEditorWidget_);

    rootLayout->addLayout(headerLayout);
    rootLayout->addWidget(contentStack_);
    rootLayout->addWidget(editorStack_);

    connect(refreshButton_, &QPushButton::clicked, this, [this]() {
        viewModel_.refreshCurrentKey();
    });
    connect(saveStringButton_, &QPushButton::clicked, this, [this]() {
        viewModel_.saveStringValue(stringValueTextEdit_->toPlainText());
    });
    connect(saveHashFieldButton_, &QPushButton::clicked, this, [this]() {
        viewModel_.saveHashField(hashFieldEdit_->text(), hashValueEdit_->text());
    });
    connect(clearHashEditorButton_, &QPushButton::clicked, this, [this]() {
        clearHashEditor();
    });
    connect(tableView_->selectionModel(), &QItemSelectionModel::selectionChanged, this, [this]() {
        const auto& data = viewModel_.viewData();
        if (data.type != redisforge::domain::value_inspector::RedisValueType::Hash)
        {
            return;
        }

        const auto selectedRows = tableView_->selectionModel()->selectedRows();
        if (selectedRows.isEmpty())
        {
            clearHashEditor();
            return;
        }

        if (!selectedRows.front().isValid())
        {
            clearHashEditor();
            return;
        }

        const auto rowValues = tableModel_->rowValuesAt(selectedRows.front().row());
        if (rowValues.size() < 2)
        {
            clearHashEditor();
            return;
        }

        hashFieldEdit_->setText(rowValues.at(0));
        hashValueEdit_->setText(rowValues.at(1));
    });

    connect(&viewModel_, &ValueInspectorViewModel::inspectionChanged, this, [this]() {
        render();
    });

    render();
}

void ValueInspectionDetailsWidget::render()
{
    const auto& data = viewModel_.viewData();

    selectedKeyLabel_->setText(
        data.key.empty() ? "No key selected" : QString::fromStdString(data.key));
    auto typeLabel =
        data.key.empty() ? QString("Type: -") : QString("Type: %1").arg(QString::fromStdString(data.typeLabel));
    if (data.hasAdditionalEntries)
    {
        typeLabel.append(QString("   Preview entries: %1").arg(static_cast<qulonglong>(data.loadedEntryCount)));
    }
    selectedTypeLabel_->setText(typeLabel);
    refreshButton_->setEnabled(!data.key.empty());

    if (data.key.empty())
    {
        emptyStateLabel_->setText("Select a key to inspect.");
        contentStack_->setCurrentWidget(emptyStateLabel_);
        editorStack_->setCurrentWidget(emptyEditorWidget_);
        return;
    }

    if (data.typeLabel == "Loading...")
    {
        emptyStateLabel_->setText("Loading value...");
        contentStack_->setCurrentWidget(emptyStateLabel_);
        editorStack_->setCurrentWidget(emptyEditorWidget_);
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
        editorStack_->setCurrentWidget(emptyEditorWidget_);
        return;
    case redisforge::domain::value_inspector::RedisValueType::Unsupported:
        emptyStateLabel_->setText("RedisForge does not render this Redis type yet.");
        contentStack_->setCurrentWidget(emptyStateLabel_);
        editorStack_->setCurrentWidget(emptyEditorWidget_);
        return;
    }
}

void ValueInspectionDetailsWidget::renderString()
{
    const auto& data = viewModel_.viewData();
    stringValueTextEdit_->setReadOnly(data.isStringTruncated);
    stringValueTextEdit_->setPlainText(QString::fromStdString(data.stringValue));
    if (data.isStringTruncated)
    {
        stringValueTextEdit_->appendPlainText("\n\n[truncated]");
    }
    contentStack_->setCurrentWidget(stringValueTextEdit_);
    editorStack_->setCurrentWidget(stringEditorWidget_);
    saveStringButton_->setEnabled(!data.isStringTruncated);
    saveStringButton_->setToolTip(
        data.isStringTruncated ? "Increase the preview limit before editing this string." : QString{});
}

void ValueInspectionDetailsWidget::renderHash()
{
    const auto& data = viewModel_.viewData();
    tableModel_->setHashEntries(data.hashEntries);
    contentStack_->setCurrentWidget(tableView_);
    editorStack_->setCurrentWidget(hashEditorWidget_);
    clearHashEditor();
}

void ValueInspectionDetailsWidget::renderList()
{
    const auto& data = viewModel_.viewData();
    tableModel_->setListEntries(data.listEntries);
    contentStack_->setCurrentWidget(tableView_);
    editorStack_->setCurrentWidget(emptyEditorWidget_);
}

void ValueInspectionDetailsWidget::renderSet()
{
    const auto& data = viewModel_.viewData();
    tableModel_->setSetMembers(data.setMembers);
    contentStack_->setCurrentWidget(tableView_);
    editorStack_->setCurrentWidget(emptyEditorWidget_);
}

void ValueInspectionDetailsWidget::renderSortedSet()
{
    const auto& data = viewModel_.viewData();
    tableModel_->setSortedSetEntries(data.sortedSetEntries);
    contentStack_->setCurrentWidget(tableView_);
    editorStack_->setCurrentWidget(emptyEditorWidget_);
}

void ValueInspectionDetailsWidget::renderStream()
{
    const auto& data = viewModel_.viewData();
    tableModel_->setStreamEntries(data.streamEntries);
    contentStack_->setCurrentWidget(tableView_);
    editorStack_->setCurrentWidget(emptyEditorWidget_);
}

void ValueInspectionDetailsWidget::clearHashEditor()
{
    hashFieldEdit_->clear();
    hashValueEdit_->clear();
}

} // namespace redisforge::presentation::value_inspector
