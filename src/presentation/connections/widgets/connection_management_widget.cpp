#include "presentation/connections/widgets/connection_management_widget.h"

#include <QFrame>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QSpinBox>
#include <QStyle>
#include <QVBoxLayout>

namespace redisforge::presentation::connections
{
    ConnectionManagementWidget::ConnectionManagementWidget(ConnectionManagementViewModel& viewModel, QWidget* parent)
        : QWidget(parent), viewModel_(viewModel)
    {
        setObjectName("connectionsScreen");
        auto* rootLayout = new QHBoxLayout(this);
        rootLayout->setContentsMargins(0, 0, 0, 0);
        rootLayout->setSpacing(12);

        auto* savedProfilesPanel = new QWidget(this);
        savedProfilesPanel->setObjectName("panelCard");
        auto* savedProfilesLayout = new QVBoxLayout(savedProfilesPanel);
        savedProfilesLayout->setContentsMargins(14, 14, 14, 14);
        savedProfilesLayout->setSpacing(10);
        auto* libraryTitle = new QLabel("Connections", savedProfilesPanel);
        libraryTitle->setObjectName("panelTitle");
        profilesList_ = new QListWidget(savedProfilesPanel);
        profilesList_->setAlternatingRowColors(true);
        profilesList_->setMinimumWidth(280);
        paginationLabel_ = new QLabel("Page 1 / 1", savedProfilesPanel);
        paginationLabel_->setObjectName("metaLabel");
        previousPageButton_ = new QPushButton("Previous", savedProfilesPanel);
        nextPageButton_ = new QPushButton("Next", savedProfilesPanel);
        nextPageButton_->setObjectName("ghostButton");
        previousPageButton_->setObjectName("ghostButton");

        auto* paginationButtonsLayout = new QHBoxLayout();
        paginationButtonsLayout->setSpacing(10);
        paginationButtonsLayout->addWidget(previousPageButton_);
        paginationButtonsLayout->addWidget(nextPageButton_);

        savedProfilesLayout->addWidget(libraryTitle);
        savedProfilesLayout->addWidget(profilesList_);
        savedProfilesLayout->addWidget(paginationLabel_);
        savedProfilesLayout->addLayout(paginationButtonsLayout);

        auto* formPanel = new QWidget(this);
        formPanel->setObjectName("panelCard");
        auto* formPanelLayout = new QVBoxLayout(formPanel);
        formPanelLayout->setContentsMargins(14, 14, 14, 14);
        formPanelLayout->setSpacing(12);
        auto* formTitle = new QLabel("Connection settings", formPanel);
        formTitle->setObjectName("panelTitle");
        auto* formLayout = new QFormLayout();
        formLayout->setSpacing(10);
        formLayout->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        formLayout->setFormAlignment(Qt::AlignTop);
        formLayout->setHorizontalSpacing(12);
        nameEdit_ = new QLineEdit(formPanel);
        nameEdit_->setPlaceholderText("Local Redis");
        hostEdit_ = new QLineEdit(formPanel);
        hostEdit_->setPlaceholderText("127.0.0.1");
        portSpinBox_ = new QSpinBox(formPanel);
        portSpinBox_->setRange(1, 65535);
        portSpinBox_->setValue(6379);
        usernameEdit_ = new QLineEdit(formPanel);
        usernameEdit_->setPlaceholderText("Optional username");
        passwordEdit_ = new QLineEdit(formPanel);
        passwordEdit_->setEchoMode(QLineEdit::Password);
        passwordEdit_->setPlaceholderText("Password or token");
        databaseSpinBox_ = new QSpinBox(formPanel);
        databaseSpinBox_->setRange(0, 1024);
        timeoutSpinBox_ = new QSpinBox(formPanel);
        timeoutSpinBox_->setRange(250, 30000);
        timeoutSpinBox_->setSingleStep(250);
        timeoutSpinBox_->setValue(3000);
        timeoutSpinBox_->setSuffix(" ms");

        formLayout->addRow("Profile name", nameEdit_);
        formLayout->addRow("Host", hostEdit_);
        formLayout->addRow("Port", portSpinBox_);
        formLayout->addRow("Username", usernameEdit_);
        formLayout->addRow("Password", passwordEdit_);
        formLayout->addRow("Database", databaseSpinBox_);
        formLayout->addRow("Timeout", timeoutSpinBox_);
        formPanelLayout->addWidget(formTitle);
        formPanelLayout->addLayout(formLayout);

        auto* actionLayout = new QHBoxLayout();
        actionLayout->setSpacing(8);
        auto* clearButton = new QPushButton("New Profile", this);
        clearButton->setObjectName("ghostButton");
        saveButton_ = new QPushButton("Save Profile", this);
        testButton_ = new QPushButton("Test Connection", this);
        testButton_->setObjectName("primaryButton");
        actionLayout->addWidget(clearButton);
        actionLayout->addStretch(1);
        actionLayout->addWidget(saveButton_);
        actionLayout->addWidget(testButton_);

        auto* statusBanner = new QFrame(this);
        statusBanner->setObjectName("infoBanner");
        auto* statusLayout = new QHBoxLayout(statusBanner);
        statusLayout->setContentsMargins(12, 10, 12, 10);
        statusLayout->setSpacing(8);
        statusLabel_ = new QLabel("Ready.", statusBanner);
        statusLabel_->setObjectName("statusCard");
        statusLabel_->setWordWrap(true);
        statusLayout->addWidget(statusLabel_);

        auto* rightPaneLayout = new QVBoxLayout();
        rightPaneLayout->setSpacing(10);
        auto* rightPaneTitle = new QLabel("Open or create", this);
        rightPaneTitle->setObjectName("sectionTitle");
        rightPaneLayout->addWidget(rightPaneTitle);
        rightPaneLayout->addWidget(formPanel);
        rightPaneLayout->addLayout(actionLayout);
        rightPaneLayout->addWidget(statusBanner);
        rightPaneLayout->addStretch();

        rootLayout->addWidget(savedProfilesPanel, 1);
        rootLayout->addLayout(rightPaneLayout, 2);

        connect(saveButton_, &QPushButton::clicked, this, [this]()
        {
            viewModel_.saveConnection(buildFormData());
        });
        connect(clearButton, &QPushButton::clicked, this, [this]()
        {
            profilesList_->clearSelection();
            populateForm(ConnectionFormData{});
            statusLabel_->setText("Ready.");
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
            statusLabel_->setProperty("success", QVariant{});
            style()->unpolish(statusLabel_);
            style()->polish(statusLabel_);
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
                style()->unpolish(statusLabel_);
                style()->polish(statusLabel_);
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
    }
} // namespace redisforge::presentation::connections
