// DlgSNCustom.cpp : 实现文件
//

#include "stdafx.h"
#include "SCS_SN_Gomora_001.h"
#include "DlgSNCustom.h"


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
	pCfg->DeviceIDAddr=0x1FF100;
	pCfg->DeviceIDLen=17;
	pCfg->SecretAddr=0x1FF000;
	pCfg->SecretLen=64;
	pCfg->strCSVFile="";
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

	strText.Format("%I64X",pCfg->SecretAddr);
	GetDlgItem(IDC_SECRET_ADDR)->SetWindowText(strText);
	strText.Format("%d",pCfg->SecretLen);
	GetDlgItem(IDC_SECRET_LEN)->SetWindowText(strText);

	m_strCSVFile=pCfg->strCSVFile;
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
			m_CSVFile.CloseFile();
			if(m_CSVFile.OpenFile(pCfg->strCSVFile)!=0){
				strErrMsg=m_CSVFile.GetErrMsg();
				MessageBox(strErrMsg,"",MB_OK);
			}
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
		MessageBox("获取DeviceID起始地址错误,请确认");
		return FALSE;
	}

	GetDlgItem(IDC_DEVICEID_LEN)->GetWindowText(strText);
	if(sscanf(strText,"%d",&pCfg->DeviceIDLen)!=1){
		MessageBox("获取DeviceID字节长度错误,请确认");
		return FALSE;
	}

	GetDlgItem(IDC_SECRET_ADDR)->GetWindowText(strText);
	if(sscanf(strText,"%I64X",&pCfg->SecretAddr)!=1){
		MessageBox("获取Secret起始地址错误,请确认");
		return FALSE;
	}

	GetDlgItem(IDC_SECRET_LEN)->GetWindowText(strText);
	if(sscanf(strText,"%d",&pCfg->SecretLen)!=1){
		MessageBox("获取Secret字节长度错误,请确认");
		return FALSE;
	}

	pCfg->strCSVFile=m_strCSVFile;
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

INT CDlgSNCustom::QuerySN(UINT AdapIdx,DWORD Idx,BYTE*pData,INT*pSize)
{
	INT Ret=0;
	CString Curtime;
	CTime Time;
	CSerial lSerial;
	CString strFstCol,strSecCol;
	UINT GroupCnt=2;
	tSNCustomCfg* pCfg=m_SnCfg.GetCustomCfg();
	if(m_CSVFile.IsReady()==FALSE){
		MessageBox("Please check the csv file is opened");
		Ret=-1; goto __end;
	}

	Ret=m_CSVFile.ReadSN(Idx,strFstCol,strSecCol);
	if(Ret!=0){
		Ret=-1; goto __end;
	}

	if(strFstCol.GetLength()!=pCfg->DeviceIDLen || strSecCol.GetLength()!=pCfg->SecretLen){
		Ret=-1; goto __end;
	}
	
	lSerial<<GroupCnt;
	lSerial<<pCfg->DeviceIDAddr;
	lSerial<<pCfg->DeviceIDLen;
	lSerial.SerialInBuff((BYTE*)strFstCol.GetBuffer(),strFstCol.GetLength());
	lSerial<<pCfg->SecretAddr;
	lSerial<<pCfg->SecretLen;
	lSerial.SerialInBuff((BYTE*)strSecCol.GetBuffer(),strSecCol.GetLength());

	if(*pSize<lSerial.GetLength()){///这个地方要注意返回实际使用的字节数与实际可以填充的大小
		*pSize=lSerial.GetLength();
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

void CDlgSNCustom::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	/*DDX_Text(pDX, IDC_DEVICEID_ADDR, m_DeviceAddr);
	DDX_Text(pDX, IDC_DEVICEID_LEN, m_DeviceIDLen);
	DDX_Text(pDX, IDC_SECRET_ADDR, m_SecretAddr);
	DDX_Text(pDX, IDC_SECRET_LEN, m_SecretLen);*/
	DDX_Text(pDX, IDC_CSVFILE, m_strCSVFile);
}


BEGIN_MESSAGE_MAP(CDlgSNCustom, CDialog)
	ON_BN_CLICKED(IDC_BTNSELCSV, &CDlgSNCustom::OnBnClickedBtnselcsv)
	ON_EN_KILLFOCUS(IDC_DEVICEID_ADDR, &CDlgSNCustom::OnEnKillfocusEdit)
	ON_EN_KILLFOCUS(IDC_DEVICEID_LEN, &CDlgSNCustom::OnEnKillfocusEdit)
	ON_EN_KILLFOCUS(IDC_SECRET_ADDR, &CDlgSNCustom::OnEnKillfocusEdit)
	ON_EN_KILLFOCUS(IDC_SECRET_LEN, &CDlgSNCustom::OnEnKillfocusEdit)
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
	SaveCtrls();
}
