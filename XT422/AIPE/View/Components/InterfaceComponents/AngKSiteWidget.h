#pragma once

#include <QtWidgets/QWidget>
#include "GlobalDefine.h"
#include "ACCmdPacket.h"
#include "DeviceModel.h"
namespace Ui { class AngKSiteWidget; };

class AngKSiteWidget : public QWidget
{
	Q_OBJECT

public:
	AngKSiteWidget(int idx, BPU_SKT_VALUE BPUValue, std::string strAdapterID, QWidget* parent = Q_NULLPTR);
	~AngKSiteWidget();

	void setSiteStatus(SiteStatus sitState, uint16_t resultCode = 0);

	void setSiteNumLabel(int index);

	void setSiteCheck(bool bClick);

	void setSiteProgress(int nValue);

	int GetSiteProgress();

	BPU_SKT_VALUE GetBPU_Value() { return m_BPU_Value; }

	bool GetSiteIsCheck();

	void SetAdapterID(std::string _nameID) { m_strAdapterID = _nameID; }

	void SetUseEnable(bool bEnable);

	bool GetUseEnable();

	void SetSKTInfo(int& nLifeCycleCnt, std::string& strUID);

	tSktInfo& GetSKTInfo();
signals:
	void sgnCheckAllSelect();

private:
	Ui::AngKSiteWidget *ui;
	SiteStatus m_curState;
	BPU_SKT_VALUE m_BPU_Value;
	std::string m_strAdapterID;
	tSktInfo siteSKTInfo;
};
