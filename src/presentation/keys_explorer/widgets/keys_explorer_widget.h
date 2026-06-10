#pragma once

#include <QWidget>

namespace redisforge::presentation::keys_explorer
{
class KeysExplorerViewModel;
}

class QLabel;
class QLineEdit;
class QPushButton;
class QPlainTextEdit;
class QSpinBox;
class QTableWidget;

namespace redisforge::presentation::keys_explorer
{
class KeysExplorerWidget final : public QWidget
{
    Q_OBJECT

public:
    explicit KeysExplorerWidget(KeysExplorerViewModel& viewModel, QWidget* parent = nullptr);

private:
    KeysExplorerViewModel& viewModel_;
    QLabel* connectionLabel_{nullptr};
    QLineEdit* patternEdit_{nullptr};
    QSpinBox* pageSizeSpinBox_{nullptr};
    QPushButton* refreshButton_{nullptr};
    QPushButton* previousPageButton_{nullptr};
    QPushButton* nextPageButton_{nullptr};
    QLabel* paginationLabel_{nullptr};
    QTableWidget* keysTable_{nullptr};
    QLabel* selectedKeyLabel_{nullptr};
    QLabel* selectedTypeLabel_{nullptr};
    QPlainTextEdit* stringValueTextEdit_{nullptr};
    QLabel* statusLabel_{nullptr};
};
} // namespace redisforge::presentation::keys_explorer
