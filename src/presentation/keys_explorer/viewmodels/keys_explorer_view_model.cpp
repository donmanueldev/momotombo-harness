#include "presentation/keys_explorer/viewmodels/keys_explorer_view_model.h"

#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrent>

#include <exception>

namespace redisforge::presentation::keys_explorer
{
KeysExplorerViewModel::KeysExplorerViewModel(
    redisforge::app::keys_explorer::ScanKeysUseCase& scanKeysUseCase,
    redisforge::app::value_inspector::InspectKeyUseCase& inspectKeyUseCase,
    QObject* parent)
    : QObject(parent), scanKeysUseCase_(scanKeysUseCase), inspectKeyUseCase_(inspectKeyUseCase)
{
}

void KeysExplorerViewModel::refresh()
{
    pageCursors_.assign(1, "0");
    currentPage_ = 1;
    loadCursor("0", 1);
}

void KeysExplorerViewModel::nextPage()
{
    if (busy_ || static_cast<std::size_t>(currentPage_) >= pageCursors_.size())
    {
        return;
    }

    loadCursor(pageCursors_[static_cast<std::size_t>(currentPage_)], currentPage_ + 1);
}

void KeysExplorerViewModel::previousPage()
{
    if (busy_ || currentPage_ <= 1)
    {
        return;
    }

    loadCursor(pageCursors_[static_cast<std::size_t>(currentPage_ - 2)], currentPage_ - 1);
}

void KeysExplorerViewModel::updatePattern(const QString& pattern)
{
    pattern_ = pattern.trimmed().isEmpty() ? "*" : pattern.trimmed();
}

void KeysExplorerViewModel::updatePageSize(const int pageSize)
{
    pageSize_ = pageSize;
}

void KeysExplorerViewModel::onActiveConnectionChanged(const QString& connectionSummary)
{
    emit activeConnectionChanged(connectionSummary);
    refresh();
}

void KeysExplorerViewModel::inspectKey(const QString& key, const int row)
{
    if (key.trimmed().isEmpty())
    {
        resetInspection();
        return;
    }

    emit keyDetailsChanged(key, "Loading...", "", false);

    auto* watcher =
        new QFutureWatcher<redisforge::shared::result::OperationResult<redisforge::domain::value_inspector::KeyInspectionResult>>{
            this};

    connect(
        watcher,
        &QFutureWatcher<redisforge::shared::result::OperationResult<redisforge::domain::value_inspector::KeyInspectionResult>>::finished,
        this,
        [this, watcher, key, row]() {
            const auto result = watcher->result();
            if (!result.isSuccessful)
            {
                emit statusChanged(QString::fromStdString(
                    result.errors.empty() ? "Failed to inspect key." : result.errors.front()));
                emit keyTypeResolved(row, "error");
                emit keyDetailsChanged(key, "error", "", false);
                watcher->deleteLater();
                return;
            }

            const auto& inspection = result.value;
            const auto type = QString::fromStdString(inspection.type);
            emit keyTypeResolved(row, type);

            QString value;
            if (inspection.hasStringValue)
            {
                value = QString::fromStdString(inspection.stringValue);
                emit statusChanged(
                    QString("Loaded key '%1' as %2 with string value.").arg(QString::fromStdString(inspection.key), type));
            }
            else
            {
                emit statusChanged(
                    QString("Loaded key '%1' as %2. Value rendering for this type comes next.")
                        .arg(QString::fromStdString(inspection.key), type));
            }

            emit keyDetailsChanged(QString::fromStdString(inspection.key), type, value, inspection.hasStringValue);
            watcher->deleteLater();
        });

    watcher->setFuture(QtConcurrent::run([this, key]() {
        try
        {
            return inspectKeyUseCase_.execute(key.toStdString());
        }
        catch (const std::exception& exception)
        {
            return redisforge::shared::result::OperationResult<redisforge::domain::value_inspector::KeyInspectionResult>::failure(
                {exception.what()});
        }
    }));
}

void KeysExplorerViewModel::loadCursor(const std::string& cursor, const int pageNumber)
{
    if (busy_)
    {
        return;
    }

    busy_ = true;
    emit busyChanged(true);
    emit statusChanged("Loading keys...");
    resetInspection();

    auto* watcher =
        new QFutureWatcher<redisforge::shared::result::OperationResult<redisforge::domain::keys_explorer::KeyScanPage>>{
            this};

    connect(
        watcher,
        &QFutureWatcher<redisforge::shared::result::OperationResult<redisforge::domain::keys_explorer::KeyScanPage>>::finished,
        this,
        [this, watcher, pageNumber]() {
            busy_ = false;
            emit busyChanged(false);

            const auto result = watcher->result();
            if (!result.isSuccessful)
            {
                emit statusChanged(QString::fromStdString(
                    result.errors.empty() ? "Failed to load keys." : result.errors.front()));
                emit keysChanged({});
                emit paginationChanged(currentPage_, currentPage_ > 1, false);
                watcher->deleteLater();
                return;
            }

            currentPage_ = pageNumber;
            const auto& page = result.value;
            if (page.hasMore)
            {
                const auto nextIndex = static_cast<std::size_t>(currentPage_);
                if (pageCursors_.size() <= nextIndex)
                {
                    pageCursors_.push_back(page.nextCursor);
                }
                else
                {
                    pageCursors_[nextIndex] = page.nextCursor;
                }
            }
            else if (pageCursors_.size() > static_cast<std::size_t>(currentPage_))
            {
                pageCursors_.resize(static_cast<std::size_t>(currentPage_));
            }

            auto keyList = QStringList{};
            for (const auto& key : page.keys)
            {
                keyList.push_back(QString::fromStdString(key));
            }

            emit keysChanged(keyList);
            if (keyList.isEmpty())
            {
                emit statusChanged(
                    QString("No keys found on page %1 for pattern '%2'. Verify the active connection and database.")
                        .arg(currentPage_)
                        .arg(pattern_));
            }
            else
            {
                emit statusChanged(
                    QString("Loaded %1 keys from page %2 for pattern '%3'.")
                        .arg(keyList.size())
                        .arg(currentPage_)
                        .arg(pattern_));
            }
            emit paginationChanged(currentPage_, currentPage_ > 1, page.hasMore);
            watcher->deleteLater();
        });

    const auto query = redisforge::app::keys_explorer::ScanKeysQuery{
        cursor,
        pattern_.toStdString(),
        static_cast<std::size_t>(pageSize_),
    };

    watcher->setFuture(QtConcurrent::run([this, query]() {
        try
        {
            return scanKeysUseCase_.execute(query);
        }
        catch (const std::exception& exception)
        {
            return redisforge::shared::result::OperationResult<redisforge::domain::keys_explorer::KeyScanPage>::failure(
                {exception.what()});
        }
    }));
}

void KeysExplorerViewModel::resetInspection()
{
    emit keyDetailsChanged("", "", "", false);
}
} // namespace redisforge::presentation::keys_explorer
