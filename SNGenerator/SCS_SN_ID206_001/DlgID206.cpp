// DlgID206.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ID206.h"
#include "DlgID206.h"
#include "../Com/ComTool.h"

#include <conio.h>
#include "libxl.h"

using namespace libxl;


#define isdigital(a)	(((a)<='9' && (a)>='0')?1:0)
#define isupper(a)	(((a)>='A' && (a)<='F')?1:0)
#define islower(a)		(((a)>='a' && (a)<='f')?1:0)
#define isalpha_hex(a)	((((a)>='A' && (a)<='F')||((a)>='a'&&(a)<='f'))?1:0)
// CDlgID206 �Ի���

IMPLEMENT_DYNAMIC(CDlgID206, CDialog)

CDlgID206::CDlgID206(DRVSNCFGPARA *pSNCfgPara,CWnd* pParent /*=NULL*/)
	: m_pSNCfgPara(pSNCfgPara),CDialog(CDlgID206::IDD, pParent)
	, m_strFile(_T(""))
	, m_strID(_T(""))
	, m_strFactory(_T(""))
	, m_strYear(_T(""))
{
	tSNCustomCfg* pCfg=m_SnCfg.GetCustomCfg();

	pCfg->uStartAddr = 0xBCD000;
	pCfg->uLenUUID =16 ;
	pCfg->uLenKEY =32 ;
	pCfg->uLenMAC =12 ;
	pCfg->uLenLINK =27 ;
	pCfg->uTotalLen=pCfg->uLenUUID +pCfg->uLenKEY+pCfg->uLenMAC+pCfg->uLenLINK ;
	
	pCfg->UUIDAddr = 0xBCD000;
	pCfg->KeyAddr = 0xBCD010;
	pCfg->MacAddr = 0xBCD030;
	pCfg->LinkAddr = 0xBCD03c;

	pCfg->strCSVFile="";

}

CDlgID206::~CDlgID206()
{

}

BOOL CDlgID206::InitCtrlsValue(CSNCustomCfg& SNCfg)
{
	tSNCustomCfg* pCfg=SNCfg.GetCustomCfg();
	CString strText;

	m_strCSVFile=pCfg->strCSVFile;

	///////////////////////
	strText.Format("%d",pCfg->uLenUUID);
	GetDlgItem(IDC_EDIT_UUID_LEN)->SetWindowText(strText);

	strText.Format("%d",pCfg->uLenMAC);
	GetDlgItem(IDC_EDIT_MAC_LEN)->SetWindowText(strText);

	strText.Format("%d",pCfg->uLenKEY);
	GetDlgItem(IDC_EDIT_KEY_LEN)->SetWindowText(strText);

	strText.Format("%d",pCfg->uLenLINK);
	GetDlgItem(IDC_EDIT_LINK_LEN)->SetWindowText(strText);

	/*strText.Format("%I64X",pCfg->uStartAddr);
	GetDlgItem(IDC_EDIT_FIRSTADDR)->SetWindowText(strText);*/

	strText.Format("%I64X",pCfg->UUIDAddr);
	GetDlgItem(IDC_EDIT_UUID_ADR)->SetWindowText(strText);

	strText.Format("%I64X",pCfg->KeyAddr);
	GetDlgItem(IDC_EDIT_KEY_ADR)->SetWindowText(strText);

	strText.Format("%I64X",pCfg->MacAddr);
	GetDlgItem(IDC_EDIT_MAC_ADR)->SetWindowText(strText);

	strText.Format("%I64X",pCfg->LinkAddr);
	GetDlgItem(IDC_EDIT_LINK_ADR)->SetWindowText(strText);
	
	UpdateData(FALSE);
	return TRUE;
}


BOOL CDlgID206::InitCtrls(CSerial& lSerial)
{
	BOOL Ret=TRUE;
	CString strErrMsg;
	if(lSerial.GetLength()!=0){///�ٴδ򿪶Ի����ʱ��ͻᴫ�룬��֮ǰ����Ϊ��Դ���ͷţ�������Ҫ���´��ļ�
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
	else{///�״μ��ص�ʱ��lSerial��û��ֵ
	}
	Ret=InitCtrlsValue(m_SnCfg);
	return Ret;
}

void CDlgID206::OnEnKillfocusEdit()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	SaveCtrls();
}


BOOL CDlgID206::SaveCtrls()
{
	UpdateData(TRUE);
	CString strText;
	tSNCustomCfg* pCfg=m_SnCfg.GetCustomCfg();

	CString strTemp;
	
	GetDlgItem(IDC_EDIT_UUID_LEN)->GetWindowText(strTemp);
	if(sscanf(strTemp,"%d",&pCfg->uLenUUID)!=1){
		MessageBox("��ȡuuid�ֽڳ��ȴ���,��ȷ��");
		return FALSE;
	}

	GetDlgItem(IDC_EDIT_KEY_LEN)->GetWindowText(strTemp);
	if(sscanf(strTemp,"%d",&pCfg->uLenKEY)!=1){
		MessageBox("��ȡkey�ֽڳ��ȴ���,��ȷ��");
		return FALSE;
	}
	
	GetDlgItem(IDC_EDIT_MAC_LEN)->GetWindowText(strTemp);
	if(sscanf(strTemp,"%d",&pCfg->uLenMAC)!=1){
		MessageBox("��ȡmac�ֽڳ��ȴ���,��ȷ��");
		return FALSE;
	}

	GetDlgItem(IDC_EDIT_LINK_LEN)->GetWindowText(strText);
	if(sscanf(strText,"%d",&pCfg->uLenLINK)!=1){
		MessageBox("��ȡlen���ȴ���,��ȷ��");
		return FALSE;
	}
	/////////////////
	/*GetDlgItem(IDC_EDIT_FIRSTADDR)->GetWindowText(strText);
	if(sscanf(strText,"%I64X",&pCfg->uStartAddr)!=1){
		MessageBox("��ȡ��ʼ��ַ����,��ȷ��");
		return FALSE;
	}*/

	GetDlgItem(IDC_EDIT_UUID_ADR)->GetWindowText(strText);
	if(sscanf(strText,"%I64X",&pCfg->UUIDAddr)!=1){
		MessageBox("��ȡuuid��ַ����,��ȷ��");
		return FALSE;
	}

	GetDlgItem(IDC_EDIT_KEY_ADR)->GetWindowText(strText);
	if(sscanf(strText,"%I64X",&pCfg->KeyAddr)!=1){
		MessageBox("��ȡkey��ַ����,��ȷ��");
		return FALSE;
	}

	GetDlgItem(IDC_EDIT_MAC_ADR)->GetWindowText(strText);
	if(sscanf(strText,"%I64X",&pCfg->MacAddr)!=1){
		MessageBox("��ȡmac��ַ����,��ȷ��");
		return FALSE;
	}

	GetDlgItem(IDC_EDIT_LINK_ADR)->GetWindowText(strText);
	if(sscanf(strText,"%I64X",&pCfg->LinkAddr)!=1){
		MessageBox("��ȡlink��ַ����,��ȷ��");
		return FALSE;
	}
	//////////

	pCfg->strCSVFile=m_strCSVFile;
	return TRUE;
}

BOOL CDlgID206::GetCtrls(CSerial&lSerial)
{
	BOOL Ret=TRUE;
	Ret=SaveCtrls();
	if(Ret!=TRUE){
		return Ret;
	}
	return m_SnCfg.SerialOutCfgData(lSerial);
}


INT CDlgID206::TellResult(DWORD Idx,INT IsPass)
{
	INT Ret=0;
	return Ret;
}

INT CDlgID206::PreLoad()
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

INT CDlgID206::QuerySN(UINT AdapIdx,DWORD Idx,BYTE*pData,INT*pSize)
{
	INT Ret=0;
	CSerial lSerial;
	UINT GroupCnt=4;  //�̶�Ϊ����2��

	CString strItemValue;
	CString strErrMsg;
	CString strOneGroupData;

	tSNCustomCfg* pCfg=m_SnCfg.GetCustomCfg();
	Book* book = xlCreateXMLBook();
	if (book == NULL){
		Ret=-1; goto __end;
	}

	if (!book->load((LPCSTR)m_strCSVFile) ){
		MessageBox("load file failed,please check the .xlsx file is opened");
		Ret=-1; goto __end;
	}
	
	Sheet* sheet = book->getSheet(0);
	if (sheet == NULL){
		MessageBox("cannot find the sheet from file");
		Ret=-1; goto __end;
	}

	lSerial<<GroupCnt;
	////////////////////

	const char*  pTemp = sheet->readStr(Idx, 0); //uuid,key,mac,sn,link
	if (pTemp == NULL){
		strErrMsg.Format("cannot find data");
		MessageBox(strErrMsg);
		Ret=-1; goto __end;
	}

	strItemValue.Format("%s", pTemp);
	if (strItemValue.GetLength() != pCfg->uLenUUID){
		strErrMsg.Format("the length of [uuid] is not correct, please check,  expect=%d, real=%d", pCfg->uLenUUID, strItemValue.GetLength());
		MessageBox(strErrMsg);
		Ret=-1; goto __end;
	}
	//strOneGroupData += strItemValue;
	lSerial<<pCfg->UUIDAddr;
	lSerial<<pCfg->uLenUUID;
	lSerial.SerialInBuff((BYTE*)strItemValue.GetBuffer(), strItemValue.GetLength());
	strItemValue.ReleaseBuffer();
	
	pTemp = sheet->readStr(Idx, 1);
	if (pTemp == NULL){
		strErrMsg.Format("cannot find data");
		MessageBox(strErrMsg);
		Ret=-1; goto __end;
	}
	strItemValue.Format("%s", pTemp);
	if (strItemValue.GetLength() != pCfg->uLenKEY){
		strErrMsg.Format("the length of [key] is not correct, please check,  expect=%d, real=%d", pCfg->uLenKEY, strItemValue.GetLength());
		MessageBox(strErrMsg);
		Ret=-1; goto __end;
	}
	//strOneGroupData += strItemValue;
	lSerial<<pCfg->KeyAddr;
	lSerial<<pCfg->uLenKEY;
	lSerial.SerialInBuff((BYTE*)strItemValue.GetBuffer(), strItemValue.GetLength());
	strItemValue.ReleaseBuffer();
	
	pTemp = sheet->readStr(Idx, 2); 
	if (pTemp == NULL){
		strErrMsg.Format("cannot find data");
		MessageBox(strErrMsg);
		Ret=-1; goto __end;
	}
	strItemValue.Format("%s", pTemp);
	if (strItemValue.GetLength() != pCfg->uLenMAC){
		strErrMsg.Format("the length of [mac] is not correct, please check,  expect=%d, real=%d", pCfg->uLenMAC, strItemValue.GetLength());
		MessageBox(strErrMsg);
		Ret=-1; goto __end;
	}
	//strOneGroupData += strItemValue;
	lSerial<<pCfg->MacAddr;
	lSerial<<pCfg->uLenMAC;
	lSerial.SerialInBuff((BYTE*)strItemValue.GetBuffer(), strItemValue.GetLength());
	strItemValue.ReleaseBuffer();
	
	pTemp = sheet->readStr(Idx, 4); 
	if (pTemp == NULL){
		strErrMsg.Format("cannot find data");
		MessageBox(strErrMsg);
		Ret=-1; goto __end;
	}
	strItemValue.Format("%s", pTemp);
	if (strItemValue.GetLength() != pCfg->uLenLINK){
		strErrMsg.Format("the length of [link] is not correct, please check,  expect=%d, real=%d", pCfg->uLenLINK, strItemValue.GetLength());
		MessageBox(strErrMsg);
		Ret=-1; goto __end;
	}
	//strOneGroupData += strItemValue;
	lSerial<<pCfg->LinkAddr;
	lSerial<<pCfg->uLenLINK;
	lSerial.SerialInBuff((BYTE*)strItemValue.GetBuffer(), strItemValue.GetLength());
	strItemValue.ReleaseBuffer();

	if(*pSize<lSerial.GetLength()){///����ط�Ҫע�ⷵ��ʵ��ʹ�õ��ֽ�����ʵ�ʿ������Ĵ�С
		*pSize=lSerial.GetLength();
		Ret=-2; goto __end;
	}
	Ret=lSerial.GetLength();///����ֵ��Ҫ�����������ֽ���
	memcpy(pData,lSerial.GetBuffer(),lSerial.GetLength());
	
__end:
	return Ret;
}

//INT CDlgID206::DoGetFirstGroupData(CString& strData, DWORD Idx)
//{
//	int nRet = FALSE;
//	int index = 0;
//	CString strIdx;
//
//	tSNCustomCfg* pCfg=m_SnCfg.GetCustomCfg();
//
//	if (pCfg->strID.GetLength()  != 6 ){
//		goto __end;
//	}
//	strData += pCfg->strID;
//
//	if (pCfg->strFactory.GetLength() != 2){
//		goto __end;
//	}
//	strData += pCfg->strFactory;
//
//	if (pCfg->strYear.GetLength() != 1){
//		goto __end;
//	}
//	strData += pCfg->strYear;
//
//	if (pCfg->strMonth.GetLength() != 2){
//		goto __end;
//	}
//	strData += pCfg->strMonth;
//
//	if (pCfg->strProductSN.GetLength() != 9){
//		goto __end;
//	}
//	
//	index = _ttoi(pCfg->strProductSN);
//	if (index< 0){
//		goto __end;
//	}
//
//	index += (Idx -1);
//	strIdx.Format("%09d", index);
//	
//	strData+=strIdx ;
//
//	nRet = TRUE;
//
//__end:
//	if (nRet == FALSE){
//		strData.Empty();
//	}
//	return nRet;
//}
//
//int CDlgID206::GetFirstGroupData(CString& strData)
//{
//	int nRet = FALSE;
//	
//	CString strTemp;
//	GetDlgItemText(IDC_EDIT_ID, strTemp);
//	if (strTemp.GetLength() != 6){
//		goto __end;
//	}
//
//	strData+=strTemp;
//	GetDlgItemText(IDC_EDIT_FACTORY, strTemp);
//	if (strTemp.GetLength() != 2){
//		goto __end;
//	}
//
//	strData+=strTemp;
//	GetDlgItemText(IDC_EDIT_YEAR, strTemp);
//	if (strTemp.GetLength() != 1){
//		goto __end;
//	}
//	
//	strData+=strTemp;
//	GetDlgItemText( IDC_EDIT_MONTH, strTemp);
//	if (strTemp.GetLength() != 2){
//		goto __end;
//	}
//	
//	strData+=strTemp;
//	GetDlgItemText(IDC_EDIT_SN, strTemp);
//	if (strTemp.GetLength() != 9){
//		goto __end;
//	}
//	
//	strData+=strTemp;
//	nRet = TRUE;
//
//__end:
//	if (nRet == FALSE){
//		strData.Empty();
//	}
//
//	return nRet;
//}

void CDlgID206::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_CSVFILEPATH, m_strCSVFile);
}


BEGIN_MESSAGE_MAP(CDlgID206, CDialog)
	ON_BN_CLICKED(IDC_BTNID206SEL, &CDlgID206::OnBnClickedBtnselcsv)
	/*ON_EN_KILLFOCUS(IDC_EDIT_ID, &CDlgID206::OnEnKillfocusEdit)*/
	ON_EN_KILLFOCUS(IDC_EDIT_UUID_LEN , &CDlgID206::OnEnKillfocusEdit)
	ON_EN_KILLFOCUS(IDC_EDIT_KEY_LEN , &CDlgID206::OnEnKillfocusEdit)
	ON_EN_KILLFOCUS(IDC_EDIT_MAC_LEN, &CDlgID206::OnEnKillfocusEdit)
	ON_EN_KILLFOCUS(IDC_EDIT_LINK_LEN , &CDlgID206::OnEnKillfocusEdit)
	ON_EN_KILLFOCUS(IDC_EDIT_UUID_ADR , &CDlgID206::OnEnKillfocusEdit)
	ON_EN_KILLFOCUS(IDC_EDIT_KEY_ADR , &CDlgID206::OnEnKillfocusEdit)
	ON_EN_KILLFOCUS(IDC_EDIT_MAC_ADR, &CDlgID206::OnEnKillfocusEdit)
	ON_EN_KILLFOCUS(IDC_EDIT_LINK_ADR , &CDlgID206::OnEnKillfocusEdit)
END_MESSAGE_MAP()




// CDlgID206 ��Ϣ�������

void CDlgID206::OnBnClickedBtnselcsv()
{
	CString strFilePath;
	CString strErrMsg;
	CFileDialog dlgFile(TRUE, NULL, NULL, OFN_OVERWRITEPROMPT, "Excel File(*.xlsx)|*.xlsx||");
	if (dlgFile.DoModal() != IDOK){
		return;
	}
	m_strCSVFile=dlgFile.GetPathName();

	/*m_CSVFile.CloseFile();
	if(m_CSVFile.OpenFile(m_strCSVFile)!=0){
		strErrMsg=m_CSVFile.GetErrMsg();
		MessageBox(strErrMsg,"",MB_OK);
	}*/
	UpdateData(FALSE);
}
