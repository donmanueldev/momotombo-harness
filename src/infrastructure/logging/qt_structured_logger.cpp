#include "infrastructure/logging/qt_structured_logger.h"

#include <QDateTime>
#include <QJsonObject>
#include <QLoggingCategory>
#include <QString>

namespace
{
    QJsonObject buildPayload(
        const std::string_view level,
        const std::string_view message,
        const redisforge::shared::logging::LogContext& context)
    {
        auto payload = QJsonObject{};
        payload["timestamp"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs);
        payload["level"] = QString::fromUtf8(level.data(), static_cast<qsizetype>(level.size()));
        payload["message"] = QString::fromUtf8(message.data(), static_cast<qsizetype>(message.size()));

        for (const auto& [key, value] : context)
        {
            payload[QString::fromStdString(key)] = QString::fromStdString(value);
        }

        return payload;
    }
} // namespace

namespace redisforge::infrastructure::logging
{
    void QtStructuredLogger::info(std::string_view message, shared::logging::LogContext context)
    {
        qInfo().noquote() << QJsonDocument{buildPayload("INFO", message, context)}.toJson(QJsonDocument::Compact);
    }

    void QtStructuredLogger::warning(std::string_view message, shared::logging::LogContext context)
    {
        qWarning().noquote() << QJsonDocument{buildPayload("WARN", message, context)}.toJson(QJsonDocument::Compact);
    }

    void QtStructuredLogger::error(std::string_view message, shared::logging::LogContext context)
    {
        qCritical().noquote() << QJsonDocument{buildPayload("ERROR", message, context)}.toJson(QJsonDocument::Compact);
    }
} // namespace redisforge::infrastructure::logging
