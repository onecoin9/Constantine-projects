#include "application/WorkflowUtils.h"
#include "application/WorkflowContext.h"
#include "services/DeviceManager.h"
#include "domain/HandlerDevice.h"
#include <QRegularExpression>
#include <QVariant>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonDocument>
#include <QMetaType>
#include <memory>

namespace Application {
namespace WorkflowUtils {

// Forward declaration for recursion
QJsonValue replacePlaceholders(const QJsonValue& jsonVal, const std::shared_ptr<WorkflowContext>& context);
QVariant resolvePlaceholder(const QString& placeholder, const std::shared_ptr<WorkflowContext>& context);

QVariant getContextConfig(const QString& key, const std::shared_ptr<WorkflowContext>& context);
QVariant getDeviceConfig(const QString& key, const QString& handler, const std::shared_ptr<WorkflowContext>& context);

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

        // 判断整个字符串是不是一个单一占位符
        QRegularExpression singlePlaceholderRe("^\\$\\{([^}]+)\\}$");
        auto m = singlePlaceholderRe.match(jsonVal.toString());
        if (m.hasMatch()) {
            QVariant v = resolvePlaceholder(m.captured(1), context);
            return QJsonValue::fromVariant(v);
        }
        
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
        return getContextConfig(key, context);
    }

    if (key.startsWith("device.")) {
        key = key.mid(7);

        if (key.contains('.')) {
            QStringList parts = key.split('.');
            QString handler = parts.first();
            QString baseKey = parts.last();
            return getDeviceConfig(baseKey, handler, context);
        }
        else {
            return QVariant();
        }
    }

    return QVariant();
}

QVariant getContextConfig(const QString& key, const std::shared_ptr<WorkflowContext>& context)
{
    if (!key.contains('.')) {
        return context->getData(key);
    }

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

QVariant getDeviceConfig(const QString& key, const QString& handler, const std::shared_ptr<WorkflowContext>& context)
{
    auto deviceManager = context->getDeviceManager();
    auto device = deviceManager->getHandlerDevice();
    if (!device) {
        return QVariant();
    }

    auto handlerDevice = std::dynamic_pointer_cast<Domain::HandlerDevice>(device);
    auto configParam = handlerDevice->getConfiguration();
    if (configParam.contains(key)) {
        return configParam[key].toVariant();
    }

    return QVariant();
}

} // namespace WorkflowUtils
} // namespace Application 