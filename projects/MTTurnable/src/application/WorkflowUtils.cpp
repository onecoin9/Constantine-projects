#include "application/WorkflowUtils.h"
#include "application/WorkflowContext.h"
#include <QRegularExpression>
#include <QVariant>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonDocument>
#include <QMetaType>

namespace Application {
namespace WorkflowUtils {

// Forward declaration for recursion
QJsonValue replacePlaceholders(const QJsonValue& jsonVal, const std::shared_ptr<WorkflowContext>& context);
QVariant resolvePlaceholder(const QString& placeholder, const std::shared_ptr<WorkflowContext>& context);

QString replacePlaceholders(const QString& source, const std::shared_ptr<WorkflowContext>& context)
{
    if (!context) return source;
    
    QString result = source;
    QRegularExpression re("\\$\\{([^}]+)\\}");
    auto it = re.globalMatch(source);

    while (it.hasNext()) {
        auto match = it.next();
        QString placeholder = match.captured(1);
        QVariant value = resolvePlaceholder(placeholder, context);
        // Special case for boolean to ensure it's "true" or "false"
        if(value.type() == QVariant::Bool) {
             result.replace(match.captured(0), value.toBool() ? "true" : "false");
        } else {
             result.replace(match.captured(0), value.toString());
        }
    }
    return result;
}

QJsonObject replacePlaceholders(const QJsonObject& jsonObj, const std::shared_ptr<WorkflowContext>& context)
{
    QJsonObject resultObj;
    for (auto it = jsonObj.constBegin(); it != jsonObj.constEnd(); ++it) {
        resultObj.insert(it.key(), replacePlaceholders(it.value(), context));
    }
    return resultObj;
}

QJsonArray replacePlaceholders(const QJsonArray& jsonArr, const std::shared_ptr<WorkflowContext>& context)
{
    QJsonArray resultArray;
    for(const auto& val : jsonArr){
        resultArray.append(replacePlaceholders(val, context));
    }
    return resultArray;
}

QJsonValue replacePlaceholders(const QJsonValue& jsonVal, const std::shared_ptr<WorkflowContext>& context)
{
    if(jsonVal.isString()){
        return QJsonValue(replacePlaceholders(jsonVal.toString(), context));
    }
    if(jsonVal.isObject()){
        return QJsonValue(replacePlaceholders(jsonVal.toObject(), context));
    }
    if(jsonVal.isArray()){
        return QJsonValue(replacePlaceholders(jsonVal.toArray(), context));
    }
    return jsonVal;
}

QVariant resolvePlaceholder(const QString& placeholder, const std::shared_ptr<WorkflowContext>& context)
{
    if (!context) return QVariant();

    QString key = placeholder;
    if (key.startsWith("context.")) {
        key = key.mid(8);
    }

    if (key.contains('.')) {
        QStringList parts = key.split('.');
        QString baseKey = parts.first();
        QVariant baseValue = context->getData(baseKey);
        
        QJsonObject jsonObj;
        // A QJsonObject can be stored in a QVariant in multiple ways.
        // We must be robust and check for the most common ones.
        if (baseValue.canConvert<QJsonObject>()) {
            jsonObj = baseValue.value<QJsonObject>();
        } else if (baseValue.type() == QVariant::Map) {
            jsonObj = QJsonObject::fromVariantMap(baseValue.toMap());
        } else if (baseValue.type() == QVariant::String) {
            QJsonDocument doc = QJsonDocument::fromJson(baseValue.toString().toUtf8());
            if (doc.isObject()) {
                jsonObj = doc.object();
            }
        } else if (baseValue.type() == QVariant::ByteArray) {
            QJsonDocument doc = QJsonDocument::fromJson(baseValue.toByteArray());
            if (doc.isObject()) {
                jsonObj = doc.object();
            }
        }

        if (jsonObj.isEmpty()) {
            return QVariant(); // Not a valid object to traverse
        }

        QJsonValue currentVal = jsonObj;
        for (int i = 1; i < parts.size(); ++i) {
            if (!currentVal.isObject()) return QVariant(); // Path is invalid
            currentVal = currentVal.toObject().value(parts[i]);
        }
        return currentVal.toVariant();
    }
    
    // Fallback to simple key lookup if no dot notation.
    return context->getData(key);
}

} // namespace WorkflowUtils
} // namespace Application 