#include "application/RunProcessStep.h"
#include "application/WorkflowContext.h"
#include "core/Logger.h"
#include "GlobalItem.h"
#include "services/DutManager.h"
#include "QXlsx/xlsxdocument.h"

#include <QDir>
#include <QProcess>
#include <QRegularExpression>
#include <QJsonArray>
#include <QJsonValue>

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
    const int timeoutMs = cfg.value("timeoutMs").toInt(600000);
    const bool exitCodeAsSuccess = cfg.value("exitCodeAsSuccess").toBool(false);
    const int expectedExitCode = cfg.value("expectedExitCode").toInt(0);
    const Qt::CaseSensitivity cs = cfg.value("caseSensitive").toBool(false) ? Qt::CaseSensitive : Qt::CaseInsensitive;

    QStringList args;
    if (cfg.value("args").isArray()) {
        const QJsonArray arr = cfg.value("args").toArray();
        for (const QJsonValue& v : arr) {
            QString token = v.toString();
            if (token.contains("${global.archiveBaseDir}")) {
                token.replace("${global.archiveBaseDir}", GlobalItem::getInstance().getString("archiveBaseDir"));
            }
            if (token.contains("${context.archiveBaseDir}")) {
                token.replace("${context.archiveBaseDir}", context->getData("archiveBaseDir").toString());
            }
            // 统一为平台原生分隔符，避免被被调用程序将正斜杠解析异常
            args << QDir::toNativeSeparators(token);
        }
    }

    if (executable.isEmpty()) {
        m_status = StepStatus::Failed;
        emit statusChanged(m_status);
        emit errorOccurred("RunProcessStep: 'executable' 未配置。");
        return false;
    }

    // 启动前的存在性检查，便于快速定位问题
    const QString programPath = QDir::toNativeSeparators(executable);
    if (!QFileInfo::exists(programPath)) {
        m_status = StepStatus::Failed;
        emit statusChanged(m_status);
        emit errorOccurred(QString("RunProcessStep: 可执行文件不存在: %1").arg(programPath));
        return false;
    }
    if (!args.isEmpty()) {
        const QString firstArg = args.first();
        if (!firstArg.isEmpty()) {
            QFileInfo fi(firstArg);
            if (!fi.exists()) {
                m_status = StepStatus::Failed;
                emit statusChanged(m_status);
                emit errorOccurred(QString("RunProcessStep: 参数路径不存在: %1").arg(firstArg));
                return false;
            }
        }
    }

    QProcess proc;
    if (!workingDirectory.isEmpty()) {
        proc.setWorkingDirectory(workingDirectory);
    }
    proc.setProgram(programPath);
    proc.setArguments(args);

    LOG_MODULE_INFO("RunProcessStep", QString("启动进程: %1 %2").arg(programPath).arg(args.join(' ')).toStdString());

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
    
    // 尝试多种编码方式解决乱码问题
    QByteArray stdOutBytes = proc.readAllStandardOutput();
    QByteArray stdErrBytes = proc.readAllStandardError();
    
    QString stdOut;
    QString stdErr;
    
    // 优先尝试UTF-8编码
    stdOut = QString::fromUtf8(stdOutBytes);
    stdErr = QString::fromUtf8(stdErrBytes);
    
    // 如果UTF-8解码失败或包含替换字符，尝试系统本地编码
    if (stdOut.contains(QChar::ReplacementCharacter) || stdOutBytes.isEmpty() != stdOut.isEmpty()) {
        stdOut = QString::fromLocal8Bit(stdOutBytes);
    }
    if (stdErr.contains(QChar::ReplacementCharacter) || stdErrBytes.isEmpty() != stdErr.isEmpty()) {
        stdErr = QString::fromLocal8Bit(stdErrBytes);
    }
    
    // 如果仍有问题，尝试Latin-1编码（通常不会失败）
    if (stdOut.contains(QChar::ReplacementCharacter)) {
        stdOut = QString::fromLatin1(stdOutBytes);
    }
    if (stdErr.contains(QChar::ReplacementCharacter)) {
        stdErr = QString::fromLatin1(stdErrBytes);
    }

    // 记录基本执行信息
    LOG_MODULE_INFO("RunProcessStep", QString("退出码: %1").arg(exitCode).toStdString());
    
    // 处理并记录标准输出（避免过长的日志）
    if (!stdOut.isEmpty()) {
        QString cleanedStdOut = stdOut.trimmed();
        if (cleanedStdOut.length() > 500) {
            cleanedStdOut = cleanedStdOut.left(500) + "... (输出已截断)";
        }
        LOG_MODULE_INFO("RunProcessStep", QString("stdout: %1").arg(cleanedStdOut).toStdString());
    } else {
        LOG_MODULE_INFO("RunProcessStep", "stdout: (无输出)");
    }
    
    // 记录标准错误
    if (!stdErr.isEmpty()) {
        QString cleanedStdErr = stdErr.trimmed();
        if (cleanedStdErr.length() > 500) {
            cleanedStdErr = cleanedStdErr.left(500) + "... (错误输出已截断)";
        }
        LOG_MODULE_WARNING("RunProcessStep", QString("stderr: %1").arg(cleanedStdErr).toStdString());
    }

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
        ok = exitCodeAsSuccess ? (exitCode == expectedExitCode) : (exitCode == 0);
    }

    m_result["exitCode"] = exitCode;
    m_result["stdout"] = stdOut;
    m_result["stderr"] = stdErr;
    m_result["success"] = ok;

    if (ok) {
        m_status = StepStatus::Completed;
        emit statusChanged(m_status);
        
        // 记录成功日志
        LOG_MODULE_INFO("RunProcessStep", QString("✓ 进程执行成功 - 程序: %1, 退出码: %2")
                       .arg(QFileInfo(programPath).baseName())
                       .arg(exitCode).toStdString());

        // 解析生成结果，处理分选
        
        QString resXlsxFilePath = args[0] + "\\Result\\TestResult.xlsx";
        QXlsx::Document xlsx(resXlsxFilePath);
        // 文件不存在
        if (!QFile(resXlsxFilePath).exists()) {
            LOG_MODULE_ERROR("RunProcessStep", QString("✗ Result文件夹不存在: %1").arg(resXlsxFilePath).toStdString());
            return false;
        }
        
        xlsx.selectSheet("Test-B01");
        int rowCount = xlsx.dimension().rowCount();
        QByteArray resultData = Services::DutManager::instance()->getSiteInfoByIndex(TURNABLE_SITE_INDEX).currentChipStatus;
        for (int row = 2; row <= rowCount && row <= 9; ++row) {
            int col = 3;
            QString resultStr = xlsx.read(row, col).toString();
            int result = resultStr.toInt();

            /*
                自动机顺序：
                    1 2 3 4
                    5 6 7 8
                转台顺序：
                    1 3 5 7
                    2 4 6 8
                这里需要转换
            */
            // 转台的2 4 6 8
            if (row % 2 && resultData[(row - 2 - 1) / 2 + 4] == char(0x01)) {
                if (result != 0) { // 测试结果为失败
                    resultData[(row - 2 - 1) / 2 + 4] = 2;
                }
                LOG_MODULE_INFO("RunProcessStep", QString("转台分选 socket:%1, result:%2")
                    .arg(row - 1)
                    .arg(result).toStdString());
            }
            // 转台的1 3 5 7
            if ((row % 2) == 0 && resultData[(row - 2) / 2] == char(0x01)) {
                if (result != 0) { // 测试结果为失败
                    resultData[(row - 2) / 2] = 2;
                }
                LOG_MODULE_INFO("RunProcessStep", QString("转台分选 socket:%1, result:%2")
                    .arg(row - 1)
                    .arg(result).toStdString());
            }

        }
        Services::DutManager::instance()->updateSiteChipStatusByIndex(TURNABLE_SITE_INDEX, resultData);
        LOG_MODULE_INFO("RunProcessStep", QString("转台分选结果:%1")
            .arg(resultData.toHex().constData()).toStdString());


        return true;
    }

    m_status = StepStatus::Failed;
    emit statusChanged(m_status);
    
    // 记录失败日志
    LOG_MODULE_ERROR("RunProcessStep", QString("✗ 进程执行失败 - 程序: %1, 退出码: %2")
                    .arg(QFileInfo(programPath).baseName())
                    .arg(exitCode).toStdString());
    
    emit errorOccurred(QString("RunProcessStep: 进程执行判定为失败 (退出码: %1)").arg(exitCode));
    return false;
}

bool RunProcessStep::rollback(std::shared_ptr<WorkflowContext>) { return true; }
QJsonObject RunProcessStep::getResult() const { return m_result; }
IWorkflowStep::StepStatus RunProcessStep::getStatus() const { return m_status; }

} // namespace Application


