#pragma once

#include <cstddef>

#include <QObject>
#include <QString>

#include "app/value_inspector/use_cases/inspect_key_use_case.h"
#include "app/value_inspector/use_cases/save_hash_field_use_case.h"
#include "app/value_inspector/use_cases/save_string_value_use_case.h"
#include "presentation/value_inspector/viewmodels/value_inspection_view_data.h"

namespace redisforge::presentation::value_inspector
{
class ValueInspectorViewModel final : public QObject
{
    Q_OBJECT

public:
    explicit ValueInspectorViewModel(
        app::value_inspector::InspectKeyUseCase& inspectKeyUseCase,
        app::value_inspector::SaveStringValueUseCase& saveStringValueUseCase,
        app::value_inspector::SaveHashFieldUseCase& saveHashFieldUseCase,
        QObject* parent = nullptr);

    void inspectKey(const QString& key);
    void refreshCurrentKey();
    void saveStringValue(const QString& value);
    void saveHashField(const QString& field, const QString& value);
    void clear();
    [[nodiscard]] const ValueInspectionViewData& viewData() const;
    [[nodiscard]] QString activeConnectionSummary() const;

public slots:
    void onActiveConnectionChanged(const QString& connectionSummary);

signals:
    void busyChanged(bool isBusy);
    void statusChanged(const QString& status);
    void inspectionChanged();
    void activeConnectionChanged(const QString& connectionSummary);

private:
    app::value_inspector::InspectKeyUseCase& inspectKeyUseCase_;
    app::value_inspector::SaveStringValueUseCase& saveStringValueUseCase_;
    app::value_inspector::SaveHashFieldUseCase& saveHashFieldUseCase_;
    ValueInspectionViewData viewData_{};
    QString activeConnectionSummary_{"none"};
    bool busy_{false};
    std::size_t activeRequestId_{0};
};
} // namespace redisforge::presentation::value_inspector
