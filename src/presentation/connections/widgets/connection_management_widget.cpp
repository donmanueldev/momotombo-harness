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
        rootLayout->setSpacing(20);

        auto* savedProfilesPanel = new QWidget(this);
        savedProfilesPanel->setObjectName("panelCard");
        auto* savedProfilesLayout = new QVBoxLayout(savedProfilesPanel);
        savedProfilesLayout->setContentsMargins(20, 20, 20, 20);
        savedProfilesLayout->setSpacing(14);
        auto* libraryEyebrow = new QLabel("Library", savedProfilesPanel);
        libraryEyebrow->setObjectName("panelEyebrow");
        auto* libraryTitle = new QLabel("Saved connections", savedProfilesPanel);
        libraryTitle->setObjectName("panelTitle");
        auto* libraryDescription = new QLabel("Redis endpoints.", savedProfilesPanel);
        libraryDescription->setObjectName("panelDescription");
        libraryDescription->setWordWrap(true);
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

        savedProfilesLayout->addWidget(libraryEyebrow);
        savedProfilesLayout->addWidget(libraryTitle);
        savedProfilesLayout->addWidget(libraryDescription);
        savedProfilesLayout->addWidget(profilesList_);
        savedProfilesLayout->addWidget(paginationLabel_);
        savedProfilesLayout->addLayout(paginationButtonsLayout);

        auto* formPanel = new QWidget(this);
        formPanel->setObjectName("panelCard");
        auto* formPanelLayout = new QVBoxLayout(formPanel);
        formPanelLayout->setContentsMargins(20, 20, 20, 20);
        formPanelLayout->setSpacing(16);
        auto* formEyebrow = new QLabel("Details", formPanel);
        formEyebrow->setObjectName("panelEyebrow");
        auto* formTitle = new QLabel("Connection settings", formPanel);
        formTitle->setObjectName("panelTitle");
        auto* formDescription = new QLabel("Host, port, and credentials.", formPanel);
        formDescription->setObjectName("panelDescription");
        formDescription->setWordWrap(true);
        auto* formLayout = new QFormLayout();
        formLayout->setSpacing(12);
        formLayout->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        formLayout->setFormAlignment(Qt::AlignTop);
        formLayout->setHorizontalSpacing(18);
        nameEdit_ = new QLineEdit(formPanel);
        nameEdit_->setPlaceholderText("Production Redis, Staging cache, Local dev...");
        hostEdit_ = new QLineEdit(formPanel);
        hostEdit_->setPlaceholderText("127.0.0.1 or switchyard.proxy.rlwy.net");
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
        formLayout->addRow("Timeout (ms)", timeoutSpinBox_);
        formPanelLayout->addWidget(formEyebrow);
        formPanelLayout->addWidget(formTitle);
        formPanelLayout->addWidget(formDescription);
        formPanelLayout->addLayout(formLayout);

        auto* actionLayout = new QHBoxLayout();
        actionLayout->setSpacing(12);
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
        statusLayout->setContentsMargins(16, 14, 16, 14);
        statusLayout->setSpacing(12);
        statusLabel_ = new QLabel("Ready to configure a new connection.", statusBanner);
        statusLabel_->setObjectName("statusCard");
        statusLabel_->setWordWrap(true);
        statusLayout->addWidget(statusLabel_);

        auto* rightPaneLayout = new QVBoxLayout();
        rightPaneLayout->setSpacing(14);
        auto* rightPaneEyebrow = new QLabel("Connect", this);
        rightPaneEyebrow->setObjectName("panelEyebrow");
        auto* rightPaneTitle = new QLabel("Open or create", this);
        rightPaneTitle->setObjectName("sectionTitle");
        auto* rightPaneDescription = new QLabel("Choose a saved profile or enter details.", this);
        rightPaneDescription->setObjectName("panelDescription");
        rightPaneDescription->setWordWrap(true);
        rightPaneLayout->addWidget(rightPaneEyebrow);
        rightPaneLayout->addWidget(rightPaneTitle);
        rightPaneLayout->addWidget(rightPaneDescription);
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
            statusLabel_->setText("Ready to configure a new connection.");
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
