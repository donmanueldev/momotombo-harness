#include "presentation/keys_explorer/viewmodels/keys_explorer_view_model.h"

#include <QtConcurrent/QtConcurrent>

namespace redisforge::presentation::keys_explorer
{
    KeysExplorerViewModel::KeysExplorerViewModel(
        app::keys_explorer::ScanKeysUseCase& scanKeysUseCase,
        QObject* parent)
        : QObject(parent), scanKeysUseCase_(scanKeysUseCase)
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

    void KeysExplorerViewModel::loadCursor(const std::string& cursor, const int pageNumber)
    {
        if (busy_)
        {
            return;
        }

        busy_ = true;
        emit busyChanged(true);
        emit statusChanged("Loading keys...");

        auto* watcher =
            new QFutureWatcher<shared::result::OperationResult<
                domain::keys_explorer::KeyScanPage>>{
                this
            };

        connect(
            watcher,
            &QFutureWatcher<shared::result::OperationResult<domain::keys_explorer::KeyScanPage>>::finished,
            this,
            [this, watcher, pageNumber]()
            {
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

                emit keysChanged(page.entries);
                if (page.entries.empty())
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
                        .arg(static_cast<qulonglong>(page.entries.size()))
                        .arg(currentPage_)
                        .arg(pattern_));
                }
                emit paginationChanged(currentPage_, currentPage_ > 1, page.hasMore);
                watcher->deleteLater();
            });

        const auto query = app::keys_explorer::ScanKeysQuery{
            cursor,
            pattern_.toStdString(),
            static_cast<std::size_t>(pageSize_),
        };

        auto* useCase = &scanKeysUseCase_;
        watcher->setFuture(QtConcurrent::run([useCase, query]() { return useCase->execute(query); }));
    }
} // namespace redisforge::presentation::keys_explorer
