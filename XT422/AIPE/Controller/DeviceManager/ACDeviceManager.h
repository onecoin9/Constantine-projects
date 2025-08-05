#include <QString> 
#include <QMap>
#include <QObject>
#include <QMutex>
#include <memory>  
#include "DeviceModel.h"

class ACDeviceManager: public QObject {
	Q_OBJECT
public:
	static ACDeviceManager& instance()
	{
		static ACDeviceManager devManager;
		return devManager;
	}

	QString getErrorStr() { return mErrorStr; }
	bool devIsExist(const QString& ipStr, int hop);
	bool chainIsExist(const QString& ipStr);


	bool addDevChain(const QString& ipStr, const QMap<int, DeviceStu>& devStuMap = QMap<int, DeviceStu>());
	bool addDev(const QString& ipStr, int hop, const DeviceStu& devStu = DeviceStu());

	void delDevChain(const QString& ipStr);
	void delDev(const QString& ipStr, int hop);
	void delAllDev() { mDevChains.clear(); };

	bool setDevInfo(const QString& ipStr, int hop, const DeviceStu& devStu);

	QVector<DeviceStu> getAllDevInfo();
	QMap<int, DeviceStu> getChainDevInfoByIp(const QString& ipStr);
	QMap<QString, DeviceStu> getDevInfoMap();
	DeviceStu getDevInfo(const QString& ipStr, int hop);
	
	int getChainNum(const QString& ipStr);
	int getAllDevNum();
	QList<QString> getChainIpList();
	bool isDevieOnline(const QString& ipStr, int hop);

	void setDevEnable(QString ipStr, int hop, bool flag = true);
	void reStartDevOffLineTimer(const QString& ipStr, int hop);
private:
	explicit ACDeviceManager();
	~ACDeviceManager();
	QString mErrorStr;
	int mOffLineTime;

	QMap<QString, QMap<int, DeviceStu>> mDevChains;
	//QMap<QString, QMap<int, std::shared_ptr<QTimer>>> mDevChainsOffLineTimer;
	std::map<QString, std::map<int, qint64>> mDevChainsOffLineTimestamp;

	QMutex m_mutex;
	QTimer OffLineTimestamp;
signals:
	void devStateChanged();
	void devOffLine(QString ipStr, int hop);
	void devOnLine(QString ipStr, int hop);

public slots:
	void onSlotoffLine();
};