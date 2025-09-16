#include "domain/ProcessDevice.h"
#include <QProcess>
#include <QJsonArray>
#include "core/Logger.h"

namespace Domain {

ProcessDevice::ProcessDevice(QObject *parent) : IDevice(parent)
{
    m_status = DeviceStatus::Ready; // ProcessDevice is always ready.
}

ProcessDevice::~ProcessDevice() = default;

bool ProcessDevice::initialize()
{
    m_status = DeviceStatus::Ready;
    emit statusChanged(m_status);
    LOG_MODULE_INFO("ProcessDevice", "ProcessDevice is stateless and always initialized.");
    return true;
}

bool ProcessDevice::release()
{
    m_status = DeviceStatus::Disconnected;
    emit statusChanged(m_status);
    LOG_MODULE_INFO("ProcessDevice", "ProcessDevice released.");
    return true;
}

QJsonObject ProcessDevice::executeCommand(const QString &command, const QJsonObject &params)
{
    if (command.toLower() != "run") {
        return {
            {"success", false},
            {"error", QString("ProcessDevice only supports 'run' command, but got '%1'").arg(command)}
        };
    }

    QString executable = params["executable"].toString();
    if (executable.isEmpty()) {
        return {{"success", false}, {"error", "Parameter 'executable' is missing."}};
    }

    QStringList arguments;
    if (params.contains("args") && params["args"].isArray()) {
        for (const auto& arg : params["args"].toArray()) {
            arguments << arg.toString();
        }
    }

    LOG_MODULE_INFO("ProcessDevice", QString("Executing: %1 %2").arg(executable, arguments.join(" ")).toStdString());

    QProcess process;
    process.start(executable, arguments);

    // Block and wait for the process to finish.
    // Timeout set to 30 seconds as a safeguard.
    if (!process.waitForFinished(30000)) {
        LOG_MODULE_ERROR("ProcessDevice", QString("Process timed out: %1").arg(executable).toStdString());
        return {{"success", false}, {"error", "Process timed out."}};
    }

    int exitCode = process.exitCode();
    QString stdOut = QString::fromUtf8(process.readAllStandardOutput());
    QString stdErr = QString::fromUtf8(process.readAllStandardError());

    LOG_MODULE_DEBUG("ProcessDevice", QString("Process finished. ExitCode: %1, StdOut: %2, StdErr: %3")
        .arg(exitCode).arg(stdOut).arg(stdErr).toStdString());

    bool success = (process.exitStatus() == QProcess::NormalExit && exitCode == 0);

    return {
        {"success", success},
        {"exitCode", exitCode},
        {"stdout", stdOut},
        {"stderr", stdErr}
    };
}

IDevice::DeviceType ProcessDevice::getType() const
{
    return DeviceType::ProcessRunner;
}

IDevice::DeviceStatus ProcessDevice::getStatus() const { return m_status; }

QString ProcessDevice::getName() const { return "ProcessRunner"; }

QString ProcessDevice::getDescription() const { return "A virtual device to run external command-line processes."; }

void ProcessDevice::setCommunicationChannel(std::shared_ptr<Infrastructure::ICommunicationChannel>) {
    // ProcessDevice不使用通信通道
}

std::shared_ptr<Infrastructure::ICommunicationChannel> ProcessDevice::getCommunicationChannel() const {
    return nullptr;
}

void ProcessDevice::setConfiguration(const QJsonObject &config) {
    m_config = config;
}

QJsonObject ProcessDevice::getConfiguration() const {
    return m_config;
}

bool ProcessDevice::selfTest() {
    // 简单地返回true，因为没有硬件可以测试
    return true;
}

} // namespace Domain 
