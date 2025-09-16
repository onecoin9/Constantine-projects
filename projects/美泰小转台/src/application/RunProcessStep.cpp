#include "application/RunProcessStep.h"
#include "application/WorkflowContext.h"
#include "core/Logger.h"
#include "GlobalItem.h"

#include <QDir>
#include <QProcess>
#include <QRegularExpression>

namespace Application {

RunProcessStep::RunProcessStep(const QJsonObject& config, QObject* parent)
    : IWorkflowStep(parent)
{
    setConfiguration(config);
}

RunProcessStep::~RunProcessStep() = default;

QString RunProcessStep::getName() const {
    return m_config.value("name").toString("RunProcessStep");
}

QString RunProcessStep::getDescription() const {
    return m_config.value("description").toString("运行外部可执行程序并判定结果。");
}

void RunProcessStep::setConfiguration(const QJsonObject& config) {
    m_config = config;
}

QJsonObject RunProcessStep::getConfiguration() const {
    return m_config;
}

bool RunProcessStep::canExecute(std::shared_ptr<WorkflowContext>) const {
    return true;
}

bool RunProcessStep::matchesAnyPattern(const QString& text, const QStringList& patterns, Qt::CaseSensitivity cs) const {
    for (const auto& p : patterns) {
        if (text.contains(p, cs)) return true;
    }
    return false;
}

bool RunProcessStep::execute(std::shared_ptr<WorkflowContext> context) {
    m_status = StepStatus::Running;
    emit statusChanged(m_status);

    const QJsonObject cfg = m_config.value("config").toObject();
    const QString executable = cfg.value("executable").toString();
    const QString workingDirectory = cfg.value("workingDirectory").toString();
    const int timeoutMs = cfg.value("timeoutMs").toInt(600000); // 默认10分钟
    const bool exitCodeAsSuccess = cfg.value("exitCodeAsSuccess").toBool(false);
    const int expectedExitCode = cfg.value("expectedExitCode").toInt(0);
    const Qt::CaseSensitivity cs = cfg.value("caseSensitive").toBool(false) ? Qt::CaseSensitive : Qt::CaseInsensitive;

    // 处理参数：支持从全局变量与上下文取值
    QStringList args;
    if (cfg.value("args").isArray()) {
        const auto arr = cfg.value("args").toArray();
        for (const auto& v : arr) {
            QString token = v.toString();
            if (token.contains("${global.archiveBaseDir}")) {
                token.replace("${global.archiveBaseDir}", GlobalItem::getInstance().getString("archiveBaseDir"));
            }
            if (token.contains("${context.archiveBaseDir}")) {
                token.replace("${context.archiveBaseDir}", context->getData("archiveBaseDir").toString());
            }
            args << token;
        }
    }

    if (executable.isEmpty()) {
        m_status = StepStatus::Failed;
        emit statusChanged(m_status);
        emit errorOccurred("RunProcessStep: 'executable' 未配置。");
        return false;
    }

    QProcess proc;
    if (!workingDirectory.isEmpty()) {
        proc.setWorkingDirectory(workingDirectory);
    }
    proc.setProgram(executable);
    proc.setArguments(args);

    LOG_MODULE_INFO("RunProcessStep", QString("启动进程: %1 %2").arg(executable).arg(args.join(' ')).toStdString());

    proc.start();
    if (!proc.waitForStarted(5000)) {
        m_status = StepStatus::Failed;
        emit statusChanged(m_status);
        emit errorOccurred(QString("RunProcessStep: 启动失败: %1").arg(proc.errorString()));
        return false;
    }

    if (!proc.waitForFinished(timeoutMs)) {
        proc.kill();
        proc.waitForFinished(3000);
        m_status = StepStatus::Failed;
        emit statusChanged(m_status);
        emit errorOccurred("RunProcessStep: 超时未完成，已终止进程。");
        return false;
    }

    const int exitCode = proc.exitCode();
    const QString stdOut = QString::fromLocal8Bit(proc.readAllStandardOutput());
    const QString stdErr = QString::fromLocal8Bit(proc.readAllStandardError());

    LOG_MODULE_INFO("RunProcessStep", QString("退出码: %1").arg(exitCode).toStdString());
    LOG_MODULE_INFO("RunProcessStep", QString("stdout: %1").arg(stdOut).toStdString());
    if (!stdErr.isEmpty()) {
        LOG_MODULE_WARNING("RunProcessStep", QString("stderr: %1").arg(stdErr).toStdString());
    }

    // 判定规则：优先按输出关键字判定，否则按退出码判定
    const QStringList successPatterns = cfg.value("successPatterns").toVariant().toStringList();
    const QStringList failurePatterns = cfg.value("failurePatterns").toVariant().toStringList();

    bool ok = false;
    if (!successPatterns.isEmpty() || !failurePatterns.isEmpty()) {
        if (matchesAnyPattern(stdOut, successPatterns, cs) && !matchesAnyPattern(stdOut, failurePatterns, cs)) {
            ok = true;
        } else if (matchesAnyPattern(stdErr, failurePatterns, cs)) {
            ok = false;
        } else if (exitCodeAsSuccess) {
            ok = (exitCode == expectedExitCode);
        }
    } else {
        // 未配置关键字时，按退出码或默认0为成功
        ok = exitCodeAsSuccess ? (exitCode == expectedExitCode) : (exitCode == 0);
    }

    m_result["exitCode"] = exitCode;
    m_result["stdout"] = stdOut;
    m_result["stderr"] = stdErr;
    m_result["success"] = ok;

    if (ok) {
        m_status = StepStatus::Completed;
        emit statusChanged(m_status);
        return true;
    }

    m_status = StepStatus::Failed;
    emit statusChanged(m_status);
    emit errorOccurred("RunProcessStep: 进程执行判定为失败。");
    return false;
}

bool RunProcessStep::rollback(std::shared_ptr<WorkflowContext>) { return true; }
QJsonObject RunProcessStep::getResult() const { return m_result; }
IWorkflowStep::StepStatus RunProcessStep::getStatus() const { return m_status; }

} // namespace Application


