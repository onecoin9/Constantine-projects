// DlgSNStep.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "DlgSNStep.h"
#include <vector>
#include "../Com/ComTool.h"




// CDlgSNStep �Ի���
enum{
	TAG_SNMODE,
	TAG_SNSTYLE,
	TAG_SNMBS,
	TAG_SNSIZE,
	TAG_SNSTARTADDR,
	TAG_SNSTEP,
	TAG_SNSTARTVALUE,
	TAG_SNTOTAL
};

IMPLEMENT_DYNAMIC(CDlgSNStep, CDialog)

CDlgSNStep::CDlgSNStep(DRVSNCFGPARA *pSnCfgPara,CWnd* pParent /*=NULL*/)
	: CDialog(CDlgSNStep::IDD, pParent),
	m_pSnCfgPara(pSnCfgPara)
{

}

CDlgSNStep::~CDlgSNStep()
{
	DestroyWindow();
}

void CDlgSNStep::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_ListCtrlST);
	DDX_Control(pDX, IDC_COMBO1, m_cmbSNGroup);
}


BEGIN_MESSAGE_MAP(CDlgSNStep, CDialog)
	ON_CBN_SELCHANGE(IDC_COMBO1, &CDlgSNStep::OnCbnSelchangeCombo1)
END_MESSAGE_MAP()

LRESULT CALLBACK DlgSNSetText(void *Para,INT nRow,INT nColumn,CString&strText)
{
	CDlgSNStep*pDlgSNStep=(CDlgSNStep*)Para;
	if(pDlgSNStep){
		return pDlgSNStep->SetText(nRow,nColumn,strText);
	}
	return 0;
}

CString CALLBACK DlgSNGetTipText(void *Para,INT nRow,INT nColumn)
{
	CDlgSNStep*pDlgSNStep=(CDlgSNStep*)Para;
	if(pDlgSNStep){
		return pDlgSNStep->GetTipText(nRow,nColumn);
	}
	else{
		return CString("");
	}
}

UINT CALLBACK DlgSNGetTextInfo(void *Para,INT subCmd,INT nRow,INT nColumn)
{
	CDlgSNStep*pDlgSNStep=(CDlgSNStep*)Para;
	if(pDlgSNStep){
		return pDlgSNStep->GetTextInfo(subCmd,nRow,nColumn);
	}
	return (UINT)-1;
}


// CDlgSNStep ��Ϣ�������

UINT CDlgSNStep::GetTextInfo(INT subCmd,INT nRow,INT nColumn)
{
	SNGROUP* pSnGroup=m_SnStepCfg.GetGroup(nRow);
	if(pSnGroup!=NULL){
		if(subCmd==CMD_GETTEXTLIMIT){
			if(nColumn==TAG_SNSTARTVALUE){
				if(pSnGroup->dwSNMode==SNMODE_ASCII){
					return pSnGroup->dwSNLen;
				}
				else{
					return pSnGroup->dwSNLen*2;
				}
			}
		}
		else if(subCmd==CMD_GETTEXTTYPE){
			if(nColumn==TAG_SNSTARTVALUE || nColumn==TAG_SNSTEP){
				if(pSnGroup->dwSNStyle==SNSTYLE_BCD){
					return CListEditST::DATATYPE_NUM;//�����ǰ���õ���DEC����ֻ����������
				}
				else{
					return CListEditST::DATATYPE_HEX;///������õ�HEX������������16�����ַ�
				}
			}
			///�����������SetInputTypeAsText����
		}
	}
	return (UINT)-1;
}

CString CDlgSNStep::GetTipText(INT nRow,INT nColumn)
{
	switch(nColumn){
		case TAG_SNSIZE:
			return CString("1<=Size<=64 Bytes");
			break;
		case TAG_SNSTYLE:
			return CString("DEC(9->10),HEX(9->A)");
			break;
		case TAG_SNMBS:
			return CString("MBS: Big Endian, LBS: Little Endian");
			break;
		case TAG_SNMODE:
		case TAG_SNSTARTADDR:
		case TAG_SNSTEP:
		case TAG_SNSTARTVALUE:
		default:
			return CString("");
			break;
	}
}

BOOL CDlgSNStep::ModifySNValue(SNGROUP* pSnGroup,INT nRow,INT nColumn)
{
	CString strSNValue;
	INT MaxLen,CurLen,i;
	strSNValue=m_ListCtrlST.GetItemText(nRow,TAG_SNSTARTVALUE);
	if(pSnGroup->dwSNMode==SNMODE_ASCII){///�����ASCII
		MaxLen=pSnGroup->dwSNLen;
	}
	else{
		MaxLen=pSnGroup->dwSNLen*2;
	}
	CurLen=strSNValue.GetLength();
	if(CurLen==MaxLen){
		return TRUE;
	}
	else if(CurLen<MaxLen){
		for(i=CurLen;i<MaxLen;++i){///ǰ�油��0
			strSNValue.Insert(-1,'0');
		}
	}
	else{
		strSNValue.Delete(-1,CurLen-MaxLen);
	}
	m_ListCtrlST.SetItemText(nRow,TAG_SNSTARTVALUE,strSNValue);
	pSnGroup->strSNStartValue=strSNValue;
	m_ListCtrlST.AdjuctColumnWidth(nRow,TAG_SNSTARTVALUE);
	return TRUE;
}

BOOL CDlgSNStep::StrDec2StrHex(CString&str)
{
	BOOL Ret=TRUE;
	std::vector<BYTE>vData;
	BYTE *pData=NULL;
	if(ComTool::Str2Dec(str,ComTool::ENDIAN_BIG,vData)==FALSE){
		Ret=FALSE;
		goto __end;
	}
	pData=new BYTE[vData.size()];
	if(pData){
		for(INT j=0;j<(INT)vData.size();j++){
			pData[j]=vData[j];
		}
		ComTool::Hex2Str(ComTool::ENDIAN_BIG,pData,(INT)vData.size(),str);
		delete[] pData;
	}
__end:
	return Ret;
}

BOOL CDlgSNStep::StrHex2StrDec(CString&str)
{
	BOOL Ret=TRUE;
	std::vector<BYTE>vData;
	BYTE *pData=NULL;
	if(ComTool::Str2Hex(str,ComTool::ENDIAN_BIG,vData)==FALSE){
		Ret=FALSE;
		goto __end;
	}
	pData=new BYTE[vData.size()];
	if(pData){
		for(INT j=0;j<(INT)vData.size();j++){
			pData[j]=vData[j];
		}
		ComTool::Dec2Str(ComTool::ENDIAN_BIG,pData,(INT)vData.size(),str);
		delete[] pData;
	}
__end:
	return Ret;
}

LRESULT CDlgSNStep::SetText(INT nRow,INT nColumn,CString&strText)
{
	if(strText.IsEmpty()){
		CString strErrMsg;
		strErrMsg.Format("SN Error: %s can't be empty",vHeader[nColumn]);
		MessageBox(strErrMsg);
		return -1;
	}
	SNGROUP* pSnGroup=m_SnStepCfg.GetGroup(nRow);
	if(pSnGroup==NULL){
		return 0;
	}
	switch(nColumn){
		case TAG_SNSIZE:
			sscanf(strText,"%d",&pSnGroup->dwSNLen);
			break;
		case TAG_SNSTYLE:
			{
				UINT PreStyle=pSnGroup->dwSNStyle;
				if(strText=="DEC")
					pSnGroup->dwSNStyle=SNSTYLE_BCD;
				else
					pSnGroup->dwSNStyle=SNSTYLE_HEX;
				if(PreStyle!=pSnGroup->dwSNStyle){
					if(PreStyle==SNSTYLE_BCD){
						if(StrDec2StrHex(pSnGroup->strSNStartValue)==FALSE){
							CString strErrMsg;
							strErrMsg.Format("SN Error: %s contain invalid character",vHeader[TAG_SNSTARTVALUE]);
							MessageBox(strErrMsg);
							goto __end;
						}
						else{
							m_ListCtrlST.SetItemText(nRow,TAG_SNSTARTVALUE,pSnGroup->strSNStartValue);
							ModifySNValue(pSnGroup,nRow,nColumn);
						}
						CString strText;
						ComTool::Hex2Str(ComTool::ENDIAN_LIT,(BYTE*)&pSnGroup->dwStep,4,strText);
						if(strText.GetLength()>8){
							strText.Delete(-1,strText.GetLength()-8);
						}
						m_ListCtrlST.SetItemText(nRow,TAG_SNSTEP,strText);
						sscanf(strText,"%X",&pSnGroup->dwStep);///����д�룬��ֵ�뱣��һ��
					}
					else{
						if(StrHex2StrDec(pSnGroup->strSNStartValue)==FALSE){
							CString strErrMsg;
							strErrMsg.Format("SN Error: %s contain invalid character",vHeader[TAG_SNSTARTVALUE]);
							MessageBox(strErrMsg);
							goto __end;
						}
						else{
							m_ListCtrlST.SetItemText(nRow,TAG_SNSTARTVALUE,pSnGroup->strSNStartValue);
							ModifySNValue(pSnGroup,nRow,nColumn);
						}	
						CString strText;
						ComTool::Dec2Str(ComTool::ENDIAN_LIT,(BYTE*)&pSnGroup->dwStep,4,strText);///תΪ10������ʾ
						if(strText.GetLength()>8){///���ܻᳬ��λ��
							strText.Delete(-1,strText.GetLength()-8);
						}
						m_ListCtrlST.SetItemText(nRow,TAG_SNSTEP,strText);
						sscanf(strText,"%d",&pSnGroup->dwStep);///����д�룬��ֵ�뱣��һ��
					}
					
				}
			}
			break;
		case TAG_SNMBS:
			if(strText=="MSB")
				pSnGroup->dwSNMSB=SNMSB_MSB;
			else
				pSnGroup->dwSNMSB=SNMSB_LSB;
			break;
		case TAG_SNMODE:
			if(strText=="ASCII")
				pSnGroup->dwSNMode=SNMODE_ASCII;
			else
				pSnGroup->dwSNMode=SNMODE_BIN;
			ModifySNValue(pSnGroup,nRow,nColumn);
			break;
		case TAG_SNSTARTADDR:
			sscanf(strText,"%I64X",&pSnGroup->llSNStartAddr);
			break;
		case TAG_SNSTEP:
			if(pSnGroup->dwSNStyle==SNSTYLE_BCD)
				sscanf(strText,"%d",&pSnGroup->dwStep);
			else
				sscanf(strText,"%X",&pSnGroup->dwStep);
			break;
		case TAG_SNSTARTVALUE:
			pSnGroup->strSNStartValue=strText;
			break;
		default:
			break;
	}

__end:
	return 0;

}
BOOL CDlgSNStep::InitCtrlList()
{
	DWORD Style=m_ListCtrlST.GetStyle();
	Style |=LVS_EX_GRIDLINES;
	m_ListCtrlST.SetExtendedStyle(Style);
	m_ListCtrlST.RegistSetTextCallBack(DlgSNSetText,this);///ע�����ݸı�ص�����
	m_ListCtrlST.RegistGetTipTextCallBack(DlgSNGetTipText,this);///ע����ʾ�ص�����
	m_ListCtrlST.RegistGetTextInfoCallBack(DlgSNGetTextInfo,this);///ע���ı��༭��������������ַ�
	///�����з�����
	
	m_ListCtrlST.SetHeight(0);///���������Լ�����

	vHeader.clear();
	vHeader.push_back("SN Mode");   
	vHeader.push_back("SN Style");
	vHeader.push_back("Save To Buffer");
	vHeader.push_back("SN Size");
	vHeader.push_back("Start Address(h)");
	vHeader.push_back("Step");
	vHeader.push_back("Start Value");
	m_ListCtrlST.InitColumnHeader(vHeader);

	std::vector<CString>CmbBoxData;
	CmbBoxData.push_back("ASCII");
	CmbBoxData.push_back("BIN");
	m_ListCtrlST.SetInputTypeAsCombo(TAG_SNMODE,CmbBoxData);

	CmbBoxData.clear();
	CmbBoxData.push_back("DEC");
	CmbBoxData.push_back("HEX");
	m_ListCtrlST.SetInputTypeAsCombo(TAG_SNSTYLE,CmbBoxData);

	CmbBoxData.clear();
	CmbBoxData.push_back("LSB");
	CmbBoxData.push_back("MSB");
	m_ListCtrlST.SetInputTypeAsCombo(TAG_SNMBS,CmbBoxData);


	m_ListCtrlST.SetInputTypeAsText(TAG_SNSIZE,CListEditST::DATATYPE_NUM,2);
	m_ListCtrlST.SetInputTypeAsText(TAG_SNSTARTADDR,CListEditST::DATATYPE_HEX,8);
	m_ListCtrlST.SetInputTypeAsText(TAG_SNSTEP,CListEditST::DATATYPE_HEX,8);
	m_ListCtrlST.SetInputTypeAsText(TAG_SNSTARTVALUE,CListEditST::DATATYPE_STR,(UINT)-1);

	return TRUE;
}

extern void TestComTool(void);
BOOL CDlgSNStep::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  �ڴ���Ӷ���ĳ�ʼ��
	m_cmbSNGroup.AddString("1");
	m_cmbSNGroup.AddString("2");
	m_cmbSNGroup.AddString("3");
	m_cmbSNGroup.AddString("4");

	InitCtrlList();

	TestComTool();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	// �쳣: OCX ����ҳӦ���� FALSE
}

BOOL CDlgSNStep::InitCtrlsValue(CSNStepCfg& SNStepCfg)
{
	INT i=0,GroupCnt;
	CString strData;
	std::vector<CString>ItemData;
	GroupCnt=SNStepCfg.GetGroupNum();
	m_cmbSNGroup.SetCurSel(GroupCnt-1);
	m_ListCtrlST.DeleteAllItems();
	for(i=0;i<GroupCnt;++i){
		SNGROUP* pSnGroup=SNStepCfg.GetGroup(i);
		ItemData.clear();
		if(pSnGroup->dwSNMode==SNMODE_BIN)
			ItemData.push_back("BIN");
		else
			ItemData.push_back("ASCII");
		
		if(pSnGroup->dwSNStyle==SNSTYLE_HEX)
			ItemData.push_back("HEX");
		else
			ItemData.push_back("DEC");

		if(pSnGroup->dwSNMSB==SNMSB_MSB)
			ItemData.push_back("MSB");
		else
			ItemData.push_back("LSB");

		strData.Format("%d",pSnGroup->dwSNLen);
		ItemData.push_back(strData);
		strData.Format("%I64X",pSnGroup->llSNStartAddr);
		ItemData.push_back(strData);

		if(pSnGroup->dwSNStyle==SNSTYLE_HEX){
			strData.Format("%X",pSnGroup->dwStep);
		}
		else{
			strData.Format("%d",pSnGroup->dwStep);
		}
		ItemData.push_back(strData);
		ItemData.push_back(pSnGroup->strSNStartValue);
		m_ListCtrlST.AppendItem(ItemData);
	}
	return TRUE;
}

void CDlgSNStep::OnOK()
{
	// TODO: �ڴ����ר�ô����/����û���
	//CDialog::OnOK();
}

BOOL CDlgSNStep::InitCtrls( CSerial& lSerial )
{
	BOOL Ret=TRUE;
	if(lSerial.GetLength()!=0){
		Ret=m_SnStepCfg.SerialInCfgData(lSerial);
	}
	else{
	}
	Ret=InitCtrlsValue(m_SnStepCfg);
	return Ret;
}

BOOL CDlgSNStep::GetCtrls( CSerial&lSerial )
{
	return m_SnStepCfg.SerialOutCfgData(lSerial);
}
void CDlgSNStep::OnCbnSelchangeCombo1()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	INT CurGroup=m_cmbSNGroup.GetCurSel()+1;
	INT GroupCnt=m_SnStepCfg.GetGroupNum();
	if(CurGroup==GroupCnt)
		return;
	
	if(CurGroup>GroupCnt){
		m_SnStepCfg.AppendGroup(CurGroup-GroupCnt);
	}
	else{
		m_SnStepCfg.RemoveGroup(GroupCnt-CurGroup);
	}
	InitCtrlsValue(m_SnStepCfg);
}

INT CDlgSNStep::QuerySN(DWORD Idx,BYTE*pData,INT*pSize)
{
	INT Ret=0;
	CSerial lSerial;
	BYTE*pSNData=NULL;
	INT i,j,GroupCnt,SNSize,PreSNSize,BytesFillZero;
	std::vector<BYTE>vMulData;
	std::vector<BYTE>vBaseValue;
	std::vector<BYTE>vSNValue;
	Idx=Idx-1;
	GroupCnt=m_SnStepCfg.GetGroupNum();
	lSerial<<GroupCnt;
	
	for(i=0;i<GroupCnt;++i){
		SNGROUP* pSnGroup=m_SnStepCfg.GetGroup(i);
		lSerial<<pSnGroup->llSNStartAddr<<pSnGroup->dwSNLen;
		vMulData.clear();
		vBaseValue.clear();
		vSNValue.clear();
		ComTool::MultiBytesMul(ComTool::ENDIAN_LIT,(BYTE*)&Idx,sizeof(DWORD),(BYTE*)&pSnGroup->dwStep,sizeof(UINT),vMulData);
		if(pSnGroup->dwSNStyle==SNSTYLE_HEX){
			ComTool::Str2Hex(pSnGroup->strSNStartValue,ComTool::ENDIAN_LIT,vBaseValue);
		}
		else{
			ComTool::Str2Dec(pSnGroup->strSNStartValue,ComTool::ENDIAN_LIT,vBaseValue);
		}
		ComTool::MultiBytesAdd(ComTool::ENDIAN_LIT,vBaseValue,vMulData,vSNValue);

		if(pSnGroup->dwSNStyle==SNSTYLE_BCD){///������Ҫ��10���ƿ�Ҳ����0x0AҪ��תΪ0x10
			CString strDEC;
			ComTool::Dec2Str(ComTool::ENDIAN_LIT,vSNValue,strDEC);
			ComTool::Str2Hex(strDEC,ComTool::ENDIAN_LIT,vSNValue);
		}

		BytesFillZero=pSnGroup->dwSNLen>(INT)vSNValue.size()?(pSnGroup->dwSNLen-(INT)vSNValue.size()):0;
		for(j=0;j<BytesFillZero;j++){///��������δ�ﵽ��Ч�ֽ����λ����0
			vSNValue.push_back((BYTE)0);
		}

		SNSize=pSnGroup->dwSNLen;///ֻȡ��ô���ֽ�
		if(pSNData==NULL){
			pSNData=new BYTE[SNSize];
			PreSNSize=SNSize;
		}
		else{
			if(SNSize!=PreSNSize){
				delete[] pSNData;
				pSNData=new BYTE[SNSize];
				PreSNSize=SNSize;
			}
		}
		if(!pSNData){
			Ret=-1;
			goto __end;
		}
		if(pSnGroup->dwSNMode==SNMODE_BIN){///����16���ƽ������
			if(pSnGroup->dwSNMSB==SNMSB_MSB){
				for(j=0;j<SNSize;j++){///תΪ���ģʽ���
					pSNData[j]=vSNValue[SNSize-1-j];
				}
			}
			else{
				for(j=0;j<SNSize;j++){///С�˴��
					pSNData[j]=vSNValue[j];
				}
			}
		}
		else{
			INT strLen;
			CString strSNValue;
			ComTool::Hex2Str(ComTool::ENDIAN_LIT,vSNValue,strSNValue);///��ȡ���������ַ���
			strLen=strSNValue.GetLength();
			if(strLen<SNSize){
				for(j=SNSize-strLen;j>0;j--){///ǰ�����0
					strSNValue.Insert(-1,'0');
				}
				strLen=SNSize;
			}
			if(pSnGroup->dwSNMSB==SNMSB_MSB){
				for(j=0;j<SNSize;j++){///��˴��
					pSNData[j]=strSNValue.GetAt(j);
				}
			}
			else{
				for(j=0;j<SNSize;j++){///����С�˴��
					pSNData[j]=strSNValue.GetAt(strLen-1-j);
				}
			}
		}
		lSerial.SerialInBuff(pSNData,SNSize);
	}
	if(pData!=NULL&&*pSize>=(INT)lSerial.GetLength()){
		memcpy(pData,lSerial.GetBuffer(),lSerial.GetLength());
		Ret=lSerial.GetLength();
	}
	else{
		Ret=-2;
		*pSize=lSerial.GetLength();
	}
__end:
	if(pSNData){
		delete[] pSNData;
	}
	return Ret;
}
