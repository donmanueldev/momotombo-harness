#pragma once

#include <QWidget>

namespace redisforge::presentation::keys_explorer
{
class KeysExplorerViewModel;
}

namespace redisforge::presentation::value_inspector
{
class ValueInspectorViewModel;
class ValueInspectionDetailsWidget;
}

class QLabel;
class QLineEdit;
class QPushButton;
class QSpinBox;
class QTableWidget;

namespace redisforge::presentation::keys_explorer
{
class KeysExplorerWidget final : public QWidget
{
    Q_OBJECT

public:
    explicit KeysExplorerWidget(
        KeysExplorerViewModel& viewModel,
        redisforge::presentation::value_inspector::ValueInspectorViewModel& valueInspectorViewModel,
        QWidget* parent = nullptr);

private:
    KeysExplorerViewModel& viewModel_;
    redisforge::presentation::value_inspector::ValueInspectorViewModel& valueInspectorViewModel_;
    QLabel* connectionLabel_{nullptr};
    QLineEdit* patternEdit_{nullptr};
    QSpinBox* pageSizeSpinBox_{nullptr};
    QPushButton* refreshButton_{nullptr};
    QPushButton* previousPageButton_{nullptr};
    QPushButton* nextPageButton_{nullptr};
    QLabel* paginationLabel_{nullptr};
    QTableWidget* keysTable_{nullptr};
    redisforge::presentation::value_inspector::ValueInspectionDetailsWidget* detailsWidget_{nullptr};
    QLabel* statusLabel_{nullptr};
};
} // namespace redisforge::presentation::keys_explorer
