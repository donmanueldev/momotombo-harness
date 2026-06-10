#pragma once

#include <QWidget>

namespace redisforge::presentation::value_inspector
{
class ValueInspectorViewModel;
class ValueEntriesTableModel;
}

class QLabel;
class QPlainTextEdit;
class QStackedWidget;
class QTableView;

namespace redisforge::presentation::value_inspector
{
class ValueInspectionDetailsWidget final : public QWidget
{
    Q_OBJECT

public:
    explicit ValueInspectionDetailsWidget(ValueInspectorViewModel& viewModel, QWidget* parent = nullptr);

private:
    void render();
    void renderString();
    void renderHash();
    void renderList();
    void renderSet();
    void renderSortedSet();
    void renderStream();

    ValueInspectorViewModel& viewModel_;
    QLabel* selectedKeyLabel_{nullptr};
    QLabel* selectedTypeLabel_{nullptr};
    QStackedWidget* contentStack_{nullptr};
    QLabel* emptyStateLabel_{nullptr};
    QPlainTextEdit* stringValueTextEdit_{nullptr};
    QTableView* tableView_{nullptr};
    ValueEntriesTableModel* tableModel_{nullptr};
};
} // namespace redisforge::presentation::value_inspector
