#include "acroViewTester.h"

bool acroViewTester::loadProjectConfig()
{
    if (m_projectConfigPath.isEmpty()) {
        return false;
    }

    QFile configFile(m_projectConfigPath);
    if (!configFile.open(QIODevice::ReadOnly)) {
        LOG_ERROR("无法打开项目配置文件: %s, 错误: %s",
            qPrintable(m_projectConfigPath),
            qPrintable(configFile.errorString()));
        return false;
    }

    QByteArray jsonData = configFile.readAll();
    configFile.close();

    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        LOG_ERROR("解析项目配置文件失败: %s", qPrintable(parseError.errorString()));
        return false;
    }

    if (!jsonDoc.isObject()) {
        LOG_ERROR("项目配置文件格式错误: 根元素不是JSON对象");
        return false;
    }

    m_projectConfig = jsonDoc.object();
    LOG_INFO("成功加载项目配置: %s", qPrintable(m_projectName));

    // 应用配置到应用程序的各个部分
    applyProjectConfig();

    return true;
}


void acroViewTester::applyProjectConfig()
{

    if (m_projectConfig.contains("server")) {
        QJsonObject serverConfig = m_projectConfig["server"].toObject();

        QString serverHost = serverConfig["host"].toString("localhost");
        int serverPort = serverConfig["port"].toInt(8080);

        LOG_INFO("配置服务器连接：%s:%d", qPrintable(serverHost), serverPort);
    }

    if (m_projectConfig.contains("products")) {
        QJsonArray productsArray = m_projectConfig["products"].toArray();


        for (int i = 0; i < productsArray.size(); ++i) {
            QJsonObject product = productsArray[i].toObject();
            QString productName = product["name"].toString();
            QString productId = product["id"].toString();

            LOG_INFO("添加产品：%s (ID: %s)", qPrintable(productName), qPrintable(productId));
        }
    }


    LOG_INFO("项目配置已应用");
}