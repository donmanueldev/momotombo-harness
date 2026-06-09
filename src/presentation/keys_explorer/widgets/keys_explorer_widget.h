#pragma once

#include <QWidget>

namespace redisforge::presentation::keys_explorer
{
class KeysExplorerViewModel;
}

class QLabel;
class QLineEdit;
class QListWidget;
class QPushButton;
class QSpinBox;

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
    QListWidget* keysList_{nullptr};
    QLabel* statusLabel_{nullptr};
};
} // namespace redisforge::presentation::keys_explorer
