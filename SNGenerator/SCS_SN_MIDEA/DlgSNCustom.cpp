// DlgSNCustom.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "SCS_SN_MIDEA.h"
#include "DlgSNCustom.h"
#include "../Com/ComTool.h"
#include "../Com/ComFunc.h"
#include "../Com/cJSON.h"

//#import "d:\\sn\\Midea.MES.SN.Util.tlb" named_guids raw_interfaces_only
#import "./Midea.MES.SN.Util.tlb" named_guids raw_interfaces_only


#define BUFFER_TYPE_SN_REMAP (17)
#define SNBLOCK (0x02)
#define VERSION (0x0001)

// CDlgSNCustom �Ի���

IMPLEMENT_DYNAMIC(CDlgSNCustom, CDialog)

#define DataLenth 255

CDlgSNCustom::CDlgSNCustom(DRVSNCFGPARA *pSnCfgPara,CWnd* pParent /*=NULL*/)
	: CDialog(CDlgSNCustom::IDD, pParent)
	, m_pSnCfgPara(pSnCfgPara)
	,AddOption(FALSE)
	,Changeover(FALSE)
{
	tSNCustomCfg* pCfg=m_SnCfg.GetCustomCfg();
}

CDlgSNCustom::~CDlgSNCustom()
{
}

BOOL CDlgSNCustom::GetComboValue(BOOL IsInit)
{
	BOOL Ret = FALSE;
	CString strJsonPath;
	strJsonPath.Format("%s\\sngen\\midea\\SNInfoCfg.json", CComFunc::GetCurrentPath());
	INT Size =0;
	BYTE *pData = NULL;
	CString json;
	CFile File;
	CString test;
	CString strProductsName;
	CString strChipName;
	
	cJSON *pRootParser = NULL;
	cJSON *RootSNInfos = NULL;
	cJSON *JsonHeader = NULL;
	cJSON *JsonValue = NULL;
	tSNCustomCfg* pCfg=m_SnCfg.GetCustomCfg();

	INT Array_size;

	if (!ParseTagInfo()){//����Tagʧ��
		goto __end;
	}
	
	if (File.Open(strJsonPath, CFile::modeRead | CFile::shareDenyNone) == FALSE) {
		MessageBox("��Json�ļ�ʧ��,��ȷ���ļ�·��!");
		goto __end;
	}
	Size = (INT)File.GetLength();
	pData = new BYTE[Size+1];
	if (!pData) {
		goto __end;
	}
	memset(pData, 0, Size + 1);
	File.Read(pData, Size);

	json.Format("%s", (pData));

	pRootParser = cJSON_Parse(json.GetBuffer());
	if (pRootParser == NULL) {
		MessageBox("������Json���ݸ�ʽ");
		goto __end;
	}

	RootSNInfos = cJSON_GetObjectItem(pRootParser, "SNInfos");
	Array_size = cJSON_GetArraySize(RootSNInfos);
	for (int i = 0; i < Array_size; i++)
	{
		JsonHeader = cJSON_GetArrayItem(RootSNInfos, i);

		JsonValue = cJSON_GetObjectItem(JsonHeader, "ProductName");
		if (JsonValue == NULL) {
			MessageBox("����ProductName�ֶδ���");
			goto __end;
		}
		pCfg->ProductsName = JsonValue->valuestring;

		////оƬ���ж�
		ChipNameLen = pCfg->ProductsName.GetLength()-3;
		strProductsName = pCfg->ProductsName.Left(ChipNameLen);
		strChipName = m_strAprChipName;
		if (m_strAprChipName.IsEmpty()){//���û�м��ع��̣������������������
			if(IsInit){
				((CComboBox *)GetDlgItem(IDC_COMBO_PRODUCTS))->AddString(pCfg->ProductsName);	
			}
		}else{
			if (strChipName.Find(strProductsName) < 0 ){				continue;			}
			/*if (strProductsName.CompareNoCase(strChipName)!=0 ){				continue;			}*/
			if(IsInit){//������ֻ��ʾtag��оƬ��Ϣ
				((CComboBox *)GetDlgItem(IDC_COMBO_PRODUCTS))->AddString(pCfg->ProductsName);	
				if (!m_tCfgInI.ProductsName.IsEmpty() && pCfg->ProductsName.Compare(m_tCfgInI.ProductsName) == 0){
					if (m_tCfgInI.SNLen == 10){
						((CComboBox *)GetDlgItem(IDC_COMBO_PRODUCTS))->SetCurSel(0);
					}else{
						((CComboBox *)GetDlgItem(IDC_COMBO_PRODUCTS))->SetCurSel(1);
					}
				}
				
			}
		}
		
		if(IsInit && !m_tCfgInI.ProductsName.IsEmpty() && pCfg->ProductsName.Compare(m_tCfgInI.ProductsName) == 0){
			((CComboBox *)GetDlgItem(IDC_COMBO_PRODUCTS))->SetCurSel(i);
		}
		
		((CComboBox *)GetDlgItem(IDC_COMBO_PRODUCTS))->GetWindowText(m_strProducts);//�����������е�оƬ��

		if (m_strProducts.CompareNoCase(pCfg->ProductsName) == 0) {

			JsonValue = cJSON_GetObjectItem(JsonHeader,"SNAddr");
			if (JsonValue == NULL) {
				MessageBox("����SNAddr�ֶδ���");
				goto __end;
			}
			pCfg->SNAddr = atoi(JsonValue->valuestring);
			m_strSNAddr.Format("%s",JsonValue->valuestring);
			GetDlgItem(IDC_EDIT_SN_ADDR)->SetWindowText(m_strSNAddr);
			sscanf(m_strSNAddr, "%I64X", &m_SNInfo.SNAddr);

			JsonValue = cJSON_GetObjectItem(JsonHeader,"Size");
			if (JsonValue == NULL) {
				MessageBox("����Size�ֶδ���");
				goto __end;
			}
			pCfg->SNLen = (UINT)JsonValue->valueint;
			m_strSNLen.Format("%d",JsonValue->valueint);
			GetDlgItem(IDC_EDIT_SN_LEN)->SetWindowText(m_strSNLen);
			m_SNInfo.SNLen = JsonValue->valueint;

			/*JsonValue = cJSON_GetObjectItem(JsonHeader,"SNBlock");
			if (JsonValue == NULL) {
				MessageBox("����SNBlock�ֶδ���");
				goto __end;
			}
			m_SNInfo.SNBlock = JsonValue->valueint;*/

			//JsonValue = cJSON_GetObjectItem(JsonHeader,"BufAddr");
			//if (JsonValue == NULL) {
			//	MessageBox("����BufAddr�ֶδ���");
			//	goto __end;
			//}
			//m_strSNAddr.Format("%s",JsonValue->valuestring);
			//sscanf(m_strSNAddr, "%I64X", &pCfg->BufAddr);
			//if (!m_strTagInfo.IsEmpty() && m_strSNAddr.CompareNoCase(m_strBufAddr)!=0){//���json�����buf��ַ��TAG��ͬ
			//	MessageBox("json�ļ���buffer��ַ��������ͬ");
			//	goto __end;
			//}

			sscanf(m_strBufAddr, "%I64X", &pCfg->BufAddr);

			JsonValue = cJSON_GetObjectItem(JsonHeader,"OptionAddr");
			if (JsonValue != NULL  && m_strManuName.CompareNoCase("ABOV")==0 && pCfg->SNLen == 10){//ABOVоƬ��¼ID����Ҫ����һ��Option����
				AddOption = TRUE;
				sscanf(JsonValue->valuestring, "%I64X", &pCfg->OptionAddr);
				((CButton *)GetDlgItem(IDC_CHECK_MODE))->SetCheck(1);
			}
			if (m_strManuName.CompareNoCase("ABOV")==0 && pCfg->SNLen == 32){
				((CButton *)GetDlgItem(IDC_CHECK_MODE))->SetCheck(0);
			}
			if (m_strManuName.CompareNoCase("ABOV")==0 && JsonValue == NULL && pCfg->SNLen == 10){
				MessageBox("ABOVоƬ��¼10λID������json�����Option��ַ");
				goto __end;
			}

			JsonValue = cJSON_GetObjectItem(JsonHeader,"Changeover");
			if (JsonValue != NULL && pCfg->SNLen == 32){//9062SN����Ҫ��ת˳��
				Changeover = TRUE;
			}
			
			SaveCtrls();
		}
	}
	Ret = TRUE;

__end:

	if (File.m_hFile != CFile::hFileNull) {
		File.Close();
	}
	if (pData) {
		delete[] pData;
	}
	if (pRootParser != NULL) {
		cJSON_Delete(pRootParser);
		pRootParser = NULL;
	}
	return Ret;

}

BOOL CDlgSNCustom::InitCtrlsValue(CSNCustomCfg& SNCfg)
{
	BOOL Ret = TRUE;
	CHAR TmpBuf[512];
	INT DataMode;
	CString ChipName;
	LoadInI();
	
	CString strIniFile;
	strIniFile.Format("%s\\sngen\\SCS_SN_MIDEA.ini", CComFunc::GetCurrentPath());

	DataMode = GetPrivateProfileInt("Config", "DataMode",0, strIniFile);
	GetPrivateProfileString("Config","AprChipName","",TmpBuf,MAX_PATH,strIniFile);
	ChipName.Format("%s",TmpBuf);
	if (m_strAprChipName.CompareNoCase(ChipName)==0){
		((CButton *)GetDlgItem(IDC_CHECK_MODE))->SetCheck(DataMode);
	}
	Ret = GetComboValue(TRUE);
	if (m_strManuName.CompareNoCase("BYD")==0 ||m_strManuName.CompareNoCase("SinoWealth")==0)
	{
		((CButton *)GetDlgItem(IDC_CHECK_MODE))->EnableWindow(0);
	}
	
	UpdateData(FALSE);

	return Ret;
}

BOOL CDlgSNCustom::InitCtrls(CSerial& lSerial)
{
	BOOL Ret=TRUE;
	CString strErrMsg;
	if(lSerial.GetLength()!=0){///�ٴδ򿪶Ի����ʱ��ͻᴫ�룬��֮ǰ����Ϊ��Դ���ͷţ�������Ҫ���´��ļ�
		Ret=m_SnCfg.SerialInCfgData(lSerial);
		if(Ret==TRUE){
			tSNCustomCfg* pCfg=m_SnCfg.GetCustomCfg();
		}
	}
	else{///�״μ��ص�ʱ��lSerial��û��ֵ
	}

	Ret=InitCtrlsValue(m_SnCfg);
	return Ret;
}

BOOL CDlgSNCustom::SaveCtrls()
{
	UpdateData(TRUE);
	CString strText;
	tSNCustomCfg* pCfg=m_SnCfg.GetCustomCfg();

	INT DataMode = ((CButton *)GetDlgItem(IDC_CHECK_MODE))->GetCheck();

	GetDlgItem(IDC_EDIT_SN_ADDR)->GetWindowText(strText);
	if(sscanf(strText,"%I64X",&pCfg->SNAddr)!=1){
	}

	GetDlgItem(IDC_EDIT_SN_LEN)->GetWindowText(strText);
	if(sscanf(strText,"%d",&pCfg->SNLen)!=1){
	}

	//////////////
	CString strIniFile;
	CString strTemp;
	strIniFile.Format("%s\\sngen\\SCS_SN_MIDEA.ini", CComFunc::GetCurrentPath());

	strTemp.Format("0x%I64X", pCfg->SNAddr);
	WritePrivateProfileString("Config", "SNAddr", (LPCTSTR)strTemp, strIniFile);
	
	strTemp.Format("%d", pCfg->SNLen);
	WritePrivateProfileString("Config", "SNLen", (LPCTSTR)strTemp, strIniFile);

	((CComboBox *)GetDlgItem(IDC_COMBO_PRODUCTS))->GetWindowText(ProductsName);
	m_tCfgInI.ProductsName = ProductsName;
	strTemp.Format("%s", m_tCfgInI.ProductsName);
	WritePrivateProfileString("Config", "Products", (LPCSTR)m_strProducts, strIniFile);

	strTemp.Format("%d", DataMode);
	WritePrivateProfileString("Config", "DataMode", (LPCSTR)strTemp, strIniFile);

	strTemp.Format("%s", m_strAprChipName);
	WritePrivateProfileString("Config", "AprChipName", (LPCSTR)strTemp, strIniFile);

	return TRUE;
}

BOOL CDlgSNCustom::GetCtrls(CSerial&lSerial)
{
	BOOL Ret=TRUE;
	CString strmsg;
	if (ISMessage && (m_strManuName.CompareNoCase("BYD")==0 ||m_strManuName.CompareNoCase("SinoWealth")==0)){
		MessageBox("��ӱ���ǵ�оƬ��¼����ΪInformation Block����Ϊ�����¼����ȷ��");
		ISMessage= FALSE;
	}
	INT DataMode = ((CButton *)GetDlgItem(IDC_CHECK_MODE))->GetCheck();
	if(ISMessage){
		if (!DataMode){
			strmsg.Format("��¼ǰ��ȷ�����ݴ�ŷ�ʽ�Ƿ���ȷ,��ǰΪ���ģʽ");
			MessageBox(strmsg);
			ISMessage= FALSE;
		}else{
			strmsg.Format("��¼ǰ��ȷ�����ݴ�ŷ�ʽ�Ƿ���ȷ,��ǰΪС��ģʽ");
			MessageBox(strmsg);
			ISMessage= FALSE;
		}		
	}

	
	Ret=SaveCtrls();
	if(Ret!=TRUE){
		return Ret;
	}
	return m_SnCfg.SerialOutCfgData(lSerial);
}

INT CDlgSNCustom::QuerySN(UINT AdapIdx, DWORD Idx,BYTE*pData,INT*pSize)
{
	INT Ret=0;
	INT Result;
	CString strText;
	CSerial lSerial;
	CString strGetSn;
	UINT GroupCnt=1;///�̶�Ϊһ��
	tSNCustomCfg* pCfg=m_SnCfg.GetCustomCfg();
	CString strMsg;
	CString ChipName;
	CString temp;
	BYTE m_Mac[32];
	std::vector<BYTE> vMac;

	INT  DataMode;
	UINT Lenth;

	INT HeaderReserve[5]={0};

	CString strIniFile;
	strIniFile.Format("%s\\sngen\\SCS_SN_MIDEA.ini", CComFunc::GetCurrentPath());

	m_SNType.Version =VERSION;
	m_SNType.SNInfoCnt = 1;
	CString strMagic = "SNALV";
	memset(m_SNType.MagicNum,0,sizeof(m_SNType.MagicNum));
	memcpy(m_SNType.MagicNum,strMagic,strMagic.GetLength());
	m_SNInfo.SNBlock = SNBLOCK;

	if (AddOption && pCfg->SNLen == 10){
		GroupCnt = 2;///ABOV����ID��Ϊ����
	}

	UpdateData(TRUE);
	srand((int)time(NULL));
	UINT MAX = 1000;
	UINT MIN = 9999;
	UINT result;
	result = MAX + rand() % (MIN - MAX + 1);
	
	pCfg->TestMode = GetPrivateProfileInt("Config", "TestMode ",0, strIniFile);
	DataMode = GetPrivateProfileInt("Config", "DataMode",0, strIniFile);
	if (pCfg->SNLen == 10){
		if (pCfg->TestMode){
			testID.Format("23456%d",result);
			m_SNInfo.SNData = testID;
			strMsg.Format("AdapIdx:%d  GetRmpIdList:%s",AdapIdx+1,testID);
			m_FileLog.PrintLog(LOGLEVEL_ERR, strMsg);
			if (!DataMode){
				ComTool::Str2Hex(m_SNInfo.SNData,ComTool::ENDIAN_BIG,vMac);
			}else{
				ComTool::Str2Hex(m_SNInfo.SNData,ComTool::ENDIAN_LIT,vMac);
			}
			
			std::copy(vMac.begin(),vMac.end(),m_Mac);
			Lenth = (UINT)(vMac.size()+8+1+2+5+1+4+4);
			Sleep(1000);
		} 
		else{
			if (DllHelp.GetPmrIdInit()){
				char * lpstrReturn = DllHelp.InitLocalDB();
				CString m_str1 = lpstrReturn;
				strMsg.Format("InitLocalDB: %s",m_str1);
				m_FileLog.PrintLog(LOGLEVEL_ERR, strMsg);

				char * strReturn = DllHelp.GetRmpIdList(1);
				if (strReturn == NULL){
					MessageBox("��ȡID��ʧ��");
					Ret = -1;goto __end;
				}
				CString m_str3;
				m_str3.Format("%s",strReturn);
				CString strID = m_str3.Left(m_str3.Find(","));

				if(strID.GetLength() != 9){
					MessageBox("��ȡ�����ݲ�����");
					Ret=-1; goto __end;
					
				}else{
					strID.Format("0%s",strID);
				}
			
				strMsg.Format("GetRmpIdList:%s",strID);
				m_FileLog.PrintLog(LOGLEVEL_ERR, strMsg);
				strGetSn = strID;
				m_SNInfo.SNData = strGetSn;
				if (!DataMode){
					ComTool::Str2Hex(m_SNInfo.SNData,ComTool::ENDIAN_BIG,vMac);
				}else{
					ComTool::Str2Hex(m_SNInfo.SNData,ComTool::ENDIAN_LIT,vMac);
				}
				std::copy(vMac.begin(),vMac.end(),m_Mac);
				Lenth = (UINT)(vMac.size()+8+1+2+5+1+4+4);
			}
		else{
			::MessageBox(NULL, "Load Fail", "Load Fail!", MB_OK);
		}
		}
			
	}else {
		if (pCfg->TestMode){
			result = MAX + rand() % (MIN - MAX + 1);
			testID.Format("0000FB5115706677E39271100001%d",result);
			m_SNInfo.SNData = testID;
			strMsg.Format("AdapIdx:%d  MessageResult:%s",AdapIdx+1,testID);
			Lenth = (m_SNInfo.SNData.GetLength()+8+1+2+5+1+4+4);
			m_FileLog.PrintLog(LOGLEVEL_ERR, strMsg);
			Sleep(1000);
		} 
		else{
			CoInitialize(NULL);
			Midea_MES_SN_Util::ISNReaderPtr ptra;
			
			ptra.CreateInstance(Midea_MES_SN_Util::CLSID_SNReader);

			try{

				Midea_MES_SN_Util::MessageResult mr;
				//Result = ptra->Test_FetchNextSN(0,&mr);
				Result = ptra->FetchNextSN(&mr);
				if (Result!=0){
					Ret=-1; goto __end;
				}
				strGetSn = mr.SN;
				if (strGetSn.GetLength()!=32){
					strMsg.Format("SN:%s,��ȡ�����ݲ�����",strGetSn);
					MessageBox(strMsg);
					Ret=-1; goto __end;
				}
				m_SNInfo.SNData = strGetSn;
				Lenth = (m_SNInfo.SNData.GetLength()+8+1+2+5+1+4+4);
				strMsg.Format("AdapIdx:%d  MessageResult:%s",AdapIdx+1,strGetSn);
				m_FileLog.PrintLog(LOGLEVEL_ERR, strMsg);

			}
			catch(...){
				MessageBox("��ȡSN��ʧ��");
			Ret=-1; goto __end;
			}
		}
	}

	m_SNType.SnInfo = &m_SNInfo;
	if (!m_strTagInfo.IsEmpty()){//�����ļ�����TagInfo���ߵڶ��׷���
		lSerial<<GroupCnt;
		lSerial<<pCfg->BufAddr;
		lSerial<<DataLenth;
		lSerial.SerialInBuff((BYTE*)m_SNType.MagicNum,8);
		lSerial<<m_SNType.Version;
		lSerial<<m_SNType.SNInfoCnt;
		lSerial.SerialInBuff((BYTE*)HeaderReserve,5);
		lSerial<<m_SNInfo.SNBlock;
		lSerial<<m_SNInfo.SNAddr;
		lSerial<<m_SNInfo.SNLen;
		if (pCfg->SNLen == 10){
			lSerial.SerialInBuff((BYTE*)m_Mac,vMac.size());
		}else{
			if (DataMode){
				m_SNInfo.SNData.MakeReverse();
			}
			if(Changeover){
				ChangeData(m_SNInfo.SNData);
			}
			
			lSerial.SerialInBuff((BYTE*)m_SNInfo.SNData.GetBuffer(),m_SNInfo.SNData.GetLength());
		}
		UINT* InfoReserve = new UINT[DataLenth-Lenth];
		memset(InfoReserve,0,DataLenth-Lenth);

		lSerial.SerialInBuff((BYTE*)InfoReserve,DataLenth-Lenth);
		m_SNInfo.SNData.ReleaseBuffer();
		delete[] InfoReserve;
	} 
	else{
		lSerial<<GroupCnt;
		lSerial<<pCfg->SNAddr;
		if (pCfg->SNLen == 10){
			lSerial<<vMac.size();
			lSerial.SerialInBuff((BYTE*)m_Mac,vMac.size());
		}else{
			lSerial<<m_SNInfo.SNData.GetLength();
			if (DataMode){
				m_SNInfo.SNData.MakeReverse();
			}
			if(Changeover){
				ChangeData(m_SNInfo.SNData);
			}
			lSerial.SerialInBuff((BYTE*)m_SNInfo.SNData.GetBuffer(),m_SNInfo.SNData.GetLength());
		}
		m_SNInfo.SNData.ReleaseBuffer();
		if (AddOption){
			if (pCfg->SNLen == 10){
				lSerial<<pCfg->OptionAddr;
				lSerial<<vMac.size();
				lSerial.SerialInBuff((BYTE*)m_Mac,vMac.size());
			}
		}
	}
	

	if(*pSize<(INT)lSerial.GetLength()){///����ط�Ҫע�ⷵ��ʵ��ʹ�õ��ֽ�����ʵ�ʿ������Ĵ�С
		*pSize=lSerial.GetLength();
		MessageBox("pSize Failure");
		Ret=-2; goto __end;
	}

	Ret=lSerial.GetLength();///����ֵ��Ҫ�����������ֽ���
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
	return Ret;

}


INT CDlgSNCustom::CreatLog()
{
	CString strLogPath;
	CTime Time;
	CString strCurPath= CComFunc::GetCurrentPath();
	CString strDate;
	CString strtemppath;
	Time=CTime::GetCurrentTime();
	strDate = Time.Format("%Y-%m-%d");
	strtemppath.Format("%s\\sngen\\midea\\%s",strCurPath,strDate);
	if(!PathIsDirectory(strtemppath)){
		CreateDirectory(strtemppath,0);
	}
	strLogPath.Format("%s\\%d%02d%02d_%02d%02d%02d.txt", strtemppath,Time.GetYear(),Time.GetMonth(),Time.GetDay(),
		Time.GetHour(),Time.GetMinute(),Time.GetSecond());

	m_FileLog.SetLogFile(strLogPath);
	return 0;
}

void CDlgSNCustom::LoadInI()
{
	CHAR TmpBuf[512];
	CString strTemp;
	CString strIniFile;
	memset(TmpBuf,0,512);
	strIniFile.Format("%s\\sngen\\SCS_SN_MIDEA.ini", CComFunc::GetCurrentPath());
	if (!PathFileExists(strIniFile)){
		AfxMessageBox("�Ҳ���SCS_SN_MIDEA.ini�����ļ�������!");
		return ;
	}

	memset(TmpBuf,0,512);
	GetPrivateProfileString("Config", "Products", "",TmpBuf, MAX_PATH, strIniFile);
	m_tCfgInI.ProductsName.Format("%s",TmpBuf);

	memset(TmpBuf,0,512);
	GetPrivateProfileString("Config", "SNAddr", "",TmpBuf, MAX_PATH, strIniFile);
	strTemp.Format("%s", TmpBuf);
	sscanf(strTemp, "%I64X", &m_tCfgInI.SNAddr);

	m_tCfgInI.SNLen = GetPrivateProfileInt("Config", "SNlen ",32, strIniFile);

	tSNCustomCfg* pCfg=m_SnCfg.GetCustomCfg();
	*pCfg  = m_tCfgInI;

}

void CDlgSNCustom::InitOnce()
{	
	CreatLog();
	CString strMsg;
	strMsg.Format("����DLLʧ��");

	if (m_DllHelpApi.m_bLoadDll){
		strMsg.Format("����DLL�ɹ�");
	}
	m_FileLog.PrintLog(LOGLEVEL_ERR, strMsg);
	m_DllHelpApi.AttachLog(&m_FileLog);

}
void CDlgSNCustom::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgSNCustom, CDialog)
	
	ON_CBN_SELCHANGE(IDC_COMBO_PRODUCTS, &CDlgSNCustom::OnCbnSelchangeComboProducts)
	ON_EN_KILLFOCUS(IDC_EDIT_SN_ADDR, &CDlgSNCustom::OnEnKillfocusEditSnAddr)
	ON_BN_CLICKED(IDC_CHECK_MODE, &CDlgSNCustom::OnBnClickedCheckMode)
END_MESSAGE_MAP()


// CDlgSNCustom ��Ϣ�������

BOOL CDlgSNCustom::OnInitDialog()
{
	CDialog::OnInitDialog();
	

	// TODO:  �ڴ���Ӷ���ĳ�ʼ��
	ISMessage = TRUE;
	InitOnce();
	
	return FALSE;
	//return TRUE;  // return TRUE unless you set the focus to a control
	// �쳣: OCX ����ҳӦ���� FALSE
}


void CDlgSNCustom::OnCbnSelchangeComboProducts()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	GetComboValue(FALSE);

}

void CDlgSNCustom::OnEnKillfocusEditSnAddr()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	SaveCtrls();
}

void CDlgSNCustom::OnBnClickedCheckMode()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	SaveCtrls();
}

BOOL CDlgSNCustom::ParseTagInfo()
{
	BOOL Ret=FALSE;
	tSNCustomCfg* pCfg=m_SnCfg.GetCustomCfg();
		
	INT Array_size;
	CString strmsg;
	CString strJsonName;
	
	cJSON *pRootParser = NULL;
	cJSON *RootSNInfos = NULL;
	cJSON *JsonHeader = NULL;
	cJSON *JsonValue = NULL;


	//���m_strTagInfoΪ��,���߽ṹ������
	if (m_strTagInfo.IsEmpty()){
		if (m_strManuName.CompareNoCase("BYD")==0 ||m_strManuName.CompareNoCase("SinoWealth")==0)
		{
			MessageBox("��ӱ���ǵ�оƬ��Ҫ�����¹��̣����ߴ˿�оƬ�ݲ�֧��SN��¼");
			goto __end;
		}
		Ret = TRUE;
		goto __end;
	}
	
	pRootParser = cJSON_Parse(m_strTagInfo.GetBuffer());
	if (pRootParser == NULL) {
		MessageBox("������Json���ݸ�ʽ");
		goto __end;
	}
	
	RootSNInfos = cJSON_GetObjectItem(pRootParser, "BufferRemap");
	Array_size = cJSON_GetArraySize(RootSNInfos);
	for (int i = 0; i < Array_size; i++)
	{
		JsonHeader = cJSON_GetArrayItem(RootSNInfos, i);
	
		JsonValue = cJSON_GetObjectItem(JsonHeader, "BufferType");
		if (JsonValue == NULL) {
			MessageBox("����BufferType�ֶδ���");
			goto __end;
		}
		m_BufferType = JsonValue->valueint;
		if (m_BufferType != BUFFER_TYPE_SN_REMAP){//ֻ����SNFileInfo
			continue;
		}else{
			JsonValue = cJSON_GetObjectItem(JsonHeader, "BufferAddrStart");
			if (JsonValue == NULL) {
				MessageBox("����BufferAddrStart�ֶδ���");
				goto __end;
			}
			m_strBufAddr = JsonValue->valuestring;
	
			JsonValue = cJSON_GetObjectItem(JsonHeader, "ChipName");
			if (JsonValue == NULL) {
				MessageBox("����ChipName�ֶδ���");
				goto __end;
			}
	
			m_strChipName = JsonValue->valuestring;
		}
	}
	Ret = TRUE;
__end:
	if (pRootParser != NULL) {
		cJSON_Delete(pRootParser);
		pRootParser = NULL;
	}
	return Ret;

}

INT CDlgSNCustom::SetTagInfo(char*pData,char* pChipName,char* pManuName)
{
	m_strTagInfo.Format("%s",pData);
	m_strAprChipName.Format("%s",pChipName);
	m_strManuName.Format("%s",pManuName);
	return 0;
}

INT CDlgSNCustom::ChangeData(CString& strData)
{
	INT index =4;
	CString strTemp,strNewData;
	while(strData.GetLength()>0){
		strTemp = strData.Left(index);
		strTemp = strTemp.MakeReverse();
		strNewData +=strTemp;
		strData.Delete(0, index);
	}
	strData.Format("%s",strNewData);
	return 0;
}



