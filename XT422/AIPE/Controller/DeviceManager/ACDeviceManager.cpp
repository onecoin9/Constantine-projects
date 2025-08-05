#include "ACDeviceManager.h"
#include "AngKPathResolve.h"
#include "MessageNotify/notifyManager.h"
#include "../GlobalInit/AppModeType.h"
#include "AngkLogger.h"
#include <QSettings>
#include <QDebug>
#include <QMutexLocker>


ACDeviceManager::ACDeviceManager() : m_mutex(QMutex::Recursive) {
	QObject::connect(this, &ACDeviceManager::devOffLine, [](QString ipStr, int hop) {
		NotifyManager::instance().notify(tr("Notify"), tr("Device %1 is out line!").arg(ipStr + ":" + QString::number(hop)));
		ALOG_FATAL("Device %s:%d is out line!", "CU", "--", ipStr.toStdString().c_str(), hop);


	});
	QObject::connect(this, &ACDeviceManager::devOnLine, [](QString ipStr, int hop) {
		NotifyManager::instance().notify(tr("Notify"), tr("Device %1 has reconnected!").arg(ipStr + ":" + QString::number(hop)));
		ALOG_WARN("Device %s:%d has reconnected!", "CU", "--", ipStr.toStdString().c_str(), hop);
	});

	OffLineTimestamp.start(5000);
	connect(&OffLineTimestamp, &QTimer::timeout, this, &ACDeviceManager::onSlotoffLine);
}

ACDeviceManager::~ACDeviceManager() {
	OffLineTimestamp.stop();
}

bool ACDeviceManager::devIsExist(const QString& ipStr, int hop) {

	QMutexLocker locker(&m_mutex);
	if (mDevChains.find(ipStr) == mDevChains.end())
		return false;

	auto chainMap = mDevChains.find(ipStr).value();
	if (chainMap.find(hop) != chainMap.end())
	{
		return true;
	}

	return false;
}

bool ACDeviceManager::chainIsExist(const QString& ipStr) {
	QMutexLocker locker(&m_mutex);
	return mDevChains.find(ipStr) != mDevChains.end();
}


bool ACDeviceManager::addDevChain(const QString& ipStr, const QMap<int, DeviceStu>& devStuMap) {
	
	QMutexLocker locker(&m_mutex);
	if (mDevChains.find(ipStr) != mDevChains.end())
	{
		mErrorStr = QString("Chain ").append(ipStr).append(" already exists!");
		return false;
	}
	//locker.unlock();

	QMap<int, std::shared_ptr<QTimer>> chainTimerMap;
	for (auto iter = devStuMap.begin(); iter != devStuMap.end(); iter++) {
		//QSettings tmpSettings(Utils::AngKPathResolve::localGlobalSettingFile(), QSettings::IniFormat);
		//std::shared_ptr<QTimer> offLineTimer(new QTimer());
		//chainTimerMap.insert(iter.key(), offLineTimer);
		//offLineTimer->setInterval(tmpSettings.value("DeviceComm/HeartBeatTimeoutms", 5000).toInt());
		int hop = iter.key();
		//QObject::connect(offLineTimer.get(), &QTimer::timeout, [this, &ipStr, &hop]() {
		//	if (g_AppMode == ConnectType::Demo)
		//		return;
		//	QMutexLocker locker(&m_mutex);
		//	if (mDevChains.find(ipStr) != mDevChains.end() && mDevChains[ipStr].find(hop) != mDevChains[ipStr].end())
		//		mDevChains[ipStr].find(hop).value().bOnLine = false;
		//	emit devOffLine(ipStr, hop);
		//});
		//offLineTimer.get()->start();
		std::map<int, qint64> chainTimerstampMap;
		chainTimerstampMap[hop] = QDateTime::currentDateTime().toSecsSinceEpoch();
		mDevChainsOffLineTimestamp[ipStr] = chainTimerstampMap;
	}

	//locker.relock();
	//mDevChainsOffLineTimer[ipStr] = chainTimerMap;

	mDevChains[ipStr] = devStuMap;
	//for (auto it : chainTimerMap) {
	//	it.get()->start();
	//}

	emit devStateChanged();

	return true;
}

bool ACDeviceManager::addDev(const QString& ipStr, int hop, const DeviceStu& devStu) {
	// 不存在该链，添加新链

	QMutexLocker locker(&m_mutex);
	if (mDevChains.find(ipStr) == mDevChains.end()) {
		QMap<int, DeviceStu> newDevStuMap;
		newDevStuMap.insert(hop, devStu);


		//QMap<int, std::shared_ptr<QTimer>> chainTimerMap;
		//std::shared_ptr<QTimer> offLineTimer = std::make_shared<QTimer>();
		//chainTimerMap.insert(hop, offLineTimer);
		//QSettings tmpSettings(Utils::AngKPathResolve::localGlobalSettingFile(), QSettings::IniFormat);
		//offLineTimer->setInterval(tmpSettings.value("DeviceComm/HeartBeatTimeoutms", 5000).toInt());
		//offLineTimer->setSingleShot(true);
		//QObject::connect(offLineTimer.get(), &QTimer::timeout, [this, ipStr, hop]() {
		//	if (g_AppMode == ConnectType::Demo)
		//		return;
		//	mDevChains[ipStr].find(hop).value().bOnLine = false;
		//	emit devOffLine(ipStr, hop);
		//});

		std::map<int, qint64> chainTimerstampMap;
		chainTimerstampMap[hop] = QDateTime::currentDateTime().toSecsSinceEpoch();
		mDevChainsOffLineTimestamp[ipStr] = chainTimerstampMap;
		//mDevChainsOffLineTimer[ipStr] = chainTimerMap;
		mDevChains[ipStr] = newDevStuMap;
		//offLineTimer.get()->start();

		emit devStateChanged();
		return true;
	}

	// 链上已存在该设备，添加失败
	auto& chainMap = mDevChains.find(ipStr).value();
	if (chainMap.find(hop) != chainMap.end())
	{
		mErrorStr = QString(ipStr).append(":").append(hop).append(" already exists!");
		return false;
	}
	else {
		// 添加定时器
		//std::shared_ptr<QTimer> offLineTimer = std::make_shared<QTimer>();
		//QSettings tmpSettings(Utils::AngKPathResolve::localGlobalSettingFile(), QSettings::IniFormat);
		//offLineTimer->setInterval(tmpSettings.value("DeviceComm/HeartBeatTimeoutms", 5000).toInt());
		//offLineTimer->setSingleShot(true);
		//QObject::connect(offLineTimer.get(), &QTimer::timeout, [this, ipStr, hop]() {
		//	if (g_AppMode == ConnectType::Demo)
		//		return;
		//	mDevChains[ipStr].find(hop).value().bOnLine = false;
		//	emit devOffLine(ipStr, hop);
		//});
		//mDevChainsOffLineTimer[ipStr].insert(hop, offLineTimer);
		std::map<int, qint64> chainTimerstampMap;
		chainTimerstampMap[hop] = QDateTime::currentDateTime().toSecsSinceEpoch();
		mDevChainsOffLineTimestamp[ipStr] = chainTimerstampMap;
		chainMap.insert(hop, devStu);
		//offLineTimer.get()->start();


		// 添加设备
		emit devStateChanged();
		return true;
	}


}

void ACDeviceManager::delDevChain(const QString& ipStr) {

	QMutexLocker locker(&m_mutex);
	if (mDevChains.find(ipStr) == mDevChains.end()) {
		return;
	}
	//for (auto it : mDevChainsOffLineTimer.find(ipStr).value()) {
	//	it.get()->stop();
	//}
	//mDevChainsOffLineTimer.remove(ipStr);
	auto iter = mDevChainsOffLineTimestamp.find(ipStr);
	if (iter != mDevChainsOffLineTimestamp.end())
	{
		mDevChainsOffLineTimestamp.erase(iter);
	}
	mDevChains.remove(ipStr);
	emit devStateChanged();
}

void ACDeviceManager::delDev(const QString& ipStr, int hop) {

	QMutexLocker locker(&m_mutex);
	if (mDevChains.find(ipStr) == mDevChains.end())
		return;
	if (mDevChains.find(ipStr).value().find(hop) == mDevChains.find(ipStr).value().end())
		return;

	//mDevChainsOffLineTimer.find(ipStr).value().find(hop).value().get()->stop();
	mDevChains.find(ipStr).value().remove(hop);
	//mDevChainsOffLineTimer.find(ipStr).value().remove(hop);

	auto iter = mDevChainsOffLineTimestamp.find(ipStr);
	if (iter != mDevChainsOffLineTimestamp.end())
	{
		mDevChainsOffLineTimestamp.erase(iter);
	}

	emit devStateChanged();

}

bool ACDeviceManager::setDevInfo(const QString& ipStr, int hop, const DeviceStu& devStu) {
	if (devIsExist(ipStr, hop)) {

		QMutexLocker locker(&m_mutex);
		mDevChains.find(ipStr).value().find(hop).value() = devStu;
		emit devStateChanged();
		return true;
	}
	else {	// 设备不存在
		mErrorStr = QString(ipStr).append(":").append(hop).append(" is not exist!");
		return false;
	}
}


QVector<DeviceStu> ACDeviceManager::getAllDevInfo() {

	QMutexLocker locker(&m_mutex);
	QVector<DeviceStu> retVector;
	for (auto chainIter : mDevChains) {
		for (auto devIter : chainIter) {
			retVector.push_back(devIter);
		}
	}
	return retVector;
}

QMap<int, DeviceStu> ACDeviceManager::getChainDevInfoByIp(const QString& ipStr) {

	QMutexLocker locker(&m_mutex);
	if (mDevChains.find(ipStr) == mDevChains.end())
	{
		return QMap<int, DeviceStu>();
	}

	return mDevChains.find(ipStr).value();
}

DeviceStu ACDeviceManager::getDevInfo(const QString& ipStr, int hop) {
	if (!devIsExist(ipStr, hop)) {
		return DeviceStu();
	}

	QMutexLocker locker(&m_mutex);
	return mDevChains.find(ipStr).value().find(hop).value();
}

int ACDeviceManager::getChainNum(const QString& ipStr) {

	QMutexLocker locker(&m_mutex);
	if (mDevChains.find(ipStr) == mDevChains.end())
		return 0;
	return mDevChains.find(ipStr).value().size();
}

int ACDeviceManager::getAllDevNum() {

	QMutexLocker locker(&m_mutex);
	int ret = 0;
	for (auto chainIter : mDevChains) {
		ret += chainIter.size();
	}
	return ret;
}

QList<QString> ACDeviceManager::getChainIpList() {
	return mDevChains.keys();
}

QMap<QString, DeviceStu> ACDeviceManager::getDevInfoMap() {

	QMutexLocker locker(&m_mutex);
	QMap<QString, DeviceStu> retMap;
	for (auto chainIter = mDevChains.begin(); chainIter != mDevChains.end(); chainIter++) {
		for (auto devIter = chainIter.value().begin(); devIter != chainIter.value().end(); devIter++) {
			retMap.insert(chainIter.key() + ":" + QString::number(devIter.key()), devIter.value());
		}
	}
	return retMap;
}

void ACDeviceManager::reStartDevOffLineTimer(const QString& ipStr, int hop) {
	if (!devIsExist(ipStr, hop)) {
		return;
	}

	QMutexLocker locker(&m_mutex);
	//auto& offLineTimer = mDevChainsOffLineTimer.find(ipStr);
	//if (offLineTimer != mDevChainsOffLineTimer.end())
	//{
	//	auto& offLineTimerHop = offLineTimer.value().find(hop);
	//	if (offLineTimerHop != offLineTimer.value().end()) {
	//		//offLineTimerHop.value().get()->stop();
	//		//offLineTimerHop.value().get()->setSingleShot(true);
	//		QSettings tmpSettings(Utils::AngKPathResolve::localGlobalSettingFile(), QSettings::IniFormat);
	//		offLineTimerHop.value()->start(tmpSettings.value("DeviceComm/HeartBeatTimeoutms", 5000).toInt());
	//	}
	//}
	bool bFind = false;
	for (auto& devIpstr : mDevChainsOffLineTimestamp) {
		for (auto& OffLineTimestamp : devIpstr.second) {
			if (devIpstr.first == ipStr && OffLineTimestamp.first == hop) {
				OffLineTimestamp.second = QDateTime::currentDateTime().toSecsSinceEpoch();
				bFind = true;
			}
		}
	}

	if (!bFind) {
		//setDevEnable(ipStr, hop, true);
	}
}

bool ACDeviceManager::isDevieOnline(const QString& ipStr, int hop) {
	if (!devIsExist(ipStr, hop)) {
		return false;
	}

	QMutexLocker locker(&m_mutex);
	auto devInfo = mDevChains.find(ipStr).value().find(hop).value();
	return devInfo.bOnLine;
}

void ACDeviceManager::setDevEnable(QString ipStr, int hop, bool flag) {
	if (!devIsExist(ipStr, hop)) {
		return;
	}

	QMutexLocker locker(&m_mutex);
	if (flag) {
		//mDevChainsOffLineTimer.find(ipStr).value().find(hop).value().get()->stop();
		//QSettings tmpSettings(Utils::AngKPathResolve::localGlobalSettingFile(), QSettings::IniFormat);
		//mDevChainsOffLineTimer.find(ipStr).value().find(hop).value().get()->start(tmpSettings.value("DeviceComm/HeartBeatTimeoutms", 5000).toInt());
		mDevChains.find(ipStr).value().find(hop).value().bOnLine = true;
		std::map<int, qint64> chainTimerstampMap;
		chainTimerstampMap[hop] = QDateTime::currentDateTime().toSecsSinceEpoch();
		mDevChainsOffLineTimestamp[ipStr] = chainTimerstampMap;
		emit devOnLine(ipStr, hop);
	}
	else {
		mDevChains.find(ipStr).value().find(hop).value().bOnLine = false;
		emit devOffLine(ipStr, hop);
	}
}

void ACDeviceManager::onSlotoffLine()
{
	//QMutexLocker locker(&m_mutex);
	if (!mDevChainsOffLineTimestamp.empty()) {
		for (const auto& devIpstr : mDevChainsOffLineTimestamp) {
			for (const auto& OffLineTimestamp : devIpstr.second) {
				int tamp = QDateTime::currentDateTime().toSecsSinceEpoch() - OffLineTimestamp.second;
				if (tamp > 5) {
					if (g_AppMode == ConnectType::Demo)
						return;

					mDevChains.find(devIpstr.first).value().find(OffLineTimestamp.first).value().bOnLine = false;
					emit devOffLine(devIpstr.first, OffLineTimestamp.first);
					m_mutex.lock();
					mDevChainsOffLineTimestamp.erase(devIpstr.first);
					m_mutex.unlock();
					return;
				}
			}
		}
	}
}