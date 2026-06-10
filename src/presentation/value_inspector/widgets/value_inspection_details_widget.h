#pragma once

#include <QWidget>

namespace redisforge::presentation::value_inspector
{
class ValueInspectorViewModel;
class ValueEntriesTableModel;
}

class QLabel;
class QLineEdit;
class QPlainTextEdit;
class QPushButton;
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
    void clearHashEditor();

    ValueInspectorViewModel& viewModel_;
    QLabel* selectedKeyLabel_{nullptr};
    QLabel* selectedTypeLabel_{nullptr};
    QPushButton* refreshButton_{nullptr};
    QStackedWidget* contentStack_{nullptr};
    QStackedWidget* editorStack_{nullptr};
    QLabel* emptyStateLabel_{nullptr};
    QPlainTextEdit* stringValueTextEdit_{nullptr};
    QTableView* tableView_{nullptr};
    ValueEntriesTableModel* tableModel_{nullptr};
    QWidget* emptyEditorWidget_{nullptr};
    QWidget* stringEditorWidget_{nullptr};
    QWidget* hashEditorWidget_{nullptr};
    QPushButton* saveStringButton_{nullptr};
    QLineEdit* hashFieldEdit_{nullptr};
    QLineEdit* hashValueEdit_{nullptr};
    QPushButton* saveHashFieldButton_{nullptr};
    QPushButton* clearHashEditorButton_{nullptr};
};
} // namespace redisforge::presentation::value_inspector
