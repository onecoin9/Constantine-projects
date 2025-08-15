// DlgSNCustom.cpp : 实现文件
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

// CDlgSNCustom 对话框

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

	if (!ParseTagInfo()){//解析Tag失败
		goto __end;
	}
	
	if (File.Open(strJsonPath, CFile::modeRead | CFile::shareDenyNone) == FALSE) {
		MessageBox("打开Json文件失败,请确认文件路径!");
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
		MessageBox("不符合Json数据格式");
		goto __end;
	}

	RootSNInfos = cJSON_GetObjectItem(pRootParser, "SNInfos");
	Array_size = cJSON_GetArraySize(RootSNInfos);
	for (int i = 0; i < Array_size; i++)
	{
		JsonHeader = cJSON_GetArrayItem(RootSNInfos, i);

		JsonValue = cJSON_GetObjectItem(JsonHeader, "ProductName");
		if (JsonValue == NULL) {
			MessageBox("解析ProductName字段错误");
			goto __end;
		}
		pCfg->ProductsName = JsonValue->valuestring;

		////芯片名判断
		ChipNameLen = pCfg->ProductsName.GetLength()-3;
		strProductsName = pCfg->ProductsName.Left(ChipNameLen);
		strChipName = m_strAprChipName;
		if (m_strAprChipName.IsEmpty()){//如果没有加载工程，下拉框加载所有内容
			if(IsInit){
				((CComboBox *)GetDlgItem(IDC_COMBO_PRODUCTS))->AddString(pCfg->ProductsName);	
			}
		}else{
			if (strChipName.Find(strProductsName) < 0 ){				continue;			}
			/*if (strProductsName.CompareNoCase(strChipName)!=0 ){				continue;			}*/
			if(IsInit){//下拉框只显示tag中芯片信息
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
		
		((CComboBox *)GetDlgItem(IDC_COMBO_PRODUCTS))->GetWindowText(m_strProducts);//界面下拉框中的芯片名

		if (m_strProducts.CompareNoCase(pCfg->ProductsName) == 0) {

			JsonValue = cJSON_GetObjectItem(JsonHeader,"SNAddr");
			if (JsonValue == NULL) {
				MessageBox("解析SNAddr字段错误");
				goto __end;
			}
			pCfg->SNAddr = atoi(JsonValue->valuestring);
			m_strSNAddr.Format("%s",JsonValue->valuestring);
			GetDlgItem(IDC_EDIT_SN_ADDR)->SetWindowText(m_strSNAddr);
			sscanf(m_strSNAddr, "%I64X", &m_SNInfo.SNAddr);

			JsonValue = cJSON_GetObjectItem(JsonHeader,"Size");
			if (JsonValue == NULL) {
				MessageBox("解析Size字段错误");
				goto __end;
			}
			pCfg->SNLen = (UINT)JsonValue->valueint;
			m_strSNLen.Format("%d",JsonValue->valueint);
			GetDlgItem(IDC_EDIT_SN_LEN)->SetWindowText(m_strSNLen);
			m_SNInfo.SNLen = JsonValue->valueint;

			/*JsonValue = cJSON_GetObjectItem(JsonHeader,"SNBlock");
			if (JsonValue == NULL) {
				MessageBox("解析SNBlock字段错误");
				goto __end;
			}
			m_SNInfo.SNBlock = JsonValue->valueint;*/

			//JsonValue = cJSON_GetObjectItem(JsonHeader,"BufAddr");
			//if (JsonValue == NULL) {
			//	MessageBox("解析BufAddr字段错误");
			//	goto __end;
			//}
			//m_strSNAddr.Format("%s",JsonValue->valuestring);
			//sscanf(m_strSNAddr, "%I64X", &pCfg->BufAddr);
			//if (!m_strTagInfo.IsEmpty() && m_strSNAddr.CompareNoCase(m_strBufAddr)!=0){//如果json里面的buf地址与TAG不同
			//	MessageBox("json文件中buffer地址与驱动不同");
			//	goto __end;
			//}

			sscanf(m_strBufAddr, "%I64X", &pCfg->BufAddr);

			JsonValue = cJSON_GetObjectItem(JsonHeader,"OptionAddr");
			if (JsonValue != NULL  && m_strManuName.CompareNoCase("ABOV")==0 && pCfg->SNLen == 10){//ABOV芯片烧录ID码需要多烧一个Option区域
				AddOption = TRUE;
				sscanf(JsonValue->valuestring, "%I64X", &pCfg->OptionAddr);
				((CButton *)GetDlgItem(IDC_CHECK_MODE))->SetCheck(1);
			}
			if (m_strManuName.CompareNoCase("ABOV")==0 && pCfg->SNLen == 32){
				((CButton *)GetDlgItem(IDC_CHECK_MODE))->SetCheck(0);
			}
			if (m_strManuName.CompareNoCase("ABOV")==0 && JsonValue == NULL && pCfg->SNLen == 10){
				MessageBox("ABOV芯片烧录10位ID码请在json中添加Option地址");
				goto __end;
			}

			JsonValue = cJSON_GetObjectItem(JsonHeader,"Changeover");
			if (JsonValue != NULL && pCfg->SNLen == 32){//9062SN码需要反转顺序
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
	if(lSerial.GetLength()!=0){///再次打开对话框的时候就会传入，但之前会因为资源被释放，所以需要重新打开文件
		Ret=m_SnCfg.SerialInCfgData(lSerial);
		if(Ret==TRUE){
			tSNCustomCfg* pCfg=m_SnCfg.GetCustomCfg();
		}
	}
	else{///首次加载的时候lSerial会没有值
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
		MessageBox("中颖比亚迪芯片烧录区域为Information Block，且为大端烧录，请确认");
		ISMessage= FALSE;
	}
	INT DataMode = ((CButton *)GetDlgItem(IDC_CHECK_MODE))->GetCheck();
	if(ISMessage){
		if (!DataMode){
			strmsg.Format("烧录前请确认数据存放方式是否正确,当前为大端模式");
			MessageBox(strmsg);
			ISMessage= FALSE;
		}else{
			strmsg.Format("烧录前请确认数据存放方式是否正确,当前为小端模式");
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
	UINT GroupCnt=1;///固定为一组
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
		GroupCnt = 2;///ABOV且是ID码为两组
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
					MessageBox("获取ID码失败");
					Ret = -1;goto __end;
				}
				CString m_str3;
				m_str3.Format("%s",strReturn);
				CString strID = m_str3.Left(m_str3.Find(","));

				if(strID.GetLength() != 9){
					MessageBox("获取的数据不符合");
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
					strMsg.Format("SN:%s,获取的数据不符合",strGetSn);
					MessageBox(strMsg);
					Ret=-1; goto __end;
				}
				m_SNInfo.SNData = strGetSn;
				Lenth = (m_SNInfo.SNData.GetLength()+8+1+2+5+1+4+4);
				strMsg.Format("AdapIdx:%d  MessageResult:%s",AdapIdx+1,strGetSn);
				m_FileLog.PrintLog(LOGLEVEL_ERR, strMsg);

			}
			catch(...){
				MessageBox("获取SN码失败");
			Ret=-1; goto __end;
			}
		}
	}

	m_SNType.SnInfo = &m_SNInfo;
	if (!m_strTagInfo.IsEmpty()){//工程文件中有TagInfo，走第二套方案
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
	

	if(*pSize<(INT)lSerial.GetLength()){///这个地方要注意返回实际使用的字节数与实际可以填充的大小
		*pSize=lSerial.GetLength();
		MessageBox("pSize Failure");
		Ret=-2; goto __end;
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
		AfxMessageBox("找不到SCS_SN_MIDEA.ini配置文件，请检查!");
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
	strMsg.Format("加载DLL失败");

	if (m_DllHelpApi.m_bLoadDll){
		strMsg.Format("加载DLL成功");
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


// CDlgSNCustom 消息处理程序

BOOL CDlgSNCustom::OnInitDialog()
{
	CDialog::OnInitDialog();
	

	// TODO:  在此添加额外的初始化
	ISMessage = TRUE;
	InitOnce();
	
	return FALSE;
	//return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}


void CDlgSNCustom::OnCbnSelchangeComboProducts()
{
	// TODO: 在此添加控件通知处理程序代码
	GetComboValue(FALSE);

}

void CDlgSNCustom::OnEnKillfocusEditSnAddr()
{
	// TODO: 在此添加控件通知处理程序代码
	SaveCtrls();
}

void CDlgSNCustom::OnBnClickedCheckMode()
{
	// TODO: 在此添加控件通知处理程序代码
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


	//如果m_strTagInfo为空,则不走结构化方案
	if (m_strTagInfo.IsEmpty()){
		if (m_strManuName.CompareNoCase("BYD")==0 ||m_strManuName.CompareNoCase("SinoWealth")==0)
		{
			MessageBox("中颖比亚迪芯片需要创建新工程，或者此款芯片暂不支持SN烧录");
			goto __end;
		}
		Ret = TRUE;
		goto __end;
	}
	
	pRootParser = cJSON_Parse(m_strTagInfo.GetBuffer());
	if (pRootParser == NULL) {
		MessageBox("不符合Json数据格式");
		goto __end;
	}
	
	RootSNInfos = cJSON_GetObjectItem(pRootParser, "BufferRemap");
	Array_size = cJSON_GetArraySize(RootSNInfos);
	for (int i = 0; i < Array_size; i++)
	{
		JsonHeader = cJSON_GetArrayItem(RootSNInfos, i);
	
		JsonValue = cJSON_GetObjectItem(JsonHeader, "BufferType");
		if (JsonValue == NULL) {
			MessageBox("解析BufferType字段错误");
			goto __end;
		}
		m_BufferType = JsonValue->valueint;
		if (m_BufferType != BUFFER_TYPE_SN_REMAP){//只解析SNFileInfo
			continue;
		}else{
			JsonValue = cJSON_GetObjectItem(JsonHeader, "BufferAddrStart");
			if (JsonValue == NULL) {
				MessageBox("解析BufferAddrStart字段错误");
				goto __end;
			}
			m_strBufAddr = JsonValue->valuestring;
	
			JsonValue = cJSON_GetObjectItem(JsonHeader, "ChipName");
			if (JsonValue == NULL) {
				MessageBox("解析ChipName字段错误");
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



