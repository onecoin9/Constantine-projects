#include "services/SiteWorkflowRouter.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QRegularExpression>
#include "core/Logger.h"

namespace Services {

SiteWorkflowRouter& SiteWorkflowRouter::instance()
{
    static SiteWorkflowRouter inst;
    return inst;
}

bool SiteWorkflowRouter::loadFromFile(const QString &jsonPath)
{
    QFile f(jsonPath);
    if(!f.open(QIODevice::ReadOnly)){
        LOG_MODULE_ERROR("SiteWorkflowRouter", QString("Cannot open router file: %1").arg(jsonPath).toStdString());
        return false;
    }
    QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
    f.close();

    if(!doc.isArray()){
        LOG_MODULE_ERROR("SiteWorkflowRouter", "Router file should contain JSON array");
        return false;
    }
    m_routes.clear();
    for(const auto &val : doc.array()){
        QJsonObject obj = val.toObject();
        SiteRoute r{obj["siteSnRegex"].toString(), obj["workflow"].toString()};
        if(!r.regex.isEmpty() && !r.workflow.isEmpty()) m_routes.append(r);
    }
    LOG_MODULE_INFO("SiteWorkflowRouter", QString("Loaded %1 routes").arg(m_routes.size()).toStdString());
    return !m_routes.isEmpty();
}

std::optional<SiteRoute> SiteWorkflowRouter::match(const QString &siteSn) const
{
    for(const auto &r : m_routes){
        QRegularExpression re(r.regex);
        if(re.match(siteSn).hasMatch()) return r;
    }
    return std::nullopt;
}

} // namespace Services 