#pragma once
#include <QString>
#include <QStringList>
#include <QJsonObject>
#include <optional>

namespace Services {

struct SiteRoute {
    QString regex;
    QString workflow;
};

class SiteWorkflowRouter {
public:
    static SiteWorkflowRouter& instance();

    bool loadFromFile(const QString& jsonPath);
    std::optional<SiteRoute> match(const QString& siteSn) const;

private:
    SiteWorkflowRouter() = default;
    QList<SiteRoute> m_routes;
};

} // namespace Services 