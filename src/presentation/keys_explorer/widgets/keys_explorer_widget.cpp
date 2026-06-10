#include "presentation/keys_explorer/widgets/keys_explorer_widget.h"

#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

#include "presentation/keys_explorer/viewmodels/keys_explorer_view_model.h"

namespace redisforge::presentation::keys_explorer
{
KeysExplorerWidget::KeysExplorerWidget(KeysExplorerViewModel& viewModel, QWidget* parent)
    : QWidget(parent), viewModel_(viewModel)
{
    auto* rootLayout = new QVBoxLayout(this);

    connectionLabel_ = new QLabel("Active connection: none", this);

    auto* filtersLayout = new QHBoxLayout();
    patternEdit_ = new QLineEdit(this);
    patternEdit_->setPlaceholderText("Pattern, e.g. user:*");
    patternEdit_->setText("*");
    pageSizeSpinBox_ = new QSpinBox(this);
    pageSizeSpinBox_->setRange(10, 1000);
    pageSizeSpinBox_->setValue(100);
    refreshButton_ = new QPushButton("Refresh", this);
    previousPageButton_ = new QPushButton("Previous", this);
    nextPageButton_ = new QPushButton("Next", this);
    paginationLabel_ = new QLabel("Page 1", this);

    filtersLayout->addWidget(new QLabel("Pattern", this));
    filtersLayout->addWidget(patternEdit_, 1);
    filtersLayout->addWidget(new QLabel("Page size", this));
    filtersLayout->addWidget(pageSizeSpinBox_);
    filtersLayout->addWidget(refreshButton_);
    filtersLayout->addWidget(previousPageButton_);
    filtersLayout->addWidget(nextPageButton_);
    filtersLayout->addWidget(paginationLabel_);

    keysTable_ = new QTableWidget(this);
    keysTable_->setColumnCount(2);
    keysTable_->setHorizontalHeaderLabels({"Key", "Type"});
    keysTable_->horizontalHeader()->setStretchLastSection(true);
    keysTable_->verticalHeader()->setVisible(false);
    keysTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    keysTable_->setSelectionMode(QAbstractItemView::SingleSelection);
    keysTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    keysTable_->setAlternatingRowColors(true);

    auto* detailsLayout = new QVBoxLayout();
    selectedKeyLabel_ = new QLabel("Selected key: none", this);
    selectedTypeLabel_ = new QLabel("Type: -", this);
    stringValueTextEdit_ = new QPlainTextEdit(this);
    stringValueTextEdit_->setReadOnly(true);
    stringValueTextEdit_->setPlaceholderText("String values will appear here.");
    detailsLayout->addWidget(selectedKeyLabel_);
    detailsLayout->addWidget(selectedTypeLabel_);
    detailsLayout->addWidget(stringValueTextEdit_);

    statusLabel_ = new QLabel("Test a connection first.", this);
    statusLabel_->setWordWrap(true);

    rootLayout->addWidget(connectionLabel_);
    rootLayout->addLayout(filtersLayout);
    rootLayout->addWidget(keysTable_, 1);
    rootLayout->addLayout(detailsLayout);
    rootLayout->addWidget(statusLabel_);

    previousPageButton_->setEnabled(false);
    nextPageButton_->setEnabled(false);

    connect(patternEdit_, &QLineEdit::editingFinished, this, [this]() {
        viewModel_.updatePattern(patternEdit_->text());
        viewModel_.refresh();
    });
    connect(pageSizeSpinBox_, &QSpinBox::valueChanged, this, [this](const int value) {
        viewModel_.updatePageSize(value);
    });
    connect(refreshButton_, &QPushButton::clicked, this, [this]() {
        viewModel_.updatePattern(patternEdit_->text());
        viewModel_.refresh();
    });
    connect(previousPageButton_, &QPushButton::clicked, this, [this]() {
        viewModel_.previousPage();
    });
    connect(nextPageButton_, &QPushButton::clicked, this, [this]() {
        viewModel_.nextPage();
    });
    connect(keysTable_, &QTableWidget::itemSelectionChanged, this, [this]() {
        const auto selectedItems = keysTable_->selectedItems();
        if (selectedItems.isEmpty())
        {
            return;
        }

        const auto row = selectedItems.front()->row();
        const auto* keyItem = keysTable_->item(row, 0);
        if (keyItem == nullptr)
        {
            return;
        }

        viewModel_.inspectKey(keyItem->text(), row);
    });

    connect(&viewModel_, &KeysExplorerViewModel::activeConnectionChanged, this, [this](const QString& summary) {
        connectionLabel_->setText(QString("Active connection: %1").arg(summary));
    });
    connect(&viewModel_, &KeysExplorerViewModel::keysChanged, this, [this](const QStringList& keys) {
        keysTable_->clearContents();
        keysTable_->setRowCount(keys.size());
        for (int row = 0; row < keys.size(); ++row)
        {
            auto* keyItem = new QTableWidgetItem(keys.at(row));
            auto* typeItem = new QTableWidgetItem("-");
            keysTable_->setItem(row, 0, keyItem);
            keysTable_->setItem(row, 1, typeItem);
        }
        keysTable_->clearSelection();
    });
    connect(&viewModel_, &KeysExplorerViewModel::keyTypeResolved, this, [this](const int row, const QString& type) {
        if (row < 0 || row >= keysTable_->rowCount())
        {
            return;
        }

        auto* typeItem = keysTable_->item(row, 1);
        if (typeItem == nullptr)
        {
            typeItem = new QTableWidgetItem(type);
            keysTable_->setItem(row, 1, typeItem);
            return;
        }

        typeItem->setText(type);
    });
    connect(&viewModel_, &KeysExplorerViewModel::keyDetailsChanged, this, [this](const QString& key, const QString& type, const QString& value, const bool hasStringValue) {
        selectedKeyLabel_->setText(key.isEmpty() ? "Selected key: none" : QString("Selected key: %1").arg(key));
        selectedTypeLabel_->setText(type.isEmpty() ? "Type: -" : QString("Type: %1").arg(type));
        if (hasStringValue)
        {
            stringValueTextEdit_->setPlainText(value);
            return;
        }

        stringValueTextEdit_->clear();
        if (!key.isEmpty() && !type.isEmpty() && type != "string" && type != "Loading...")
        {
            stringValueTextEdit_->setPlaceholderText("This key type will be rendered in a later commit.");
        }
        else
        {
            stringValueTextEdit_->setPlaceholderText("String values will appear here.");
        }
    });
    connect(&viewModel_, &KeysExplorerViewModel::statusChanged, this, [this](const QString& status) {
        statusLabel_->setText(status);
    });
    connect(&viewModel_, &KeysExplorerViewModel::busyChanged, this, [this](const bool isBusy) {
        refreshButton_->setEnabled(!isBusy);
    });
    connect(&viewModel_, &KeysExplorerViewModel::paginationChanged, this, [this](const int currentPage, const bool hasPreviousPage, const bool hasNextPage) {
        paginationLabel_->setText(QString("Page %1").arg(currentPage));
        previousPageButton_->setEnabled(hasPreviousPage);
        nextPageButton_->setEnabled(hasNextPage);
    });
}
} // namespace redisforge::presentation::keys_explorer
