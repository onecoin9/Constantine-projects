#pragma once
#include <QObject>

#include "ACProjManager.h"

/*********************   SSD   *********************/
// 512G SSD

#define		SIZE_SSD_TOTAL				(1024LL * 1024LL * 1024LL * 512LL)
#define		SIZE_MU_MANAGER				(1024LL * 1024LL * 1024LL * 256LL)
#define		SIZE_LITTLE_FS_MANAGER		(1024LL * 1024LL * 1024LL * 4LL)
#define		SIZE_PC_MU_BPU_COMM			(1024LL * 1024LL * 1024LL * 4LL)

#define		ADDR_SSD_START					(0LL)
#define		ADDR_MU_MANAGER_START			(ADDR_SSD_START)
#define		ADDR_LITTLE_FS_MANAGER_START	(ADDR_MU_MANAGER_START + SIZE_MU_MANAGER)
#define		ADDR_MU_BPU_COMM_START			(ADDR_LITTLE_FS_MANAGER_START + SIZE_LITTLE_FS_MANAGER)

#define		MAX_EMMC_PARTION_NUM			(64)
// MU_MANAGER detail
#define		EMMC_PARTION_HEADER_SIZE		(1024LL * 1024LL)

#define		SIZE_CACHE_PARTION				(1024LL * 1024LL * 256LL)
#define		SIZE_EMMC_PARTION_HEADER_LIST	(EMMC_PARTION_HEADER_SIZE * MAX_EMMC_PARTION_NUM)
#define		SIZE_EMMC_PARTION_DATA_LIST		(SIZE_MU_MANAGER - SIZE_CACHE_PARTION - SIZE_EMMC_PARTION_HEADER_LIST)

#define		ADDR_CACHE_PARTION				(ADDR_MU_MANAGER_START)
#define		ADDR_EMMC_PARTION_HEADER_LIST	(ADDR_CACHE_PARTION + SIZE_CACHE_PARTION)
#define		ADDR_EMMC_PARTION_DATA_LIST		(ADDR_EMMC_PARTION_HEADER_LIST + SIZE_EMMC_PARTION_HEADER_LIST)
#define		ADDR_DDR_PARTION_DATA_LIST		(1024LL * 1024LL * 898LL)

#define		ALIGNMENT_4K					(4 * 1024)


class ACSSDProjManager : public QObject {

	Q_OBJECT

public:
	ACSSDProjManager(const QString&, int);
	~ACSSDProjManager();

	//bool isProjExist(ACProjManager*);
	bool downLoadProject(ACProjManager*);

	uint64_t getSSDMaxIdelSize(bool bNeedUpdateTableHeadList = true);

	bool cacheTableHeadList();

	uint64_t getCurWritePartitionHeadAddr() {
		return mWritePartitionHeadAddr;
	};

	bool createSSDJsonFile(const QString& fileName = "ssd.json");
protected:
	bool freeSSDSize(uint64_t size, bool bNeedUpdateTableHeadList = true);
	bool freeOldestOneProj(bool bNeedUpdateTableHeadList = true);
	bool updateTableHeadListToSSD();

	bool binFilesCompare(int partionTableHeadIndex, ACProjManager* projManagerPtr);
signals:
	void sgnUpdateProjWriteProcess(int, QString, int);
private:
	uint64_t mWritePartitionHeadAddr;
	QMap<int, eMMCTableHeader> mCacheTableHeadMap;
	QMap<int, QString> mCacheTableJsonMap;
	QString mDevIp;
	int mHop;
};