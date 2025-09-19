#pragma once

#include <vector>
#include "../Com/Serial.h"


typedef struct tagSNGroup
{
	UINT64	llSNStartAddr;	///SN的起始地址
	UINT	dwSNLen;			///SN的长度
	CString strSeperated; ///烧录之后Pass的bin档案是否进行分离,如果为Yes则会在烧录成功之后将成功的Bin拷贝到Pass目录下
	CString strSampleBin;    ///SampleBin的位置
	CString strSNDir; ////SN　Bin档案所在目录
	CString strPassDir; ///内部维护的存在Pass文件的路径
	CString strMaxSize;
	CString strFillData;
	struct tagSNGroup(){
		ReInit();
	};
	void ReInit(){
		llSNStartAddr=0;
		dwSNLen=0;
		strSampleBin="";
		strSNDir="";
		strSeperated="Yes";
		strPassDir="";
		strMaxSize="";
		strFillData="";
	};
}SNGROUP;

class CSNDirFiles_VarSizeCfg
{
public:
	CSNDirFiles_VarSizeCfg(){ReInit();};
	~CSNDirFiles_VarSizeCfg(){bSNGroupNum=0;m_vSNGroup.clear();};
	void ReInit();
	BOOL SerialInCfgData(CSerial& lSerial);
	BOOL SerialOutCfgData(CSerial& lSerial);
	BOOL AppendGroup(INT nNum);
	BOOL RemoveGroup(INT nNum);
	INT GetGroupNum(){return bSNGroupNum;};
	SNGROUP* GetGroup(INT idx);
	//SNGROUP* GetGroup(CString strFilePath);
	BOOL CheckDataValid(CString& strErrMsg);
	INT GetSNTotalSize();
private:
	BYTE bSNGroupNum;
	std::vector<SNGROUP>m_vSNGroup;
};




#ifdef WIN32
#pragma pack(1)
#define PACK_ALIGN
#else
#define PACK_ALIGN __attribute__((packed))
#endif

#ifdef WIN32
#pragma pack()
#endif
#undef PACK_ALIGN