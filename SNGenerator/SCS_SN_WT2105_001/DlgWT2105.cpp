// DlgWT2105.cpp : 实现文件
//

#include "stdafx.h"
#include "WT2105.h"
#include "DlgWT2105.h"
#include "../Com/ComTool.h"

#define isdigital(a)	(((a)<='9' && (a)>='0')?1:0)
#define isupper(a)	(((a)>='A' && (a)<='F')?1:0)
#define islower(a)		(((a)>='a' && (a)<='f')?1:0)
#define isalpha_hex(a)	((((a)>='A' && (a)<='F')||((a)>='a'&&(a)<='f'))?1:0)
// CDlgWT2105 对话框

IMPLEMENT_DYNAMIC(CDlgWT2105, CDialog)

CDlgWT2105::CDlgWT2105(DRVSNCFGPARA *pSNCfgPara,CWnd* pParent /*=NULL*/)
	: m_pSNCfgPara(pSNCfgPara),CDialog(CDlgWT2105::IDD, pParent)
	, m_strFile(_T(""))
	, m_strID(_T(""))
	, m_strFactory(_T(""))
	, m_strYear(_T(""))
{
	tSNCustomCfg* pCfg=m_SnCfg.GetCustomCfg();
	pCfg->DeviceIDAddr=0xBCD014/*0xB05014*/;
	pCfg->DeviceIDLen=32;
	pCfg->KeyAddr=0xBCD034/*0xB05034*/;
	pCfg->ConnpublicKeyLen=216;

	CTime nowTime;
	nowTime = CTime::GetCurrentTime();
	CString strMonth;
	strMonth.Format("%02d", nowTime.GetMonth());
	

	pCfg->strID.Format("%s", "WT2105");          //产品型号
	pCfg->strFactory.Format("%s", "ID");  //生产工厂
	pCfg->strYear.Format("%s", "1");			//年
	pCfg->strMonth.Format("%s",strMonth /*"05"*/);	//月
	pCfg->strProductSN.Format("%s", "000000000");  //流水号

	pCfg->uFirstGroupAddr = 0xBCD000;
	
	pCfg->strCSVFile="";

}

CDlgWT2105::~CDlgWT2105()
{

}

BOOL CDlgWT2105::InitCtrlsValue(CSNCustomCfg& SNCfg)
{
	tSNCustomCfg* pCfg=SNCfg.GetCustomCfg();
	CString strText;

	strText.Format("%s", pCfg->strID);
	GetDlgItem(IDC_EDIT_ID)->SetWindowText(strText);
	
	strText.Format("%s", pCfg->strFactory);
	GetDlgItem(IDC_EDIT_FACTORY)->SetWindowText(strText);
	
	strText.Format("%s", pCfg->strYear);
	GetDlgItem(IDC_EDIT_YEAR)->SetWindowText(strText);
	
	strText.Format("%s", pCfg->strMonth);
	GetDlgItem(IDC_EDIT_MONTH)->SetWindowText(strText);

	strText.Format("%s", pCfg->strProductSN);
	GetDlgItem(IDC_EDIT_SN)->SetWindowText(strText);

	m_strCSVFile=pCfg->strCSVFile;

	///////////////////////
	strText.Format("%d",pCfg->DeviceIDLen);
	GetDlgItem(IDC_EDIT_DIDLEN)->SetWindowText(strText);

	strText.Format("%d",pCfg->ConnpublicKeyLen);
	GetDlgItem(IDC_EDIT_KEYLEN)->SetWindowText(strText);

	///
	strText.Format("%I64X",pCfg->DeviceIDAddr);
	GetDlgItem(IDC_EDIT_DIDADDR)->SetWindowText(strText);

	strText.Format("%I64X",pCfg->KeyAddr);
	GetDlgItem(IDC_EDIT_KEYADDR)->SetWindowText(strText);
	//////////////////////
	
	strText.Format("%I64X",pCfg->uFirstGroupAddr);
	GetDlgItem(IDC_EDIT_FIRSTADDR)->SetWindowText(strText);
	

	UpdateData(FALSE);
	return TRUE;
}


BOOL CDlgWT2105::InitCtrls(CSerial& lSerial)
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

void CDlgWT2105::OnEnKillfocusEdit()
{
	// TODO: 在此添加控件通知处理程序代码
	SaveCtrls();
}


BOOL CDlgWT2105::SaveCtrls()
{
	UpdateData(TRUE);
	CString strText;
	tSNCustomCfg* pCfg=m_SnCfg.GetCustomCfg();

	CString strTemp;
	GetDlgItemText(IDC_EDIT_ID, strTemp);
	if (strTemp.GetLength() != 6){
		MessageBox("获取产品型号的长度错误,请确认");
		return FALSE;
	}
	pCfg->strID.Format("%s", strTemp);
	
	GetDlgItemText(IDC_EDIT_FACTORY, strTemp);
	if (strTemp.GetLength()  != 2){
		MessageBox("获取生产厂商的长度错误,请确认");
		return FALSE;
	}
	pCfg->strFactory.Format("%s", strTemp);

	GetDlgItemText(IDC_EDIT_YEAR, strTemp);
	if (strTemp.GetLength()  != 1){
		MessageBox("获取生产年份长度错误,请确认");
		return FALSE;
	}
	pCfg->strYear.Format("%s", strTemp);

	GetDlgItemText( IDC_EDIT_MONTH, strTemp);
	if (strTemp.GetLength()  != 2){
		MessageBox("获取生产月份长度错误,请确认");
		return FALSE;
	}
	pCfg->strMonth.Format("%s", strTemp);

	GetDlgItemText(IDC_EDIT_SN, strTemp);
	if (strTemp.GetLength()  != 9){
		MessageBox("获取产品流水号错误,请确认");
		return FALSE;
	}
	pCfg->strProductSN.Format("%s", strTemp);

	///////////////
	GetDlgItem(IDC_EDIT_DIDLEN)->GetWindowText(strTemp);
	if(sscanf(strTemp,"%d",&pCfg->DeviceIDLen)!=1){
		MessageBox("获取DeviceID字节长度错误,请确认");
		return FALSE;
	}

	GetDlgItem(IDC_EDIT_KEYLEN)->GetWindowText(strTemp);
	if(sscanf(strTemp,"%d",&pCfg->ConnpublicKeyLen)!=1){
		MessageBox("获取ConnpublicKey字节长度错误,请确认");
		return FALSE;
	}

	
	GetDlgItem(IDC_EDIT_KEYADDR)->GetWindowText(strText);
	if(sscanf(strText,"%I64X",&pCfg->KeyAddr)!=1){
		MessageBox("获取Key起始地址错误,请确认");
		return FALSE;
	}

	GetDlgItem(IDC_EDIT_DIDADDR)->GetWindowText(strText);
	if(sscanf(strText,"%I64X",&pCfg->DeviceIDAddr)!=1){
		MessageBox("获取DeviceID起始地址错误,请确认");
		return FALSE;
	}
	/////////////////
	GetDlgItem(IDC_EDIT_FIRSTADDR)->GetWindowText(strText);
	if(sscanf(strText,"%I64X",&pCfg->uFirstGroupAddr)!=1){
		MessageBox("获取第一组起始地址错误,请确认");
		return FALSE;
	}

	pCfg->strCSVFile=m_strCSVFile;
	return TRUE;
}

BOOL CDlgWT2105::GetCtrls(CSerial&lSerial)
{
	BOOL Ret=TRUE;
	Ret=SaveCtrls();
	if(Ret!=TRUE){
		return Ret;
	}
	return m_SnCfg.SerialOutCfgData(lSerial);
}


INT CDlgWT2105::TellResult(DWORD Idx,INT IsPass)
{
	INT Ret=0;
	return Ret;
}

INT CDlgWT2105::PreLoad()
{
	INT Ret=0;
	return Ret;
}
///////////////////////////////
/**************
QuerySN:查询SN
Idx：   请求查询的SN的索引值，该值大于等于1,
pData： 内部将查询得到的SN的信息放到该buffer中
pSize:     pData指向的Buffer的大小
返回值：
 -1： 查询失败
 -2:  pData的内存太小，需要重新指定，pSize返回真正需要的大小。
 >=0:  实际填充的pData的大小。
************************/

INT CDlgWT2105::QuerySN(UINT AdapIdx,DWORD Idx,BYTE*pData,INT*pSize)
{
	INT Ret=0;
	CString Curtime;
	CTime Time;
	CSerial lSerial;
	CString strThirdCol,strSecCol;
	UINT GroupCnt=3;  //固定为定制3组
	CString strFirstGroupData;

	tSNCustomCfg* pCfg=m_SnCfg.GetCustomCfg();
	if(m_CSVFile.IsReady()==FALSE){
		MessageBox("Please check the csv file is opened");
		Ret=-1; goto __end;
	}

	if (DoGetFirstGroupData(strFirstGroupData, Idx ) == FALSE){
		MessageBox("Please check the length of  sn ");
		Ret=-1; goto __end;
	}

	Ret=m_CSVFile.ReadSN(Idx, strSecCol, strThirdCol);
	if(Ret!=0){
		Ret=-1; goto __end;
	}
	
	int nLeSec = strSecCol.GetLength();
	int nLeThir = strThirdCol.GetLength();

	if( strSecCol.GetLength()!=(pCfg->DeviceIDLen) || strThirdCol.GetLength()!= (pCfg->ConnpublicKeyLen) ){
		Ret=-1; goto __end;
	}

	UINT64 FirstGroupAddr = pCfg->uFirstGroupAddr/*0xB05000*/;///第一组地址
	UINT FirstGroupDataLen = 20;	///第一组长度

	lSerial<<GroupCnt;
	////////////////////
	lSerial<<FirstGroupAddr;
	lSerial<<FirstGroupDataLen;
	lSerial.SerialInBuff((BYTE*)strFirstGroupData.GetBuffer(), strFirstGroupData.GetLength());
	////////////////////
	UINT64 DeviceIDAddr  = pCfg->DeviceIDAddr /*0xB05014*/;///DeviceID地址
	UINT DeviceIDLen = pCfg->DeviceIDLen;	///DeviceID长度
	UINT64 ConnpublicKeyAddr =pCfg->KeyAddr/*0xB05034*/ ;///ConnpublicKey的地址
	UINT ConnpublicKeyLen = pCfg->ConnpublicKeyLen;///ConnpublicKey的长度

	lSerial<<DeviceIDAddr;
	lSerial<<DeviceIDLen;
	lSerial.SerialInBuff((BYTE*)strSecCol.GetBuffer(),strSecCol.GetLength());

	lSerial<<ConnpublicKeyAddr;
	lSerial<<ConnpublicKeyLen;
	lSerial.SerialInBuff((BYTE*)strThirdCol.GetBuffer(),strThirdCol.GetLength());

	if(*pSize<lSerial.GetLength()){///这个地方要注意返回实际使用的字节数与实际可以填充的大小
		*pSize=lSerial.GetLength();
		Ret=-2; goto __end;
	}
	Ret=lSerial.GetLength();///返回值需要是正常填充的字节数
	memcpy(pData,lSerial.GetBuffer(),lSerial.GetLength());

__end:
	return Ret;
}

INT CDlgWT2105::DoGetFirstGroupData(CString& strData, DWORD Idx)
{
	int nRet = FALSE;
	int index = 0;
	CString strIdx;

	tSNCustomCfg* pCfg=m_SnCfg.GetCustomCfg();

	if (pCfg->strID.GetLength()  != 6 ){
		goto __end;
	}
	strData += pCfg->strID;

	if (pCfg->strFactory.GetLength() != 2){
		goto __end;
	}
	strData += pCfg->strFactory;

	if (pCfg->strYear.GetLength() != 1){
		goto __end;
	}
	strData += pCfg->strYear;

	if (pCfg->strMonth.GetLength() != 2){
		goto __end;
	}
	strData += pCfg->strMonth;

	if (pCfg->strProductSN.GetLength() != 9){
		goto __end;
	}
	
	index = _ttoi(pCfg->strProductSN);
	if (index< 0){
		goto __end;
	}

	index += (Idx -1);
	strIdx.Format("%09d", index);
	
	strData+=strIdx ;
	/*- m_pSNCfgPara->dwStartIndex); */

	nRet = TRUE;

__end:
	if (nRet == FALSE){
		strData.Empty();
	}
	return nRet;
}

int CDlgWT2105::GetFirstGroupData(CString& strData)
{
	/*UpdateData(TRUE);*/
	int nRet = FALSE;
	
	CString strTemp;
	GetDlgItemText(IDC_EDIT_ID, strTemp);
	if (strTemp.GetLength() != 6){
		goto __end;
	}

	strData+=strTemp;
	GetDlgItemText(IDC_EDIT_FACTORY, strTemp);
	if (strTemp.GetLength() != 2){
		goto __end;
	}

	strData+=strTemp;
	GetDlgItemText(IDC_EDIT_YEAR, strTemp);
	if (strTemp.GetLength() != 1){
		goto __end;
	}
	
	strData+=strTemp;
	GetDlgItemText( IDC_EDIT_MONTH, strTemp);
	if (strTemp.GetLength() != 2){
		goto __end;
	}
	
	strData+=strTemp;
	GetDlgItemText(IDC_EDIT_SN, strTemp);
	if (strTemp.GetLength() != 9){
		goto __end;
	}
	
	strData+=strTemp;
	nRet = TRUE;

__end:
	if (nRet == FALSE){
		strData.Empty();
	}

	return nRet;
}

void CDlgWT2105::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//DDX_Text(pDX, IDC_CSVFILEPATH, m_strFile);
	//DDX_Text(pDX, IDC_EDIT_ID, m_strID);
	//DDX_Text(pDX, IDC_EDIT_FACTORY, m_strFactory);
	//DDX_Text(pDX, IDC_EDIT_YEAR, m_strYear);
	DDX_Text(pDX, IDC_CSVFILEPATH, m_strCSVFile);
}


BEGIN_MESSAGE_MAP(CDlgWT2105, CDialog)
	ON_BN_CLICKED(IDC_BTNWT2105SEL, &CDlgWT2105::OnBnClickedBtnselcsv)
	ON_EN_KILLFOCUS(IDC_EDIT_ID, &CDlgWT2105::OnEnKillfocusEdit)
	ON_EN_KILLFOCUS(IDC_EDIT_FACTORY , &CDlgWT2105::OnEnKillfocusEdit)
	ON_EN_KILLFOCUS(IDC_EDIT_YEAR , &CDlgWT2105::OnEnKillfocusEdit)
	ON_EN_KILLFOCUS(IDC_EDIT_MONTH, &CDlgWT2105::OnEnKillfocusEdit)
	ON_EN_KILLFOCUS(IDC_EDIT_SN , &CDlgWT2105::OnEnKillfocusEdit)
	ON_EN_KILLFOCUS(IDC_EDIT_DIDLEN, &CDlgWT2105::OnEnKillfocusEdit)
	ON_EN_KILLFOCUS(IDC_EDIT_KEYLEN, &CDlgWT2105::OnEnKillfocusEdit)
	ON_EN_KILLFOCUS(IDC_EDIT_KEYADDR, &CDlgWT2105::OnEnKillfocusEdit)
	ON_EN_KILLFOCUS(IDC_EDIT_DIDADDR, &CDlgWT2105::OnEnKillfocusEdit)
END_MESSAGE_MAP()




// CDlgWT2105 消息处理程序

void CDlgWT2105::OnBnClickedBtnselcsv()
{
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
