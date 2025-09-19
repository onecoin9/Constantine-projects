#include "domain/Dut.h"
#include <QDateTime>

namespace Domain {

Dut::Dut(const QString& id, QObject *parent)
    : QObject(parent), m_id(id), m_state(State::Idle), m_lastUpdateTime(QDateTime::currentDateTime())
{
}

Dut::~Dut()
{
}

QString Dut::getId() const
{
    return m_id;
}

Dut::State Dut::getState() const
{
    return m_state;
}

void Dut::setState(Dut::State newState)
{
    if (m_state != newState) {
        m_state = newState;
        m_lastUpdateTime = QDateTime::currentDateTime();
        qDebug()<<"dut::setState";
        emit stateChanged(m_id, newState);
    }
}

QDateTime Dut::getLastUpdateTime() const
{
    return m_lastUpdateTime;
}

QVariantMap Dut::getTestData() const
{
    return m_testData;
}

QString Dut::getCurrentSite() const
{
    return m_currentSite;
}

void Dut::setCurrentSite(const QString &site)
{
    if (m_currentSite != site) {
        m_currentSite = site;
        m_lastUpdateTime = QDateTime::currentDateTime();
        emit siteChanged(m_id, m_currentSite);
    }
}

void Dut::setTestData(const QVariantMap &data)
{
    if (m_testData != data) {
        m_testData = data;
        emit testDataChanged(m_id, m_testData);
    }
}

void Dut::updateTestData(const QVariantMap &dataToUpdate)
{
    bool changed = false;
    for (auto it = dataToUpdate.constBegin(); it != dataToUpdate.constEnd(); ++it) {
        if (!m_testData.contains(it.key()) || m_testData.value(it.key()) != it.value()) {
            m_testData[it.key()] = it.value();
            changed = true;
        }
    }
    if (changed) {
        emit testDataChanged(m_id, m_testData);
    }
}

} // namespace Domain 
