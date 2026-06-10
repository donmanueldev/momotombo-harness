#include "presentation/value_inspector/widgets/value_inspector_widget.h"

#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

#include "presentation/value_inspector/viewmodels/value_inspector_view_model.h"
#include "presentation/value_inspector/widgets/value_inspection_details_widget.h"

namespace redisforge::presentation::value_inspector
{
ValueInspectorWidget::ValueInspectorWidget(ValueInspectorViewModel& viewModel, QWidget* parent)
    : QWidget(parent), viewModel_(viewModel)
{
    auto* rootLayout = new QVBoxLayout(this);

    connectionLabel_ = new QLabel(
        QString("Active connection: %1").arg(viewModel_.activeConnectionSummary()),
        this);

    auto* controlsLayout = new QHBoxLayout();
    keyEdit_ = new QLineEdit(this);
    keyEdit_->setPlaceholderText("Redis key, e.g. user:1");
    inspectButton_ = new QPushButton("Inspect", this);
    controlsLayout->addWidget(new QLabel("Key", this));
    controlsLayout->addWidget(keyEdit_, 1);
    controlsLayout->addWidget(inspectButton_);

    detailsWidget_ = new ValueInspectionDetailsWidget(viewModel_, this);
    statusLabel_ = new QLabel("Enter a key to inspect.", this);
    statusLabel_->setWordWrap(true);

    rootLayout->addWidget(connectionLabel_);
    rootLayout->addLayout(controlsLayout);
    rootLayout->addWidget(detailsWidget_, 1);
    rootLayout->addWidget(statusLabel_);

    connect(inspectButton_, &QPushButton::clicked, this, [this]() {
        viewModel_.inspectKey(keyEdit_->text());
    });
    connect(keyEdit_, &QLineEdit::returnPressed, this, [this]() {
        viewModel_.inspectKey(keyEdit_->text());
    });
    connect(&viewModel_, &ValueInspectorViewModel::busyChanged, this, [this](const bool isBusy) {
        inspectButton_->setEnabled(!isBusy);
    });
    connect(&viewModel_, &ValueInspectorViewModel::statusChanged, this, [this](const QString& status) {
        statusLabel_->setText(status);
    });
    connect(&viewModel_, &ValueInspectorViewModel::activeConnectionChanged, this, [this](const QString& summary) {
        connectionLabel_->setText(QString("Active connection: %1").arg(summary));
    });
}
} // namespace redisforge::presentation::value_inspector
