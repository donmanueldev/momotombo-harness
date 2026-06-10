#include "presentation/value_inspector/viewmodels/value_inspector_view_model.h"

#include <QtConcurrent/QtConcurrent>

#include <exception>

#include "presentation/value_inspector/viewmodels/value_inspection_view_data_mapper.h"

namespace redisforge::presentation::value_inspector
{
ValueInspectorViewModel::ValueInspectorViewModel(
    app::value_inspector::InspectKeyUseCase& inspectKeyUseCase,
    QObject* parent)
    : QObject(parent), inspectKeyUseCase_(inspectKeyUseCase)
{
}

void ValueInspectorViewModel::inspectKey(const QString& key)
{
    const auto trimmedKey = key.trimmed();
    if (trimmedKey.isEmpty())
    {
        clear();
        emit statusChanged("Enter a Redis key to inspect.");
        return;
    }

    ++activeRequestId_;
    const auto requestId = activeRequestId_;
    busy_ = true;
    emit busyChanged(true);
    emit statusChanged(QString("Inspecting key '%1'...").arg(trimmedKey));

    viewData_ = ValueInspectionViewData{};
    viewData_.key = trimmedKey.toStdString();
    viewData_.typeLabel = "Loading...";
    emit inspectionChanged();

    auto* watcher =
        new QFutureWatcher<redisforge::shared::result::OperationResult<redisforge::domain::value_inspector::KeyInspectionResult>>{
            this};

    connect(
        watcher,
        &QFutureWatcher<redisforge::shared::result::OperationResult<redisforge::domain::value_inspector::KeyInspectionResult>>::finished,
        this,
        [this, watcher, requestId]() {
            if (requestId != activeRequestId_)
            {
                watcher->deleteLater();
                return;
            }

            busy_ = false;
            emit busyChanged(false);

            const auto result = watcher->result();
            if (!result.isSuccessful)
            {
                viewData_ = ValueInspectionViewData{};
                emit inspectionChanged();
                emit statusChanged(QString::fromStdString(
                    result.errors.empty() ? "Failed to inspect key." : result.errors.front()));
                watcher->deleteLater();
                return;
            }

            viewData_ = ValueInspectionViewDataMapper::map(result.value);
            emit inspectionChanged();
            emit statusChanged(
                QString("Loaded key '%1' as %2.")
                    .arg(QString::fromStdString(viewData_.key), QString::fromStdString(viewData_.typeLabel)));
            watcher->deleteLater();
        });

    watcher->setFuture(QtConcurrent::run([this, trimmedKey]() {
        try
        {
            return inspectKeyUseCase_.execute(trimmedKey.toStdString());
        }
        catch (const std::exception& exception)
        {
            return redisforge::shared::result::OperationResult<redisforge::domain::value_inspector::KeyInspectionResult>::failure(
                {exception.what()});
        }
    }));
}

void ValueInspectorViewModel::clear()
{
    ++activeRequestId_;
    busy_ = false;
    emit busyChanged(false);
    viewData_ = ValueInspectionViewData{};
    emit inspectionChanged();
}

const ValueInspectionViewData& ValueInspectorViewModel::viewData() const
{
    return viewData_;
}

QString ValueInspectorViewModel::activeConnectionSummary() const
{
    return activeConnectionSummary_;
}

void ValueInspectorViewModel::onActiveConnectionChanged(const QString& connectionSummary)
{
    activeConnectionSummary_ = connectionSummary;
    clear();
    emit activeConnectionChanged(connectionSummary);
    emit statusChanged(QString("Ready to inspect keys on %1.").arg(connectionSummary));
}
} // namespace redisforge::presentation::value_inspector
