#ifndef WORKFLOWUTILS_H
#define WORKFLOWUTILS_H

#include <QString>
#include <QJsonObject>
#include <QVariant>
#include <memory>

namespace Application {

class WorkflowContext; // Forward declaration

namespace WorkflowUtils {

    /**
     * @brief 递归地替换QJsonObject中的占位符
     * @param jsonObj 要处理的JSON对象
     * @param context 工作流上下文，用于查找占位符的值
     */
    QJsonObject replacePlaceholders(const QJsonObject& jsonObj, const std::shared_ptr<WorkflowContext>& context);
    
    /**
     * @brief 替换字符串中的占位符
     * @param source 源字符串
     * @param context 工作流上下文
     * @return 替换后的字符串
     */
    QString replacePlaceholders(const QString& source, const std::shared_ptr<WorkflowContext>& context);

    /**
     * @brief 解析一个可能带有嵌套路径的占位符
     * @param placeholder 占位符字符串, e.g., "${context.someKey.nestedKey}"
     * @param context 工作流上下文
     * @return 解析出的值
     */
    QVariant resolvePlaceholder(const QString& placeholder, const std::shared_ptr<WorkflowContext>& context);

} // namespace WorkflowUtils
} // namespace Application

#endif // WORKFLOWUTILS_H 