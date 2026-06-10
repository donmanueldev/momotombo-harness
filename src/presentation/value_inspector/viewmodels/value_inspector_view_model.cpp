#include "presentation/value_inspector/viewmodels/value_inspector_view_model.h"

#include <QtConcurrent/QtConcurrent>

#include "presentation/value_inspector/viewmodels/value_inspection_view_data_mapper.h"

namespace redisforge::presentation::value_inspector
{
ValueInspectorViewModel::ValueInspectorViewModel(
    app::value_inspector::InspectKeyUseCase& inspectKeyUseCase,
    app::value_inspector::SaveStringValueUseCase& saveStringValueUseCase,
    app::value_inspector::SaveHashFieldUseCase& saveHashFieldUseCase,
    QObject* parent)
    : QObject(parent),
      inspectKeyUseCase_(inspectKeyUseCase),
      saveStringValueUseCase_(saveStringValueUseCase),
      saveHashFieldUseCase_(saveHashFieldUseCase)
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

    auto* useCase = &inspectKeyUseCase_;
    const auto query = app::value_inspector::InspectKeyQuery{
        trimmedKey.toStdString(),
        {
            .collectionEntryLimit = 200,
            .stringByteLimit = 65536,
        },
    };
    watcher->setFuture(QtConcurrent::run([useCase, query]() { return useCase->execute(query); }));
}

void ValueInspectorViewModel::refreshCurrentKey()
{
    if (viewData_.key.empty())
    {
        emit statusChanged("Select a key to inspect.");
        return;
    }

    inspectKey(QString::fromStdString(viewData_.key));
}

void ValueInspectorViewModel::saveStringValue(const QString& value)
{
    if (busy_)
    {
        emit statusChanged("Wait for the current Redis operation to finish.");
        return;
    }

    if (viewData_.key.empty() || viewData_.type != redisforge::domain::value_inspector::RedisValueType::String)
    {
        emit statusChanged("Select a string key before saving.");
        return;
    }

    const auto result = saveStringValueUseCase_.execute({
        viewData_.key,
        value.toStdString(),
    });
    if (!result.isSuccessful)
    {
        emit statusChanged(QString::fromStdString(
            result.errors.empty() ? "Failed to save string value." : result.errors.front()));
        return;
    }

    emit statusChanged(QString("Saved string key '%1'.").arg(QString::fromStdString(viewData_.key)));
    inspectKey(QString::fromStdString(viewData_.key));
}

void ValueInspectorViewModel::saveHashField(const QString& field, const QString& value)
{
    if (busy_)
    {
        emit statusChanged("Wait for the current Redis operation to finish.");
        return;
    }

    if (viewData_.key.empty() || viewData_.type != redisforge::domain::value_inspector::RedisValueType::Hash)
    {
        emit statusChanged("Select a hash key before saving.");
        return;
    }

    const auto result = saveHashFieldUseCase_.execute({
        viewData_.key,
        field.trimmed().toStdString(),
        value.toStdString(),
    });
    if (!result.isSuccessful)
    {
        emit statusChanged(QString::fromStdString(
            result.errors.empty() ? "Failed to save hash field." : result.errors.front()));
        return;
    }

    emit statusChanged(
        QString("Saved field '%1' on hash '%2'.").arg(field.trimmed(), QString::fromStdString(viewData_.key)));
    inspectKey(QString::fromStdString(viewData_.key));
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
