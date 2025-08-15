#pragma once

#include <vector>
#include "../Com/Serial.h"


typedef struct tagSNGroup
{
	UINT64	llSNStartAddr;	///SN����ʼ��ַ
	UINT	dwSNLen;			///SN�ĳ���
	CString strSeperated; ///��¼֮��Pass��bin�����Ƿ���з���,���ΪYes�������¼�ɹ�֮�󽫳ɹ���Bin������PassĿ¼��
	CString strSampleBin;    ///SampleBin��λ��
	CString strSNDir; ////SN��Bin��������Ŀ¼
	CString strPassDir; ///�ڲ�ά���Ĵ���Pass�ļ���·��
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