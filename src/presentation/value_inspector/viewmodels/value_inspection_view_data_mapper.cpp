#include "presentation/value_inspector/viewmodels/value_inspection_view_data_mapper.h"

#include <QJsonDocument>
#include <QJsonParseError>

#include <utility>

namespace
{
[[nodiscard]] std::pair<std::string, bool> prettifyJson(const std::string& value)
{
    auto parseError = QJsonParseError{};
    const auto document = QJsonDocument::fromJson(QByteArray::fromStdString(value), &parseError);
    if (parseError.error != QJsonParseError::NoError || document.isNull())
    {
        return {value, false};
    }

    return {document.toJson(QJsonDocument::Indented).toStdString(), true};
}
} // namespace

namespace redisforge::presentation::value_inspector
{
ValueInspectionViewData ValueInspectionViewDataMapper::map(
    const redisforge::domain::value_inspector::KeyInspectionResult& inspection)
{
    auto data = ValueInspectionViewData{};
    data.key = inspection.key;
    data.type = inspection.type;
    data.typeLabel = redisforge::domain::value_inspector::redisValueTypeToString(inspection.type);
    data.hashEntries = inspection.hashEntries;
    data.listEntries = inspection.listEntries;
    data.setMembers = inspection.setMembers;
    data.sortedSetEntries = inspection.sortedSetEntries;
    data.streamEntries = inspection.streamEntries;

    if (inspection.type == redisforge::domain::value_inspector::RedisValueType::String)
    {
        const auto [formattedString, isJson] = prettifyJson(inspection.stringValue);
        data.stringValue = formattedString;
        data.isStringJson = isJson;
    }
    else
    {
        data.stringValue = inspection.stringValue;
    }

    return data;
}
} // namespace redisforge::presentation::value_inspector
