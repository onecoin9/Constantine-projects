// DlgSNCustom.cpp : 实现文件
//

#include "stdafx.h"
#include "SCS_SN_HASEAEE_001.h"
#include "DlgSNCustom.h"
#include <stdio.h>

#include<stdlib.h>
#if _MSC_VER
#define snprintf _snprintf
#endif

// CDlgSNCustom 对话框

IMPLEMENT_DYNAMIC(CDlgSNCustom, CDialog)

CDlgSNCustom::CDlgSNCustom(DRVSNCFGPARA *pSnCfgPara,CWnd* pParent /*=NULL*/)
	: CDialog(CDlgSNCustom::IDD, pParent)
	, m_pSnCfgPara(pSnCfgPara)
	, m_SNStartAddr(0)
	, m_DeviceAddr(0)
	, m_DeviceIDLen(0)
	, m_SecretAddr(0)
	, m_SecretLen(0)
	, m_strCSVFile(_T(""))
{
	tSNCustomCfg* pCfg=m_SnCfg.GetCustomCfg();
	pCfg->DeviceIDAddr=0x00100278;
	pCfg->DeviceIDLen=12;
}

CDlgSNCustom::~CDlgSNCustom()
{
}

BOOL CDlgSNCustom::InitCtrlsValue(CSNCustomCfg& SNCfg)
{
	tSNCustomCfg* pCfg=SNCfg.GetCustomCfg();
	CString strText;
	strText.Format("%I64X",pCfg->DeviceIDAddr);
	GetDlgItem(IDC_DEVICEID_ADDR)->SetWindowText(strText);
	strText.Format("%d",pCfg->DeviceIDLen);
	GetDlgItem(IDC_DEVICEID_LEN)->SetWindowText(strText);

	//strText.Format("%I64X",pCfg->SecretAddr);
	//GetDlgItem(IDC_SECRET_ADDR)->SetWindowText(strText);
	//strText.Format("%d",pCfg->SecretLen);
	//GetDlgItem(IDC_SECRET_LEN)->SetWindowText(strText);

	//m_strCSVFile=pCfg->strCSVFile;
	UpdateData(FALSE);
	return TRUE;
}

BOOL CDlgSNCustom::InitCtrls(CSerial& lSerial)
{
	BOOL Ret=TRUE;
	CString strErrMsg;
	if(lSerial.GetLength()!=0){///再次打开对话框的时候就会传入，但之前会因为资源被释放，所以需要重新打开文件
		Ret=m_SnCfg.SerialInCfgData(lSerial);
		if(Ret==TRUE){
			tSNCustomCfg* pCfg=m_SnCfg.GetCustomCfg();
			/*m_CSVFile.CloseFile();
			if(m_CSVFile.OpenFile(pCfg->strCSVFile)!=0){
				strErrMsg=m_CSVFile.GetErrMsg();
				MessageBox(strErrMsg,"",MB_OK);
			}*/
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
	GetDlgItem(IDC_DEVICEID_ADDR)->GetWindowText(strText);
	if(sscanf(strText,"%I64X",&pCfg->DeviceIDAddr)!=1){
		MessageBox("获取SN写入起始地址错误,请确认");
		return FALSE;
	}

	GetDlgItem(IDC_DEVICEID_LEN)->GetWindowText(strText);
	if(sscanf(strText,"%d",&pCfg->DeviceIDLen)!=1){
		MessageBox("获取字节长度错误,请确认");
		return FALSE;
	}

	/*GetDlgItem(IDC_SECRET_ADDR)->GetWindowText(strText);
	if(sscanf(strText,"%I64X",&pCfg->SecretAddr)!=1){
		MessageBox("获取Secret起始地址错误,请确认");
		return FALSE;
	}

	GetDlgItem(IDC_SECRET_LEN)->GetWindowText(strText);
	if(sscanf(strText,"%d",&pCfg->SecretLen)!=1){
		MessageBox("获取Secret字节长度错误,请确认");
		return FALSE;
	}

	pCfg->strCSVFile=m_strCSVFile;*/
	return TRUE;
}

BOOL CDlgSNCustom::GetCtrls(CSerial&lSerial)
{
	BOOL Ret=TRUE;
	Ret=SaveCtrls();
	if(Ret!=TRUE){
		return Ret;
	}
	return m_SnCfg.SerialOutCfgData(lSerial);
}

CString CDlgSNCustom::GetCurrentPath( void )
{
	TCHAR szFilePath[MAX_PATH + 1]; 
	TCHAR *pPos=NULL;
	CString str_url;
	GetModuleFileName(NULL, szFilePath, MAX_PATH); 
	pPos=_tcsrchr(szFilePath, _T('\\'));
	if(pPos!=NULL){
		pPos[0] = 0;//脡戮鲁媒脦脛录镁脙没拢卢脰禄禄帽碌脙脗路戮露
		str_url=CString(szFilePath);
	}
	else{
		pPos=_tcsrchr(szFilePath, _T('/'));
		if(pPos==NULL){
			str_url="";
		}
		else{
			str_url=CString(szFilePath);
		}
	}	
	return str_url;
}

INT CDlgSNCustom::QuerySN(UINT AdapIdx,DWORD Idx,BYTE*pData,INT*pSize)
{
	INT Ret=0;
	CString Curtime;
	CTime Time;
	CSerial lSerial;
	CString strFstCol,strSecCol;
	UINT GroupCnt=1;
	tSNCustomCfg* pCfg=m_SnCfg.GetCustomCfg();
	
	char data[12] = {0};
	data[11] =0x20;

	SYSTEMTIME st;
	GetLocalTime(&st);
	UINT num = 1;
	
	CHAR TmpBuf[512];
	CString strIniFile;
	memset(TmpBuf,0,512);

	CString strYear;
	strYear.Format("%04d",  st.wYear);
	strYear.Delete(0, 2);

	int nYear = 0;
	sscanf_s((LPSTR)(LPCSTR)strYear, "%d", &nYear);

	CString strCache;
	strCache.Format("%04d%02d%02d", st.wYear, st.wMonth, st.wDay);

	CString strRoamingPath = _T("");
	
	CString strTempPath;
	
	SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0,strTempPath.GetBuffer(MAX_PATH) /*(LPSTR)(LPCTSTR)strRoamingPath*/);
	strTempPath.ReleaseBuffer();
	strRoamingPath.Format("%s",strTempPath );

	strIniFile.Format("%s\\SCS_HASEAEE_CACHE.ini", strRoamingPath/*GetCurrentPath()*/);

	char cBuff[16] = {0};
	if (PathFileExists(strIniFile) != TRUE){
		if(WritePrivateProfileString("Extend", "Index", itoa(num,cBuff, 10), strIniFile) ==FALSE){
			DeleteFile(strIniFile);
			Ret=-2;goto __end;
		}

		if(WritePrivateProfileString("Extend", "CurrData", strCache, strIniFile) ==FALSE){
			DeleteFile(strIniFile);
			Ret=-2;goto __end;
		}
	}
	else
	{
		CString strCurrData;
		char tempArr[MAX_PATH] = {0};
		memset(tempArr,0,MAX_PATH);
		GetPrivateProfileString("Extend", "CurrData","",tempArr, MAX_PATH, strIniFile);
		if (tempArr[0] == 0)
		{
			Ret=-2;goto __end;
		}

		strCurrData.Format("%s", tempArr);

		if (strCurrData.CompareNoCase(strCache) == 0)
		{
			num = GetPrivateProfileInt("Extend", "Index",1, strIniFile);
			num++;	
		}else{//时间不同重置
			if(WritePrivateProfileString("Extend", "CurrData", strCache, strIniFile) ==FALSE){
				Ret=-2;goto __end;
			}

			num = 1;
			if(WritePrivateProfileString("Extend", "Index", itoa(num,cBuff, 10), strIniFile) ==FALSE){
				Ret=-2;goto __end;
			}
		}
		
		if (num >99999)
		{
			Ret=-2;goto __end;
		}

		if(WritePrivateProfileString("Extend", "Index", itoa(num,cBuff, 10), strIniFile) ==FALSE){
			Ret=-2;goto __end;
		}
	}
	
	 snprintf(data, 12, "%02d%02d%02d%05d",nYear, st.wMonth, st.wDay, num);
	data[11] =0x20;

	lSerial<<GroupCnt;
	lSerial<<pCfg->DeviceIDAddr;
	lSerial<<pCfg->DeviceIDLen;
	lSerial.SerialInBuff((BYTE*)data, 12);

	/*if(*pSize<lSerial.GetLength()){///这个地方要注意返回实际使用的字节数与实际可以填充的大小
		*pSize=lSerial.GetLength();
		Ret=-2; goto __end;
	}*/
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

void CDlgSNCustom::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	/*DDX_Text(pDX, IDC_DEVICEID_ADDR, m_DeviceAddr);
	DDX_Text(pDX, IDC_DEVICEID_LEN, m_DeviceIDLen);
	DDX_Text(pDX, IDC_SECRET_ADDR, m_SecretAddr);
	DDX_Text(pDX, IDC_SECRET_LEN, m_SecretLen);*/
	/*DDX_Text(pDX, IDC_CSVFILE, m_strCSVFile);*/
}


BEGIN_MESSAGE_MAP(CDlgSNCustom, CDialog)
	ON_BN_CLICKED(IDC_BTNSELCSV, &CDlgSNCustom::OnBnClickedBtnselcsv)
	ON_EN_KILLFOCUS(IDC_DEVICEID_ADDR, &CDlgSNCustom::OnEnKillfocusEdit)
	ON_EN_KILLFOCUS(IDC_DEVICEID_LEN, &CDlgSNCustom::OnEnKillfocusEdit)
END_MESSAGE_MAP()


// CDlgSNCustom 消息处理程序

BOOL CDlgSNCustom::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化
	return FALSE;
	//return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CDlgSNCustom::OnBnClickedBtnselcsv()
{
	// TODO: 在此添加控件通知处理程序代码
	CString strFilePath;
	CString strErrMsg;
	CFileDialog dlgFile(TRUE, NULL, NULL, OFN_OVERWRITEPROMPT, "CSV File(*.csv)|*.csv||");
	if (dlgFile.DoModal() != IDOK){
		return;
	}
	m_strCSVFile=dlgFile.GetPathName();
	
	m_CSVFile.CloseFile();
	if(m_CSVFile.OpenFile(m_strCSVFile)!=0){
		strErrMsg=m_CSVFile.GetErrMsg();
		MessageBox(strErrMsg,"",MB_OK);
	}
	UpdateData(FALSE);
}

void CDlgSNCustom::OnEnKillfocusEdit()
{
	// TODO: 在此添加控件通知处理程序代码
	//SaveCtrls();
}
