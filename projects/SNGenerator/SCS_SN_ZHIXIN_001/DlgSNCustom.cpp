// DlgSNCustom.cpp : 实现文件
//

#include "stdafx.h"
#include "SCS_SN_ZHIXIN_001.h"
#include "DlgSNCustom.h"

#include "../Com/cJSON.h"

#include <iostream>  
#include <string>  
#include <sstream>  

#define PREHEADZERO_NEED (0x0004)///在转换的过程中需要前导0
#define ShareMemSize (4096)
// CDlgSNCustom 对话框

BOOL New_Str2Hex( CString&str,UINT EndianType,std::vector<BYTE>&vDecDataOut)
{
	vDecDataOut.clear();
	BYTE AddInc=0,cData;
	BYTE ByteData;
	USHORT Tmp=0;
	INT i,len,j;
	std::vector<BYTE>vDecData;
	len=str.GetLength();
	if((EndianType&PREHEADZERO_NEED)==0){
		vDecData.push_back(AddInc);
		for(i=0;i<len;i++){
			cData=str.GetAt(i);
			if(isdigit(cData)){
				cData=cData-'0';
			}
			else if(cData>='A'&&cData<='F'){
				cData=cData-'A'+10;
			}
			else if(cData>='a'&&cData<='f'){
				cData=cData-'a'+10;
			}
			else{
				return FALSE;
			}
			for(j=0;j<(INT)vDecData.size();j++){
				if(j==0){
					Tmp=vDecData[j]*16+cData;
				}
				else{
					Tmp=vDecData[j]*16+AddInc;
				}
				AddInc=(Tmp>>8)&0xFF;
				vDecData[j]=(BYTE)Tmp;///写回去
			}
			if(AddInc!=0){//有进位需要再次产生一个
				vDecData.push_back(AddInc);
			}	
		}
	}
	else{///不能忽略前导0
		if(len%2!=0){///需要是2的整数倍
			return FALSE;
		}
		for(i=len-1;i>=0;i--){
			cData=str.GetAt(i);
			if(isdigit(cData)){
				cData=cData-'0';
			}
			else if(cData>='A'&&cData<='F'){
				cData=cData-'A'+10;
			}
			else if(cData>='a'&&cData<='f'){
				cData=cData-'a'+10;
			}
			else{
				return FALSE;
			}
			if(i%2==0){
				ByteData +=cData<<4;
				vDecData.push_back(ByteData);
			}
			else{
				ByteData=cData;

			}
		}
	}
	if(EndianType&2){
		for(i=(INT)vDecData.size()-1;i>=0;i--){
			vDecDataOut.push_back(vDecData[i]);
		}
	}
	else{
		vDecDataOut=vDecData;
	}
	return TRUE;
}


BOOL CDlgSNCustom::GetDataFromShareMem()
{
	CString strMsg;

	BOOL Ret = FALSE;
	int nSize = MAX_PATH;
	int pid = _getpid();
	CString strName;
	strName.Format("AC_MULTI_BurnSN_4_ZHIXIN");
	BYTE pReadMem[ShareMemSize] = {0};
	memset(pReadMem, 0, sizeof(BYTE));

	HANDLE hMap = OpenFileMapping(FILE_MAP_WRITE, FALSE, strName);
	if (hMap == NULL) {
		DWORD dwError = GetLastError();

		strMsg.Format("open file_map  fail %d", dwError);
		/*AfxMessageBox(strMsg);*/
		return Ret;
	}

	LPSTR pMapView = (LPSTR)MapViewOfFile(hMap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, ShareMemSize);
	if (pMapView == NULL) {
		/*AfxMessageBox("读写映射数据失败");*/
		CloseHandle(hMap); 
		return Ret;
	}

	memcpy(&pReadMem, pMapView, ShareMemSize);

	UnmapViewOfFile(pMapView);  
	CloseHandle(hMap);  

	m_vSNBurnInfo.clear();

	cJSON* pRoot = NULL;
	cJSON* pObj = NULL;
	int nGruop = 0;

	CString strReadData;
	strReadData.Format("%s", pReadMem);
	m_strJsonData.Format("%s", strReadData);

	pRoot=cJSON_Parse(strReadData.GetBuffer());
	strReadData.ReleaseBuffer();

	if(pRoot==NULL){
		goto __end;
	}

	pObj = cJSON_GetObjectItem(pRoot,"SNList");
	if(pObj == NULL){
		goto __end;
	}

	nGruop = cJSON_GetArraySize(pObj);
	if (nGruop == 0)
	{
		goto __end;
	}

	//
	for (int i = 0; i < nGruop; i++)
	{
		CString strItem;
		tSNBurnInfo SNBurnInfo;

		cJSON* pChild =cJSON_GetArrayItem(pObj, i);

		cJSON* pItem = cJSON_GetObjectItem(pChild,"Addr");
		if (pItem == NULL){
			goto __end ;
		}
		strItem.Format("%s", pItem->valuestring);
		strItem.Replace("0x", "");
		strItem.Replace("0X", "");

		sscanf(strItem.GetBuffer(), "%I64X", &SNBurnInfo.Addr);
		strItem.ReleaseBuffer();

		pItem = cJSON_GetObjectItem(pChild,"Len");
		if (pItem == NULL){
			goto __end ;
		}
		SNBurnInfo.Len = pItem->valueint;

		pItem = cJSON_GetObjectItem(pChild,"Data");
		if (pItem == NULL){
			goto __end ;
		}
		strItem.Format("%s", pItem->valuestring);	
		strItem.Replace("0x", "");
		strItem.Replace("0X", "");

		if (New_Str2Hex(strItem, 0x02|0x04, SNBurnInfo.vData) == FALSE ){
			AfxMessageBox("New_Str2Hex");
			goto __end ;
		}

		m_vSNBurnInfo.push_back(SNBurnInfo);
	}

	Ret = TRUE;

__end:

	if (Ret != TRUE)
	{
		m_vSNBurnInfo.clear();
	}else{
		
	}

	return Ret;
}


IMPLEMENT_DYNAMIC(CDlgSNCustom, CDialog)

CDlgSNCustom::CDlgSNCustom(DRVSNCFGPARA *pSnCfgPara,CWnd* pParent /*=NULL*/)
	: CDialog(CDlgSNCustom::IDD, pParent)
	, m_SNFixSize(0)
	, m_pSnCfgPara(pSnCfgPara)
	, m_SNStartAddr(0)
{
	GetDataFromShareMem();
}

CDlgSNCustom::~CDlgSNCustom()
{
}

BOOL CDlgSNCustom::InitCtrlsValue(CSNCustomCfg& SNCfg)
{
	tSNCustomCfg* pCfg=SNCfg.GetCustomCfg();
	CString strText;
	//m_strSNFix=pCfg->strSNFix;
	//m_SNFixSize=pCfg->SNFixSize;
	//m_SNStartAddr=pCfg->SNStartAddr;
	//strText.Format("%I64X",m_SNStartAddr);
	//GetDlgItem(IDC_EDIT3)->SetWindowText(strText);
	CWnd* pWnd = GetDlgItem(IDC_STATIC_JSON_CONTENT);
	if (pWnd != NULL)
	{
		pWnd->SetWindowText(m_strJsonData);
	}
	//UpdateData(FALSE);
	return TRUE;
}

BOOL CDlgSNCustom::InitCtrls(CSerial& lSerial)
{
	BOOL Ret=TRUE;
	if(lSerial.GetLength()!=0){
		Ret=m_SnCfg.SerialInCfgData(lSerial);
	}
	else{
	}
	Ret=InitCtrlsValue(m_SnCfg);
	return Ret;
}

BOOL CDlgSNCustom::GetCtrls(CSerial&lSerial)
{
	//UpdateData(TRUE);
	CString strText;
	//return TRUE;
	tSNCustomCfg* pCfg=m_SnCfg.GetCustomCfg();
	//GetDlgItem(IDC_EDIT3)->GetWindowText(strText);
	//if(sscanf(strText,"%I64X",&m_SNStartAddr)!=1){
	//	MessageBox("获取SN起始地址错误,请确认");
	//	return FALSE;
	//}
	//pCfg->strSNFix=m_strSNFix;
	//pCfg->SNFixSize=m_SNFixSize;
	//pCfg->SNStartAddr=m_SNStartAddr;
	//if(m_SNFixSize!=(m_strSNFix.GetLength()+1)/2){
	//	MessageBox("输入错误,固定序列号部分的字节数需要等于输入的字节数");
	//	return FALSE;
	//}
	return m_SnCfg.SerialOutCfgData(lSerial);
}

INT CDlgSNCustom::QuerySN(UINT AdapIdx,DWORD Idx,BYTE*pData,INT*pSize)
{
	INT Ret=0;
	std::vector<BYTE> vFixData,vTimeData;

	CSerial lSerial;
	UINT GroupCnt=m_vSNBurnInfo.size();
	
	if (GroupCnt == 0)
	{
		Ret = -1;
		goto __end;
	}

	lSerial<<GroupCnt;

	for (int i = 0; i < m_vSNBurnInfo.size(); i++)
	{
		tSNBurnInfo SNBurnInfo;
		SNBurnInfo = m_vSNBurnInfo[i];
		lSerial<<SNBurnInfo.Addr;
		lSerial<<SNBurnInfo.Len;
		
		CString strShow;
		strShow.Format("Addr:0x%I64X, LEN:%d", SNBurnInfo.Addr, SNBurnInfo.Len);
		//AfxMessageBox(strShow);

		if (SNBurnInfo.vData.size() != SNBurnInfo.Len)
		{
			Ret = -1;
			goto __end;
		}

		for (int j = 0/*SNBurnInfo.vData.size() -1*/; j < SNBurnInfo.vData.size() /*>=0*/ ; j++)
		{
			lSerial<<SNBurnInfo.vData[j];
		}
	}

	Ret=lSerial.GetLength();///返回值需要是正常填充的字节数
	memcpy(pData,lSerial.GetBuffer(),lSerial.GetLength());

__end:

	return Ret;
}

INT CDlgSNCustom::TellResult(DWORD Idx,INT IsPass)
{
	INT Ret=0;
	return Ret;
}

INT CDlgSNCustom::PreLoad()
{
	INT Ret=0;
	GetDataFromShareMem();
	return Ret;
}

void CDlgSNCustom::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_SNFixSize);
	DDX_Text(pDX, IDC_EDIT2, m_strSNFix);
}


BEGIN_MESSAGE_MAP(CDlgSNCustom, CDialog)
END_MESSAGE_MAP()


// CDlgSNCustom 消息处理程序

//BOOL CDlgSNCustom::OnInitDialog()
//{
//	CDialog::OnInitDialog();
//
//	// TODO:  在此添加额外的初始化
//#if 0
//	CString strText;
//	strText.Format("%d",m_SNFixSize);
//	GetDlgItem(IDC_EDIT1)->SetWindowText(strText);
//	GetDlgItem(IDC_EDIT2)->SetWindowText(m_strSNFix);
//#endif 
//	GetDlgItem(IDC_EDIT2)->SetFocus();
//	return FALSE;
//	//return TRUE;  // return TRUE unless you set the focus to a control
//	// 异常: OCX 属性页应返回 FALSE
//}
