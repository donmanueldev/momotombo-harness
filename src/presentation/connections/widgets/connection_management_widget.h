#pragma once

#include <QWidget>

#include "presentation/connections/viewmodels/connection_management_view_model.h"

class QLabel;
class QLineEdit;
class QListWidget;
class QPushButton;
class QSpinBox;

namespace redisforge::presentation::connections
{
    class ConnectionManagementWidget final : public QWidget
    {
        Q_OBJECT

    public:
        explicit ConnectionManagementWidget(ConnectionManagementViewModel& viewModel, QWidget* parent = nullptr);

    private:
        [[nodiscard]] ConnectionFormData buildFormData() const;
        void populateForm(const ConnectionFormData& formData);

        ConnectionManagementViewModel& viewModel_;
        QListWidget* profilesList_{nullptr};
        QLabel* paginationLabel_{nullptr};
        QLabel* statusLabel_{nullptr};
        QLineEdit* nameEdit_{nullptr};
        QLineEdit* hostEdit_{nullptr};
        QSpinBox* portSpinBox_{nullptr};
        QLineEdit* usernameEdit_{nullptr};
        QLineEdit* passwordEdit_{nullptr};
        QSpinBox* databaseSpinBox_{nullptr};
        QSpinBox* timeoutSpinBox_{nullptr};
        QPushButton* previousPageButton_{nullptr};
        QPushButton* nextPageButton_{nullptr};
        QPushButton* saveButton_{nullptr};
        QPushButton* testButton_{nullptr};
        QString selectedProfileId_;
        int currentPage_{1};
        int totalPages_{1};
    };
} // namespace redisforge::presentation::connections
