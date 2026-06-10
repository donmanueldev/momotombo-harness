#include "presentation/keys_explorer/widgets/keys_explorer_widget.h"

#include <QHeaderView>
#include <QItemSelectionModel>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
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
        auto* rootLayout = new QVBoxLayout(this);

        connectionLabel_ = new QLabel("Active connection: none", this);

        auto* filtersLayout = new QHBoxLayout();
        patternEdit_ = new QLineEdit(this);
        patternEdit_->setPlaceholderText("Pattern, e.g. user:*");
        patternEdit_->setText("*");
        pageSizeSpinBox_ = new QSpinBox(this);
        pageSizeSpinBox_->setRange(10, 1000);
        pageSizeSpinBox_->setValue(100);
        refreshButton_ = new QPushButton("Refresh", this);
        previousPageButton_ = new QPushButton("Previous", this);
        nextPageButton_ = new QPushButton("Next", this);
        paginationLabel_ = new QLabel("Page 1", this);

        filtersLayout->addWidget(new QLabel("Pattern", this));
        filtersLayout->addWidget(patternEdit_, 1);
        filtersLayout->addWidget(new QLabel("Page size", this));
        filtersLayout->addWidget(pageSizeSpinBox_);
        filtersLayout->addWidget(refreshButton_);
        filtersLayout->addWidget(previousPageButton_);
        filtersLayout->addWidget(nextPageButton_);
        filtersLayout->addWidget(paginationLabel_);

        keysTable_ = new QTableView(this);
        keysTableModel_ = new KeysTableModel(this);
        keysTable_->setModel(keysTableModel_);
        keysTable_->horizontalHeader()->setStretchLastSection(true);
        keysTable_->verticalHeader()->setVisible(false);
        keysTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
        keysTable_->setSelectionMode(QAbstractItemView::SingleSelection);
        keysTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
        keysTable_->setAlternatingRowColors(true);

        detailsWidget_ = new redisforge::presentation::value_inspector::ValueInspectionDetailsWidget(
            valueInspectorViewModel_, this);

        statusLabel_ = new QLabel("Test a connection first.", this);
        statusLabel_->setWordWrap(true);

        rootLayout->addWidget(connectionLabel_);
        rootLayout->addLayout(filtersLayout);
        rootLayout->addWidget(keysTable_, 1);
        rootLayout->addWidget(detailsWidget_);
        rootLayout->addWidget(statusLabel_);

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

            const auto row = selectedRows.front().row();
            const auto key = keysTableModel_->keyAt(row);
            if (key.isEmpty())
            {
                return;
            }

            keysTableModel_->setTypeForKey(key, "Loading...");
            valueInspectorViewModel_.inspectKey(key);
        });

        connect(&viewModel_, &KeysExplorerViewModel::activeConnectionChanged, this, [this](const QString& summary)
        {
            connectionLabel_->setText(QString("Active connection: %1").arg(summary));
        });
        connect(&viewModel_, &KeysExplorerViewModel::keysChanged, this, [this](const QStringList& keys)
        {
            keysTableModel_->setKeys(keys);
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
                const auto selectedKey = keysTableModel_->keyAt(selectedRows.front().row());
                if (data.key != selectedKey.toStdString())
                {
                    return;
                }

                keysTableModel_->setTypeForKey(selectedKey, QString::fromStdString(data.typeLabel));
            });
    }
} // namespace redisforge::presentation::keys_explorer
