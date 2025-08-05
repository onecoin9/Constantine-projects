#pragma once

#include <QWidget>
#include "ACCmdPacket.h"
#include "GlobalDefine.h"
#include "DeviceModel.h"
namespace Ui { class AngKProgrammerWidget; };

class AngKSiteWidget;
class AngKProgrammerWidget : public QWidget
{
	Q_OBJECT

public:
	AngKProgrammerWidget(int nIndex, QString ipHop = "", QWidget* parent = Q_NULLPTR);
	~AngKProgrammerWidget();

	void InitWidget();

	void InitBPUSite();

	void setProgramerIndex(QString strProgName);

	void setProgramerName(QString strText);

	void setBPUProgress(int nBPUID, int nVal);

	void setProgramerCheck(bool bCheck);

	uint32_t GetSitesChecked(int& siteCount);

	void ProjPropSwitchSiteEnable(std::vector<int> vecSite);

	bool GetProgramerCheck();

	QString GetDevProgramIndex();

	//void SetDevInfo(DeviceStu devStu);

	DeviceStu GetDevInfo();

	void SetBPUResultStatus(int nBPUIdx, int nSktInfo, QString RetCode, std::string strExtMsg);

	void RestSiteStatus();

	int GetProgramProgress();

	void setDevDescription(const QString& ipHop) { this->m_strIPHop = ipHop; }

	QString GetIpHop() { return m_strIPHop; }

	void SetDevConnectState(bool state);
	bool GetDevConnectState() { return m_ConnectState; };

	void SetProjBindBPUState(int nSktEn, std::string strAdapterID);

	void SetProjBindBPUState(int nSktEn);

	/// <summary>
	/// 重置AG06-Prog的烧录选择
	/// </summary>
	/// <param name="nHopNum">AG06的所属编号</param>
	void ResetSiteCheck(int nHopNum);

	void SetSiteStatus(int nSktEnable, SiteStatus typeStatu);

	void SetBPUCntState(uint32_t& nTrueSktEn, int& nLifeCycleCnt, std::string& strUID);

	void GetBPUExeCnt(uint32_t& nTotalNum, uint32_t& nFailNum);

	void SetDevBurnStatus(bool burnStatus);
private:
	/// <summary>
	/// 将界面生成的顺序索引转换为烧录器上展示的索引
	/// </summary>
	/// <param name="_idx">界面逻辑索引号</param>
	/// <returns></returns>
	int IndexSwitchSiteIdx(int _idx);

	/// <summary>
	/// 根据SiteIdx从BPUInfo中获取烧录座型号并设置
	/// </summary>
	/// <param name="nSiteIdx">Site索引号</param>
	/// <returns></returns>
	std::string GetAdapterID(int nSiteIdx, bool& bFind);

	void InitProgress();

	void CalProgAllSiteProgress();
signals:
	void sgnCurrentAllSelect(bool);

public slots:

	void onSlotExpandWidget(int);

	void onSlotCheckAllSelect();
private:
	Ui::AngKProgrammerWidget *ui;
	//std::vector<SingleBPU> m_BPUSite;
	std::map<int, AngKSiteWidget*> m_mapSiteWgt;
	QString m_strProgName;
	//DeviceStu	m_selfDev;
	QString m_strIPHop;//唯一确定一个设备，通过该变量查设备管理类

	bool m_ConnectState;
};
