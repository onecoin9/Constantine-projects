// DlgSNCustom.cpp : 实现文件
//

#include "stdafx.h"
#include "SCS_SN_ESP32.h"
#include "DlgSNCustom.h"
#include "../Com/ComFunc.h"

// CDlgSNCustom 对话框

#define DEFAULT_ADDR (0xF0000000)

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
	pCfg->DeviceIDAddr = DEFAULT_ADDR;
	pCfg->strCSVFile= "";

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

	pCfg->strCSVFile=m_strCSVFile;

	//////////////
	CString strIniFile;
	CString strTemp;
	strIniFile.Format("%s\\sngen\\SCS_SN_ESP32.ini", CComFunc::GetCurrentPath());

	strTemp.Format("%I64X", pCfg->DeviceIDAddr);
	WritePrivateProfileString("Config", "SNAddress", (LPCTSTR)strTemp, strIniFile);


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
	UINT GroupCnt=1;

	CString CImage = _T("ACSV");
	CString LogMsg;
	UINT16 CNum = 6;
	UINT16 index = 1;
	UINT16 infoLen = 0;

	INT nTotalLen = 0;
	tParamCol ParamCol;

	tSNCustomCfg* pCfg=m_SnCfg.GetCustomCfg();
	if(m_CSVFile.IsReady()==FALSE){
		MessageBox("Please check the csv file is opened");
		Ret=-1; goto __end;
	}

	Ret=m_CSVFile.ReadSN(Idx, ParamCol, nTotalLen);
	if(Ret!=0){
		MessageBox(m_CSVFile.GetErrMsg());
		Ret=-1; goto __end;
	}

	//header
	nTotalLen += (4+2+4*6);
	lSerial<<GroupCnt;
	lSerial<<pCfg->DeviceIDAddr<<nTotalLen;
	lSerial.SerialInBuff((BYTE*)CImage.GetBuffer(), CImage.GetLength());
	CImage.ReleaseBuffer();
	lSerial<<CNum;

	//DevName
	infoLen = (UINT16)ParamCol.strDevNameCol.GetLength();
	lSerial<<index<<infoLen;
	lSerial.SerialInBuff((BYTE*)ParamCol.strDevNameCol.GetBuffer(), ParamCol.strDevNameCol.GetLength());
	ParamCol.strDevNameCol.ReleaseBuffer();
	index++;

	//DevSecret
	infoLen = (UINT16)ParamCol.strDevSecretCol.GetLength();
	lSerial<<index<<infoLen;
	lSerial.SerialInBuff((BYTE*)ParamCol.strDevSecretCol.GetBuffer(), ParamCol.strDevSecretCol.GetLength());
	ParamCol.strDevSecretCol.ReleaseBuffer();
	index++;

	//ProdKey
	infoLen = (UINT16)ParamCol.strProdKeyCol.GetLength();
	lSerial<<index<<infoLen;
	lSerial.SerialInBuff((BYTE*)ParamCol.strProdKeyCol.GetBuffer(), ParamCol.strProdKeyCol.GetLength());
	ParamCol.strProdKeyCol.ReleaseBuffer();
	index++;
	
	//ProdSecret
	infoLen = (UINT16)ParamCol.strProdSecretCol.GetLength();
	lSerial<<index<<infoLen;
	lSerial.SerialInBuff((BYTE*)ParamCol.strProdSecretCol.GetBuffer(), ParamCol.strProdSecretCol.GetLength());
	ParamCol.strProdSecretCol.ReleaseBuffer();
	index++;

	//Pid
	infoLen = (UINT16)ParamCol.strPidCol.GetLength();
	lSerial<<index<<infoLen;
	lSerial.SerialInBuff((BYTE*)ParamCol.strPidCol.GetBuffer(), ParamCol.strPidCol.GetLength());
	ParamCol.strPidCol.ReleaseBuffer();
	index++;

	//Mac
	infoLen = (UINT16)MAC_LEN_BYTE_SIZE;
	lSerial<<index<<infoLen;
	lSerial.SerialInBuff(ParamCol.MacCol, MAC_LEN_BYTE_SIZE);

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
	return 0;
}

void CDlgSNCustom::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	/*DDX_Text(pDX, IDC_DEVICEID_ADDR, m_DeviceAddr);*/
	DDX_Text(pDX, IDC_CSVFILE, m_strCSVFile);
}


BEGIN_MESSAGE_MAP(CDlgSNCustom, CDialog)
	ON_BN_CLICKED(IDC_BTNSELCSV, &CDlgSNCustom::OnBnClickedBtnselcsv)
	ON_EN_KILLFOCUS(IDC_DEVICEID_ADDR, &CDlgSNCustom::OnEnKillfocusEdit)
END_MESSAGE_MAP()


// CDlgSNCustom 消息处理程序

BOOL CDlgSNCustom::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化

	CString strIniFile;
	strIniFile.Format("%s\\sngen\\SCS_SN_ESP32.ini", CComFunc::GetCurrentPath());
	if (PathFileExists(strIniFile)){
		tSNCustomCfg* pCfg=m_SnCfg.GetCustomCfg();

		CString strTemp;
		CHAR TmpBuf[512];
		memset(TmpBuf,0,512);
		GetPrivateProfileString("Config", "SNAddress", "",TmpBuf, MAX_PATH, strIniFile);
		strTemp.Format("%s", TmpBuf);
		sscanf(strTemp, "%I64X", &(pCfg->DeviceIDAddr));
	}

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
