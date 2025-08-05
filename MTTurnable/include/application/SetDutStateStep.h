#ifndef SETDUTSTATESTEP_H
#define SETDUTSTATESTEP_H

#include "application/IWorkflowStep.h"
#include <QJsonObject>
#include <QString>

namespace Application {

class SetDutStateStep : public IWorkflowStep {
public:
    SetDutStateStep(const QJsonObject &config, QObject *parent = nullptr);
    ~SetDutStateStep() override;

    bool execute(std::shared_ptr<WorkflowContext> context) override;
    QString getName() const override;
    QString getDescription() const override;
    StepStatus getStatus() const override;
    QJsonObject getResult() const override;
    void setConfiguration(const QJsonObject &config) override;
    QJsonObject getConfiguration() const override;
    bool canExecute(std::shared_ptr<WorkflowContext> context) const override;

private:
    QJsonObject m_config;
};

} // namespace Application

#endif // SETDUTSTATESTEP_H 