// DlgWT2105.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "WT2105.h"
#include "DlgWT2105.h"
#include "../Com/ComTool.h"

#define isdigital(a)	(((a)<='9' && (a)>='0')?1:0)
#define isupper(a)	(((a)>='A' && (a)<='F')?1:0)
#define islower(a)		(((a)>='a' && (a)<='f')?1:0)
#define isalpha_hex(a)	((((a)>='A' && (a)<='F')||((a)>='a'&&(a)<='f'))?1:0)
// CDlgWT2105 �Ի���

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
	

	pCfg->strID.Format("%s", "WT2105");          //��Ʒ�ͺ�
	pCfg->strFactory.Format("%s", "ID");  //��������
	pCfg->strYear.Format("%s", "1");			//��
	pCfg->strMonth.Format("%s",strMonth /*"05"*/);	//��
	pCfg->strProductSN.Format("%s", "000000000");  //��ˮ��

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
	if(lSerial.GetLength()!=0){///�ٴδ򿪶Ի����ʱ��ͻᴫ�룬��֮ǰ����Ϊ��Դ���ͷţ�������Ҫ���´��ļ�
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
	else{///�״μ��ص�ʱ��lSerial��û��ֵ
	}
	Ret=InitCtrlsValue(m_SnCfg);
	return Ret;
}

void CDlgWT2105::OnEnKillfocusEdit()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
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
		MessageBox("��ȡ��Ʒ�ͺŵĳ��ȴ���,��ȷ��");
		return FALSE;
	}
	pCfg->strID.Format("%s", strTemp);
	
	GetDlgItemText(IDC_EDIT_FACTORY, strTemp);
	if (strTemp.GetLength()  != 2){
		MessageBox("��ȡ�������̵ĳ��ȴ���,��ȷ��");
		return FALSE;
	}
	pCfg->strFactory.Format("%s", strTemp);

	GetDlgItemText(IDC_EDIT_YEAR, strTemp);
	if (strTemp.GetLength()  != 1){
		MessageBox("��ȡ������ݳ��ȴ���,��ȷ��");
		return FALSE;
	}
	pCfg->strYear.Format("%s", strTemp);

	GetDlgItemText( IDC_EDIT_MONTH, strTemp);
	if (strTemp.GetLength()  != 2){
		MessageBox("��ȡ�����·ݳ��ȴ���,��ȷ��");
		return FALSE;
	}
	pCfg->strMonth.Format("%s", strTemp);

	GetDlgItemText(IDC_EDIT_SN, strTemp);
	if (strTemp.GetLength()  != 9){
		MessageBox("��ȡ��Ʒ��ˮ�Ŵ���,��ȷ��");
		return FALSE;
	}
	pCfg->strProductSN.Format("%s", strTemp);

	///////////////
	GetDlgItem(IDC_EDIT_DIDLEN)->GetWindowText(strTemp);
	if(sscanf(strTemp,"%d",&pCfg->DeviceIDLen)!=1){
		MessageBox("��ȡDeviceID�ֽڳ��ȴ���,��ȷ��");
		return FALSE;
	}

	GetDlgItem(IDC_EDIT_KEYLEN)->GetWindowText(strTemp);
	if(sscanf(strTemp,"%d",&pCfg->ConnpublicKeyLen)!=1){
		MessageBox("��ȡConnpublicKey�ֽڳ��ȴ���,��ȷ��");
		return FALSE;
	}

	
	GetDlgItem(IDC_EDIT_KEYADDR)->GetWindowText(strText);
	if(sscanf(strText,"%I64X",&pCfg->KeyAddr)!=1){
		MessageBox("��ȡKey��ʼ��ַ����,��ȷ��");
		return FALSE;
	}

	GetDlgItem(IDC_EDIT_DIDADDR)->GetWindowText(strText);
	if(sscanf(strText,"%I64X",&pCfg->DeviceIDAddr)!=1){
		MessageBox("��ȡDeviceID��ʼ��ַ����,��ȷ��");
		return FALSE;
	}
	/////////////////
	GetDlgItem(IDC_EDIT_FIRSTADDR)->GetWindowText(strText);
	if(sscanf(strText,"%I64X",&pCfg->uFirstGroupAddr)!=1){
		MessageBox("��ȡ��һ����ʼ��ַ����,��ȷ��");
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
QuerySN:��ѯSN
Idx��   �����ѯ��SN������ֵ����ֵ���ڵ���1,
pData�� �ڲ�����ѯ�õ���SN����Ϣ�ŵ���buffer��
pSize:     pDataָ���Buffer�Ĵ�С
����ֵ��
 -1�� ��ѯʧ��
 -2:  pData���ڴ�̫С����Ҫ����ָ����pSize����������Ҫ�Ĵ�С��
 >=0:  ʵ������pData�Ĵ�С��
************************/

INT CDlgWT2105::QuerySN(UINT AdapIdx,DWORD Idx,BYTE*pData,INT*pSize)
{
	INT Ret=0;
	CString Curtime;
	CTime Time;
	CSerial lSerial;
	CString strThirdCol,strSecCol;
	UINT GroupCnt=3;  //�̶�Ϊ����3��
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

	UINT64 FirstGroupAddr = pCfg->uFirstGroupAddr/*0xB05000*/;///��һ���ַ
	UINT FirstGroupDataLen = 20;	///��һ�鳤��

	lSerial<<GroupCnt;
	////////////////////
	lSerial<<FirstGroupAddr;
	lSerial<<FirstGroupDataLen;
	lSerial.SerialInBuff((BYTE*)strFirstGroupData.GetBuffer(), strFirstGroupData.GetLength());
	////////////////////
	UINT64 DeviceIDAddr  = pCfg->DeviceIDAddr /*0xB05014*/;///DeviceID��ַ
	UINT DeviceIDLen = pCfg->DeviceIDLen;	///DeviceID����
	UINT64 ConnpublicKeyAddr =pCfg->KeyAddr/*0xB05034*/ ;///ConnpublicKey�ĵ�ַ
	UINT ConnpublicKeyLen = pCfg->ConnpublicKeyLen;///ConnpublicKey�ĳ���

	lSerial<<DeviceIDAddr;
	lSerial<<DeviceIDLen;
	lSerial.SerialInBuff((BYTE*)strSecCol.GetBuffer(),strSecCol.GetLength());

	lSerial<<ConnpublicKeyAddr;
	lSerial<<ConnpublicKeyLen;
	lSerial.SerialInBuff((BYTE*)strThirdCol.GetBuffer(),strThirdCol.GetLength());

	if(*pSize<lSerial.GetLength()){///����ط�Ҫע�ⷵ��ʵ��ʹ�õ��ֽ�����ʵ�ʿ������Ĵ�С
		*pSize=lSerial.GetLength();
		Ret=-2; goto __end;
	}
	Ret=lSerial.GetLength();///����ֵ��Ҫ�����������ֽ���
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




// CDlgWT2105 ��Ϣ�������

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
