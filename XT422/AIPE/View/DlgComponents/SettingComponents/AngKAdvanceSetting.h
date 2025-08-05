#pragma once

#include "AngKDialog.h"
namespace Ui { class AngKAdvanceSetting; };

typedef struct tagAdvSetting
{
	bool bPasswdEn;			///使能Passwd   1为使能，0不使能
	bool bQuanCtlEn;		///使能量产控制模式
	bool bFactoryModeEn;	///使能工程模式，1为使能，0为不使能
	QString sPasswd;		///客户输入的密码
	QString sDevSN;			///绑定的机台SN
	uint nQuanCtlCnt;		///量产控制次数
	bool bBindWhenLoad;		///下载的时候才绑定
	bool bAdvDataProtect;	///高级数据加密是否使能
	QString sQuanCtlKey;	///量产校验时使用的秘钥
	uint nFactoryOpt;		///工厂模式下允许的操作
	QString sFactoryKey;	///工厂模式恢复到正常模式下的秘钥
	//tUSBKeyBindInfo USBKeyBind;///USB Key信息
	//tUIDInfo UIDInfoSetting;
	bool bPlistSelEN;		///是否绑定编程器
	QString strPlistPath;	///Plist文件路径
	void Init()
	{
		bPasswdEn = false;
		bQuanCtlEn = false;
		bFactoryModeEn = false;
		sPasswd = "";
		sDevSN = "";
		nQuanCtlCnt = 0;
		bBindWhenLoad = false;
		bPlistSelEN = false;
		strPlistPath = "";
		sQuanCtlKey = "";
		nFactoryOpt = 0;
		sFactoryKey = "";
		bAdvDataProtect = false;
		//USBKeyBind.bAdvDataProtect = FALSE;
		//USBKeyBind.bUSBKeyBindEn = FALSE;
		//USBKeyBind.nQuanCtlCnt = 0;
		//USBKeyBind.sQuanCtlKey = "";
		//USBKeyBind.USBKeySN = "";

		//UIDInfoSetting.bUIDEn = 1;
		//UIDInfoSetting.nUIDMODE = 2;
		//UIDInfoSetting.nEndType = 1;
		//UIDInfoSetting.uSwapUnit = 0;
	}
	struct tagAdvSetting() { Init(); };
}ADVSETTING;

class AngKAdvanceSetting : public AngKDialog
{
	Q_OBJECT

public:
	AngKAdvanceSetting(QWidget *parent = Q_NULLPTR);
	~AngKAdvanceSetting();

	void InitText();

	void InitSetting(ADVSETTING* _setting);
public slots:
	void onSlotSaveAdvanceSetting();
private:
	Ui::AngKAdvanceSetting *ui;
	ADVSETTING* m_pAdvSetting;
};
