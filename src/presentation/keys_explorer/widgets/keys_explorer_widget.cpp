#include "presentation/keys_explorer/widgets/keys_explorer_widget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QSpinBox>
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

    keysList_ = new QListWidget(this);
    statusLabel_ = new QLabel("Test a connection first.", this);
    statusLabel_->setWordWrap(true);

    rootLayout->addWidget(connectionLabel_);
    rootLayout->addLayout(filtersLayout);
    rootLayout->addWidget(keysList_, 1);
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

    connect(&viewModel_, &KeysExplorerViewModel::activeConnectionChanged, this, [this](const QString& summary) {
        connectionLabel_->setText(QString("Active connection: %1").arg(summary));
    });
    connect(&viewModel_, &KeysExplorerViewModel::keysChanged, this, [this](const QStringList& keys) {
        keysList_->clear();
        keysList_->addItems(keys);
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
