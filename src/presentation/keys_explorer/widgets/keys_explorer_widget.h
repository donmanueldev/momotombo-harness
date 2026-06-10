#pragma once

#include <QWidget>

namespace redisforge::presentation::keys_explorer
{
    class KeysExplorerViewModel;
    class KeysTableModel;
}

namespace redisforge::presentation::value_inspector
{
    class ValueInspectorViewModel;
    class ValueInspectionDetailsWidget;
}

class QLabel;
class QLineEdit;
class QPushButton;
class QComboBox;
class QSortFilterProxyModel;
class QSplitter;
class QSpinBox;
class QTableView;

namespace redisforge::presentation::keys_explorer
{
    class KeysExplorerWidget final : public QWidget
    {
        Q_OBJECT

    public:
        explicit KeysExplorerWidget(
            KeysExplorerViewModel& viewModel,
            value_inspector::ValueInspectorViewModel& valueInspectorViewModel,
            QWidget* parent = nullptr);

    private:
        KeysExplorerViewModel& viewModel_;
        value_inspector::ValueInspectorViewModel& valueInspectorViewModel_;
        QLabel* connectionLabel_{nullptr};
        QLineEdit* patternEdit_{nullptr};
        QComboBox* typeFilterComboBox_{nullptr};
        QSpinBox* pageSizeSpinBox_{nullptr};
        QPushButton* refreshButton_{nullptr};
        QPushButton* previousPageButton_{nullptr};
        QPushButton* nextPageButton_{nullptr};
        QLabel* paginationLabel_{nullptr};
        QTableView* keysTable_{nullptr};
        KeysTableModel* keysTableModel_{nullptr};
        QSortFilterProxyModel* keysFilterModel_{nullptr};
        QSplitter* contentSplitter_{nullptr};
        value_inspector::ValueInspectionDetailsWidget* detailsWidget_{nullptr};
        QLabel* statusLabel_{nullptr};
    };
} // namespace redisforge::presentation::keys_explorer
