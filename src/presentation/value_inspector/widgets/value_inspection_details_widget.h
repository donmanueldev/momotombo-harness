#pragma once

#include <QWidget>

namespace redisforge::presentation::value_inspector
{
class ValueInspectorViewModel;
}

class QLabel;
class QPlainTextEdit;
class QStackedWidget;
class QTableWidget;

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
    void clearTable(int columnCount, const QStringList& headers);

    ValueInspectorViewModel& viewModel_;
    QLabel* selectedKeyLabel_{nullptr};
    QLabel* selectedTypeLabel_{nullptr};
    QStackedWidget* contentStack_{nullptr};
    QLabel* emptyStateLabel_{nullptr};
    QPlainTextEdit* stringValueTextEdit_{nullptr};
    QTableWidget* tableWidget_{nullptr};
};
} // namespace redisforge::presentation::value_inspector
