#include "presentation/connections/widgets/connection_management_widget.h"

#include <QCheckBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QSpinBox>

namespace redisforge::presentation::connections
{
    ConnectionManagementWidget::ConnectionManagementWidget(ConnectionManagementViewModel& viewModel, QWidget* parent)
        : QWidget(parent), viewModel_(viewModel)
    {
        auto* rootLayout = new QHBoxLayout(this);

        auto* savedProfilesGroup = new QGroupBox("Saved Connections", this);
        auto* savedProfilesLayout = new QVBoxLayout(savedProfilesGroup);
        profilesList_ = new QListWidget(savedProfilesGroup);
        paginationLabel_ = new QLabel("Page 1 / 1", savedProfilesGroup);
        previousPageButton_ = new QPushButton("Previous", savedProfilesGroup);
        nextPageButton_ = new QPushButton("Next", savedProfilesGroup);

        auto* paginationButtonsLayout = new QHBoxLayout();
        paginationButtonsLayout->addWidget(previousPageButton_);
        paginationButtonsLayout->addWidget(nextPageButton_);

        savedProfilesLayout->addWidget(profilesList_);
        savedProfilesLayout->addWidget(paginationLabel_);
        savedProfilesLayout->addLayout(paginationButtonsLayout);

        auto* formGroup = new QGroupBox("Connection Details", this);
        auto* formLayout = new QFormLayout(formGroup);
        nameEdit_ = new QLineEdit(formGroup);
        hostEdit_ = new QLineEdit(formGroup);
        portSpinBox_ = new QSpinBox(formGroup);
        portSpinBox_->setRange(1, 65535);
        portSpinBox_->setValue(6379);
        usernameEdit_ = new QLineEdit(formGroup);
        passwordEdit_ = new QLineEdit(formGroup);
        passwordEdit_->setEchoMode(QLineEdit::Password);
        databaseSpinBox_ = new QSpinBox(formGroup);
        databaseSpinBox_->setRange(0, 1024);
        timeoutSpinBox_ = new QSpinBox(formGroup);
        timeoutSpinBox_->setRange(250, 30000);
        timeoutSpinBox_->setSingleStep(250);
        timeoutSpinBox_->setValue(3000);
        tlsCheckBox_ = new QCheckBox("Enable TLS (placeholder)", formGroup);
        tlsCheckBox_->setEnabled(false);

        formLayout->addRow("Profile name", nameEdit_);
        formLayout->addRow("Host", hostEdit_);
        formLayout->addRow("Port", portSpinBox_);
        formLayout->addRow("Username", usernameEdit_);
        formLayout->addRow("Password", passwordEdit_);
        formLayout->addRow("Database", databaseSpinBox_);
        formLayout->addRow("Timeout (ms)", timeoutSpinBox_);
        formLayout->addRow("", tlsCheckBox_);

        auto* actionLayout = new QHBoxLayout();
        saveButton_ = new QPushButton("Save Profile", this);
        testButton_ = new QPushButton("Test Connection", this);
        actionLayout->addWidget(saveButton_);
        actionLayout->addWidget(testButton_);

        statusLabel_ = new QLabel("Ready.", this);
        statusLabel_->setWordWrap(true);

        auto* rightPaneLayout = new QVBoxLayout();
        rightPaneLayout->addWidget(formGroup);
        rightPaneLayout->addLayout(actionLayout);
        rightPaneLayout->addWidget(statusLabel_);
        rightPaneLayout->addStretch();

        rootLayout->addWidget(savedProfilesGroup, 1);
        rootLayout->addLayout(rightPaneLayout, 2);

        connect(saveButton_, &QPushButton::clicked, this, [this]()
        {
            viewModel_.saveConnection(buildFormData());
        });
        connect(testButton_, &QPushButton::clicked, this, [this]()
        {
            viewModel_.testConnection(buildFormData());
        });
        connect(previousPageButton_, &QPushButton::clicked, this, [this]()
        {
            if (currentPage_ > 1)
            {
                viewModel_.loadProfiles(static_cast<std::size_t>(currentPage_ - 1));
            }
        });
        connect(nextPageButton_, &QPushButton::clicked, this, [this]()
        {
            if (currentPage_ < totalPages_)
            {
                viewModel_.loadProfiles(static_cast<std::size_t>(currentPage_ + 1));
            }
        });
        connect(profilesList_, &QListWidget::currentRowChanged, this, [this](const int row)
        {
            populateForm(viewModel_.profileAt(row));
        });

        connect(&viewModel_, &ConnectionManagementViewModel::profilesChanged, this, [this](const auto& profiles)
        {
            profilesList_->clear();
            for (const auto& profile : profiles)
            {
                profilesList_->addItem(QString("%1\n%2").arg(profile.title, profile.subtitle));
            }
        });
        connect(&viewModel_, &ConnectionManagementViewModel::paginationChanged, this,
                [this](const int currentPage, const int totalPages)
                {
                    currentPage_ = currentPage;
                    totalPages_ = totalPages;
                    paginationLabel_->setText(QString("Page %1 / %2").arg(currentPage_).arg(totalPages_));
                    previousPageButton_->setEnabled(currentPage_ > 1);
                    nextPageButton_->setEnabled(currentPage_ < totalPages_);
                });
        connect(&viewModel_, &ConnectionManagementViewModel::notificationChanged, this, [this](const QString& message)
        {
            statusLabel_->setText(message);
        });
        connect(&viewModel_, &ConnectionManagementViewModel::busyChanged, this, [this](const bool isBusy)
        {
            testButton_->setEnabled(!isBusy);
            saveButton_->setEnabled(!isBusy);
        });
        connect(
            &viewModel_,
            &ConnectionManagementViewModel::connectionTestCompleted,
            this,
            [this](const bool isSuccessful, const QString& message, const qint64 latencyMs,
                   const QString& serverVersion)
            {
                auto summary = QString("%1\nLatency: %2 ms").arg(message).arg(latencyMs);
                if (!serverVersion.isEmpty())
                {
                    summary.append(QString("\nServer: Redis %1").arg(serverVersion));
                }
                statusLabel_->setText(summary);
                statusLabel_->setProperty("success", isSuccessful);
            });

        viewModel_.loadProfiles();
    }

    ConnectionFormData ConnectionManagementWidget::buildFormData() const
    {
        return ConnectionFormData{
            selectedProfileId_,
            nameEdit_->text().trimmed(),
            hostEdit_->text().trimmed(),
            static_cast<quint16>(portSpinBox_->value()),
            usernameEdit_->text().trimmed(),
            passwordEdit_->text(),
            databaseSpinBox_->value(),
            timeoutSpinBox_->value(),
            tlsCheckBox_->isChecked(),
        };
    }

    void ConnectionManagementWidget::populateForm(const ConnectionFormData& formData)
    {
        selectedProfileId_ = formData.id;
        nameEdit_->setText(formData.name);
        hostEdit_->setText(formData.host);
        portSpinBox_->setValue(formData.port);
        usernameEdit_->setText(formData.username);
        passwordEdit_->setText(formData.password);
        databaseSpinBox_->setValue(formData.databaseIndex);
        timeoutSpinBox_->setValue(formData.connectTimeoutMs);
        tlsCheckBox_->setChecked(formData.useTls);
    }
} // namespace redisforge::presentation::connections
