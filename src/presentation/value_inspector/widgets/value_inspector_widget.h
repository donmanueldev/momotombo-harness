#pragma once

#include <QWidget>

namespace redisforge::presentation::value_inspector
{
class ValueInspectorViewModel;
class ValueInspectionDetailsWidget;
}

class QLabel;
class QLineEdit;
class QPushButton;

namespace redisforge::presentation::value_inspector
{
class ValueInspectorWidget final : public QWidget
{
    Q_OBJECT

public:
    explicit ValueInspectorWidget(ValueInspectorViewModel& viewModel, QWidget* parent = nullptr);

private:
    ValueInspectorViewModel& viewModel_;
    QLabel* connectionLabel_{nullptr};
    QLineEdit* keyEdit_{nullptr};
    QPushButton* inspectButton_{nullptr};
    ValueInspectionDetailsWidget* detailsWidget_{nullptr};
    QLabel* statusLabel_{nullptr};
};
} // namespace redisforge::presentation::value_inspector
