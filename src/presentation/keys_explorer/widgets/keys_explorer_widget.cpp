#include "presentation/keys_explorer/widgets/keys_explorer_widget.h"

#include <QFrame>
#include <QComboBox>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QSpinBox>
#include <QSplitter>
#include <QTableView>
#include <QVBoxLayout>

#include "presentation/keys_explorer/models/keys_table_model.h"
#include "presentation/keys_explorer/viewmodels/keys_explorer_view_model.h"
#include "presentation/value_inspector/viewmodels/value_inspector_view_model.h"
#include "presentation/value_inspector/widgets/value_inspection_details_widget.h"

namespace redisforge::presentation::keys_explorer
{
    KeysExplorerWidget::KeysExplorerWidget(
        KeysExplorerViewModel& viewModel,
        value_inspector::ValueInspectorViewModel& valueInspectorViewModel,
        QWidget* parent)
        : QWidget(parent), viewModel_(viewModel), valueInspectorViewModel_(valueInspectorViewModel)
    {
        setObjectName("browserScreen");
        auto* rootLayout = new QVBoxLayout(this);
        rootLayout->setContentsMargins(0, 0, 0, 0);
        rootLayout->setSpacing(10);

        auto* titleLabel = new QLabel("Data Browser", this);
        titleLabel->setObjectName("sectionTitle");
        connectionLabel_ = new QLabel("Active connection: none", this);
        connectionLabel_->setObjectName("statusCard");
        auto* connectionBanner = new QFrame(this);
        connectionBanner->setObjectName("infoBanner");
        auto* connectionBannerLayout = new QHBoxLayout(connectionBanner);
        connectionBannerLayout->setContentsMargins(12, 10, 12, 10);
        connectionBannerLayout->addWidget(connectionLabel_);

        auto* filtersCard = new QWidget(this);
        filtersCard->setObjectName("toolbarCard");
        auto* filtersLayout = new QHBoxLayout(filtersCard);
        filtersLayout->setContentsMargins(12, 10, 12, 10);
        filtersLayout->setSpacing(8);
        patternEdit_ = new QLineEdit(this);
        patternEdit_->setPlaceholderText("Pattern");
        patternEdit_->setText("*");
        typeFilterComboBox_ = new QComboBox(this);
        typeFilterComboBox_->addItems({"All types", "string", "hash", "list", "set", "zset", "stream"});
        typeFilterComboBox_->setMinimumContentsLength(10);
        typeFilterComboBox_->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
        pageSizeSpinBox_ = new QSpinBox(this);
        pageSizeSpinBox_->setRange(10, 1000);
        pageSizeSpinBox_->setValue(100);
        pageSizeSpinBox_->setSuffix(" rows");
        refreshButton_ = new QPushButton("Refresh", this);
        refreshButton_->setObjectName("primaryButton");
        previousPageButton_ = new QPushButton("Previous", this);
        nextPageButton_ = new QPushButton("Next", this);
        previousPageButton_->setObjectName("ghostButton");
        nextPageButton_->setObjectName("ghostButton");
        paginationLabel_ = new QLabel("Page 1", this);

        filtersLayout->addWidget(new QLabel("Pattern", this));
        filtersLayout->addWidget(patternEdit_, 1);
        filtersLayout->addWidget(new QLabel("Type", this));
        filtersLayout->addWidget(typeFilterComboBox_);
        filtersLayout->addWidget(new QLabel("Page size", this));
        filtersLayout->addWidget(pageSizeSpinBox_);
        filtersLayout->addWidget(refreshButton_);
        filtersLayout->addWidget(previousPageButton_);
        filtersLayout->addWidget(nextPageButton_);
        filtersLayout->addWidget(paginationLabel_);

        keysTable_ = new QTableView(this);
        keysTableModel_ = new KeysTableModel(this);
        keysFilterModel_ = new QSortFilterProxyModel(this);
        keysFilterModel_->setSourceModel(keysTableModel_);
        keysFilterModel_->setFilterKeyColumn(1);
        keysFilterModel_->setFilterCaseSensitivity(Qt::CaseInsensitive);
        keysTable_->setModel(keysFilterModel_);
        keysTable_->horizontalHeader()->setStretchLastSection(true);
        keysTable_->verticalHeader()->setVisible(false);
        keysTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
        keysTable_->setSelectionMode(QAbstractItemView::SingleSelection);
        keysTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
        keysTable_->setAlternatingRowColors(true);
        keysTable_->setShowGrid(false);
        keysTable_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
        keysTable_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);

        detailsWidget_ = new redisforge::presentation::value_inspector::ValueInspectionDetailsWidget(
            valueInspectorViewModel_, this);

        statusLabel_ = new QLabel("Test a connection first.", this);
        statusLabel_->setObjectName("statusCard");
        statusLabel_->setWordWrap(true);

        rootLayout->addWidget(titleLabel);
        rootLayout->addWidget(connectionBanner);
        rootLayout->addWidget(filtersCard);
        contentSplitter_ = new QSplitter(Qt::Horizontal, this);
        auto* browserPane = new QWidget(contentSplitter_);
        browserPane->setObjectName("panelCard");
        auto* browserPaneLayout = new QVBoxLayout(browserPane);
        browserPaneLayout->setContentsMargins(12, 12, 12, 12);
        browserPaneLayout->setSpacing(8);
        auto* browserPaneTitle = new QLabel("Keys", browserPane);
        browserPaneTitle->setObjectName("panelTitle");
        browserPaneLayout->addWidget(browserPaneTitle);
        browserPaneLayout->addWidget(keysTable_);
        auto* detailsPane = new QWidget(contentSplitter_);
        detailsPane->setObjectName("panelCard");
        auto* detailsPaneLayout = new QVBoxLayout(detailsPane);
        detailsPaneLayout->setContentsMargins(12, 12, 12, 12);
        detailsPaneLayout->addWidget(detailsWidget_);
        contentSplitter_->addWidget(browserPane);
        contentSplitter_->addWidget(detailsPane);
        contentSplitter_->setStretchFactor(0, 2);
        contentSplitter_->setStretchFactor(1, 3);
        rootLayout->addWidget(contentSplitter_, 1);
        auto* statusBanner = new QFrame(this);
        statusBanner->setObjectName("infoBanner");
        auto* statusLayout = new QHBoxLayout(statusBanner);
        statusLayout->setContentsMargins(12, 10, 12, 10);
        statusLayout->addWidget(statusLabel_);
        rootLayout->addWidget(statusBanner);

        previousPageButton_->setEnabled(false);
        nextPageButton_->setEnabled(false);

        connect(patternEdit_, &QLineEdit::editingFinished, this, [this]()
        {
            viewModel_.updatePattern(patternEdit_->text());
            viewModel_.refresh();
        });
        connect(pageSizeSpinBox_, &QSpinBox::valueChanged, this, [this](const int value)
        {
            viewModel_.updatePageSize(value);
        });
        connect(typeFilterComboBox_, &QComboBox::currentTextChanged, this, [this](const QString& selectedType)
        {
            if (selectedType == "All types")
            {
                keysFilterModel_->setFilterFixedString(QString{});
                return;
            }

            keysFilterModel_->setFilterFixedString(selectedType);
        });
        connect(refreshButton_, &QPushButton::clicked, this, [this]()
        {
            viewModel_.updatePattern(patternEdit_->text());
            viewModel_.refresh();
        });
        connect(previousPageButton_, &QPushButton::clicked, this, [this]()
        {
            viewModel_.previousPage();
        });
        connect(nextPageButton_, &QPushButton::clicked, this, [this]()
        {
            viewModel_.nextPage();
        });
        connect(keysTable_->selectionModel(), &QItemSelectionModel::selectionChanged, this, [this]()
        {
            const auto selectedRows = keysTable_->selectionModel()->selectedRows();
            if (selectedRows.isEmpty())
            {
                return;
            }

            const auto sourceIndex = keysFilterModel_->mapToSource(selectedRows.front());
            if (!sourceIndex.isValid())
            {
                return;
            }

            const auto row = sourceIndex.row();
            const auto key = keysTableModel_->keyAt(row);
            if (key.isEmpty())
            {
                return;
            }
            valueInspectorViewModel_.inspectKey(key);
        });

        connect(&viewModel_, &KeysExplorerViewModel::activeConnectionChanged, this, [this](const QString& summary)
        {
            connectionLabel_->setText(summary.isEmpty() ? "No active connection" : summary);
        });
        connect(&viewModel_, &KeysExplorerViewModel::keysChanged, this, [this](const std::vector<redisforge::domain::keys_explorer::KeyListEntry>& entries)
        {
            keysTableModel_->setEntries(entries);
            keysTable_->clearSelection();
            valueInspectorViewModel_.clear();
        });
        connect(&viewModel_, &KeysExplorerViewModel::statusChanged, this, [this](const QString& status)
        {
            statusLabel_->setText(status);
        });
        connect(&viewModel_, &KeysExplorerViewModel::busyChanged, this, [this](const bool isBusy)
        {
            refreshButton_->setEnabled(!isBusy);
        });
        connect(&viewModel_, &KeysExplorerViewModel::paginationChanged, this,
                [this](const int currentPage, const bool hasPreviousPage, const bool hasNextPage)
                {
                    paginationLabel_->setText(QString("Page %1").arg(currentPage));
                    previousPageButton_->setEnabled(hasPreviousPage);
                    nextPageButton_->setEnabled(hasNextPage);
                });
        connect(
            &valueInspectorViewModel_,
            &redisforge::presentation::value_inspector::ValueInspectorViewModel::inspectionChanged,
            this,
            [this]()
            {
                const auto selectedRows = keysTable_->selectionModel()->selectedRows();
                if (selectedRows.isEmpty())
                {
                    return;
                }

                const auto& data = valueInspectorViewModel_.viewData();
                const auto sourceIndex = keysFilterModel_->mapToSource(selectedRows.front());
                if (!sourceIndex.isValid())
                {
                    return;
                }

                const auto selectedKey = keysTableModel_->keyAt(sourceIndex.row());
                if (data.key != selectedKey.toStdString())
                {
                    return;
                }

                keysTableModel_->setTypeForKey(selectedKey, QString::fromStdString(data.typeLabel));
            });
    }
} // namespace redisforge::presentation::keys_explorer
