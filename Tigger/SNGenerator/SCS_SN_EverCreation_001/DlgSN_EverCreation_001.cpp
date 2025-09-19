// DlgSN_EverCreation_001.cpp : 实现文件
//

#include "stdafx.h"
#include "DlgSN_EverCreation_001.h"
#include <vector>
#include "../Com/ComTool.h"
#include "../Com/ComFunc.h"


// CDlgSN_EverCreation_001 对话框
enum{
	TAG_SNMODE,
	TAG_SNSTYLE,
	TAG_SNMBS,
	TAG_SNSIZE,
	TAG_SNSTARTADDR,
	TAG_SN_EverCreation_001,
	TAG_SNSTARTVALUE,
	TAG_SNTOTAL
};

IMPLEMENT_DYNAMIC(CDlgSN_EverCreation_001, CDialog)

CDlgSN_EverCreation_001::CDlgSN_EverCreation_001(DRVSNCFGPARA *pSnCfgPara,CWnd* pParent /*=NULL*/)
	: CDialog(CDlgSN_EverCreation_001::IDD, pParent),
	m_pSnCfgPara(pSnCfgPara)
{
	InitializeCriticalSection(&m_csQuery);
}

CDlgSN_EverCreation_001::~CDlgSN_EverCreation_001()
{
	DestroyWindow();
	DeleteCriticalSection(&m_csQuery);
}

void CDlgSN_EverCreation_001::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_ListCtrlST);
	DDX_Control(pDX, IDC_COMBO1, m_cmbSNGroup);
}


BEGIN_MESSAGE_MAP(CDlgSN_EverCreation_001, CDialog)
	ON_CBN_SELCHANGE(IDC_COMBO1, &CDlgSN_EverCreation_001::OnCbnSelchangeCombo1)
END_MESSAGE_MAP()

LRESULT CALLBACK DlgSNSetText(void *Para,INT nRow,INT nColumn,CString&strText)
{
	CDlgSN_EverCreation_001*pDlgSN_EverCreation_001=(CDlgSN_EverCreation_001*)Para;
	if(pDlgSN_EverCreation_001){
		return pDlgSN_EverCreation_001->SetText(nRow,nColumn,strText);
	}
	return 0;
}

CString CALLBACK DlgSNGetTipText(void *Para,INT nRow,INT nColumn)
{
	CDlgSN_EverCreation_001*pDlgSN_EverCreation_001=(CDlgSN_EverCreation_001*)Para;
	if(pDlgSN_EverCreation_001){
		return pDlgSN_EverCreation_001->GetTipText(nRow,nColumn);
	}
	else{
		return CString("");
	}
}

UINT CALLBACK DlgSNGetTextInfo(void *Para,INT subCmd,INT nRow,INT nColumn)
{
	CDlgSN_EverCreation_001*pDlgSN_EverCreation_001=(CDlgSN_EverCreation_001*)Para;
	if(pDlgSN_EverCreation_001){
		return pDlgSN_EverCreation_001->GetTextInfo(subCmd,nRow,nColumn);
	}
	return (UINT)-1;
}


// CDlgSN_EverCreation_001 消息处理程序

UINT CDlgSN_EverCreation_001::GetTextInfo(INT subCmd,INT nRow,INT nColumn)
{
	SNGROUP* pSnGroup=m_SN_EverCreation_001Cfg.GetGroup(nRow);
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
			if(nColumn==TAG_SNSTARTVALUE || nColumn==TAG_SN_EverCreation_001){
				if(pSnGroup->dwSNStyle==SNSTYLE_BCD){
					return CListEditST::DATATYPE_NUM;//如果当前设置的是DEC，则只能输入数字
				}
				else{
					return CListEditST::DATATYPE_HEX;///如果设置的HEX，则允许输入16进制字符
				}
			}
			///其他情况交给SetInputTypeAsText负责
		}
	}
	return (UINT)-1;
}

CString CDlgSN_EverCreation_001::GetTipText(INT nRow,INT nColumn)
{
	switch(nColumn+3){
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
		case TAG_SN_EverCreation_001:
		case TAG_SNSTARTVALUE:
		default:
			return CString("");
			break;
	}
}

BOOL CDlgSN_EverCreation_001::ModifySNValue(SNGROUP* pSnGroup,INT nRow,INT nColumn)
{
	//CString strSNValue;
	//INT MaxLen,CurLen,i;
	//strSNValue=m_ListCtrlST.GetItemText(nRow,TAG_SNSTARTVALUE);
	//if(pSnGroup->dwSNMode==SNMODE_ASCII){///如果是ASCII
	//	MaxLen=pSnGroup->dwSNLen;
	//}
	//else{
	//	MaxLen=pSnGroup->dwSNLen*2;
	//}
	//CurLen=strSNValue.GetLength();
	//if(CurLen==MaxLen){
	//	return TRUE;
	//}
	//else if(CurLen<MaxLen){
	//	for(i=CurLen;i<MaxLen;++i){///前面补充0
	//		strSNValue.Insert(-1,'0');
	//	}
	//}
	//else{
	//	strSNValue.Delete(-1,CurLen-MaxLen);
	//}
	////m_ListCtrlST.SetItemText(nRow,TAG_SNSTARTVALUE,strSNValue);
	//pSnGroup->strSNStartValue=strSNValue;
	////m_ListCtrlST.AdjuctColumnWidth(nRow,TAG_SNSTARTVALUE);
	return TRUE;
}

BOOL CDlgSN_EverCreation_001::StrDec2StrHex(CString&str)
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

BOOL CDlgSN_EverCreation_001::StrHex2StrDec(CString&str)
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

LRESULT CDlgSN_EverCreation_001::SetText(INT nRow,INT nColumn,CString&strText)
{
	if(strText.IsEmpty()){
		CString strErrMsg;
		strErrMsg.Format("SN Error: %s can't be empty",vHeader[nColumn]);
		MessageBox(strErrMsg);
		return -1;
	}
	SNGROUP* pSnGroup=m_SN_EverCreation_001Cfg.GetGroup(nRow);
	if(pSnGroup==NULL){
		return 0;
	}
	switch(nColumn + 3){
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
							/*strErrMsg.Format("SN Error: %s contain invalid character",vHeader[TAG_SNSTARTVALUE]);
							MessageBox(strErrMsg);*/
							goto __end;
						}
						else{
							/*m_ListCtrlST.SetItemText(nRow,TAG_SNSTARTVALUE,pSnGroup->strSNStartValue);
							ModifySNValue(pSnGroup,nRow,nColumn);*/
						}
						CString strText;
						ComTool::Hex2Str(ComTool::ENDIAN_LIT,(BYTE*)&pSnGroup->dwStep,4,strText);
						if(strText.GetLength()>8){
							strText.Delete(-1,strText.GetLength()-8);
						}
						//m_ListCtrlST.SetItemText(nRow,TAG_SN_EverCreation_001,strText);
						sscanf(strText,"%X",&pSnGroup->dwStep);///重新写入，将值与保持一致
					}
					else{
						if(StrHex2StrDec(pSnGroup->strSNStartValue)==FALSE){
							CString strErrMsg;
						/*	strErrMsg.Format("SN Error: %s contain invalid character",vHeader[TAG_SNSTARTVALUE]);
							MessageBox(strErrMsg);*/
							goto __end;
						}
						else{
							/*m_ListCtrlST.SetItemText(nRow,TAG_SNSTARTVALUE,pSnGroup->strSNStartValue);
							ModifySNValue(pSnGroup,nRow,nColumn);*/
						}	
						CString strText;
						ComTool::Dec2Str(ComTool::ENDIAN_LIT,(BYTE*)&pSnGroup->dwStep,4,strText);///转为10进制显示
						if(strText.GetLength()>8){///可能会超出位数
							strText.Delete(-1,strText.GetLength()-8);
						}
						//m_ListCtrlST.SetItemText(nRow,TAG_SN_EverCreation_001,strText);
						sscanf(strText,"%d",&pSnGroup->dwStep);///重新写入，将值与保持一致
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
			//ModifySNValue(pSnGroup,nRow,nColumn);
			break;
		case TAG_SNSTARTADDR:
			sscanf(strText,"%I64X",&pSnGroup->llSNStartAddr);
			break;
		case TAG_SN_EverCreation_001:
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
BOOL CDlgSN_EverCreation_001::InitCtrlList()
{
	DWORD Style=m_ListCtrlST.GetStyle();
	Style |=LVS_EX_GRIDLINES;
	m_ListCtrlST.SetExtendedStyle(Style);
	m_ListCtrlST.RegistSetTextCallBack(DlgSNSetText,this);///注册数据改变回调函数
	m_ListCtrlST.RegistGetTipTextCallBack(DlgSNGetTipText,this);///注册提示回调函数
	m_ListCtrlST.RegistGetTextInfoCallBack(DlgSNGetTextInfo,this);///注册文本编辑框输入允许最大字符
	///设置有方格线
	
	m_ListCtrlST.SetHeight(0);///根据字体自己调整

	vHeader.clear();
	//vHeader.push_back("SN Mode");   
	//vHeader.push_back("SN Style");
	//vHeader.push_back("Save To Buffer");
	vHeader.push_back("SN Size");
	vHeader.push_back("Start Address(h)");
	/*vHeader.push_back("Step");
	vHeader.push_back("Start Value");*/
	m_ListCtrlST.InitColumnHeader(vHeader);

	/*std::vector<CString>CmbBoxData;
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
	m_ListCtrlST.SetInputTypeAsCombo(TAG_SNMBS,CmbBoxData);*/


	m_ListCtrlST.SetInputTypeAsText(TAG_SNSIZE - 3,CListEditST::DATATYPE_NUM,2);
	m_ListCtrlST.SetInputTypeAsText(TAG_SNSTARTADDR - 3,CListEditST::DATATYPE_HEX,8);
	/*m_ListCtrlST.SetInputTypeAsText(TAG_SN_EverCreation_001,CListEditST::DATATYPE_HEX,8);
	m_ListCtrlST.SetInputTypeAsText(TAG_SNSTARTVALUE,CListEditST::DATATYPE_STR,(UINT)-1);*/

	return TRUE;
}

extern void TestComTool(void);
BOOL CDlgSN_EverCreation_001::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化
	m_cmbSNGroup.AddString("1");
	m_cmbSNGroup.AddString("2");
	m_cmbSNGroup.AddString("3");
	m_cmbSNGroup.AddString("4");
	m_cmbSNGroup.AddString("5");
	m_cmbSNGroup.AddString("6");
	m_cmbSNGroup.AddString("7");
	m_cmbSNGroup.AddString("8");
	m_cmbSNGroup.AddString("9");
	m_cmbSNGroup.AddString("10");
	m_cmbSNGroup.AddString("11");
	m_cmbSNGroup.AddString("12");

	InitCtrlList();

	TestComTool();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

BOOL CDlgSN_EverCreation_001::InitCtrlsValue(CSN_EverCreation_001Cfg& SN_EverCreation_001Cfg)
{
	INT i=0,GroupCnt;
	CString strData;
	std::vector<CString>ItemData;
	GroupCnt=SN_EverCreation_001Cfg.GetGroupNum();
	m_cmbSNGroup.SetCurSel(GroupCnt-1);
	m_ListCtrlST.DeleteAllItems();
	for(i=0;i<GroupCnt;++i){
		SNGROUP* pSnGroup=SN_EverCreation_001Cfg.GetGroup(i);
		ItemData.clear();
		/*if(pSnGroup->dwSNMode==SNMODE_BIN)
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
			ItemData.push_back("LSB");*/

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
		/*ItemData.push_back(strData);
		ItemData.push_back(pSnGroup->strSNStartValue);*/
		m_ListCtrlST.AppendItem(ItemData);
	}
	return TRUE;
}

void CDlgSN_EverCreation_001::OnOK()
{
	// TODO: 在此添加专用代码和/或调用基类
	//CDialog::OnOK();
}

BOOL CDlgSN_EverCreation_001::InitCtrls( CSerial& lSerial )
{
	BOOL Ret=TRUE;
	if(lSerial.GetLength()!=0){
		Ret=m_SN_EverCreation_001Cfg.SerialInCfgData(lSerial);
	}
	else{
	}
	Ret=InitCtrlsValue(m_SN_EverCreation_001Cfg);
	return Ret;
}

BOOL CDlgSN_EverCreation_001::GetCtrls( CSerial&lSerial )
{
	return m_SN_EverCreation_001Cfg.SerialOutCfgData(lSerial);
}
void CDlgSN_EverCreation_001::OnCbnSelchangeCombo1()
{
	// TODO: 在此添加控件通知处理程序代码
	INT CurGroup=m_cmbSNGroup.GetCurSel()+1;
	INT GroupCnt=m_SN_EverCreation_001Cfg.GetGroupNum();
	if(CurGroup==GroupCnt)
		return;
	
	if(CurGroup>GroupCnt){
		m_SN_EverCreation_001Cfg.AppendGroup(CurGroup-GroupCnt);
	}
	else{
		m_SN_EverCreation_001Cfg.RemoveGroup(GroupCnt-CurGroup);
	}
	InitCtrlsValue(m_SN_EverCreation_001Cfg);
}

CString GetCurTime(char Seperator)
{
	CString strTime;
	SYSTEMTIME st;
	GetLocalTime(&st);
	strTime.Format("%04d%02d%02d_%02d%c%02d%c%02d_%03d", st.wYear, st.wMonth, st.wDay, st.wHour,
		Seperator, st.wMinute, Seperator, st.wSecond, st.wMilliseconds);
	return strTime;
}

INT CDlgSN_EverCreation_001::RunExcuCmdLine(DWORD nIdx, std::vector<BYTE>& vOut)
{
	int nRetryCnt = 0;
	int nRetryOpenProcCnt = 0;

	int nRtn = -1;
	CString strCurCmd;
	int nFileLen;
	char pFileData[1024] = { 0 };
	
	CFile RetFile;
	CFile BatFile;
	CString strBatPath;
	BYTE pData[1024] = {0};

	CString strMsg;
	CString strTempDir;
	CString strEachCmdLine;

	CString strDay;
	CString strCurTime;
	CString strLogDataPath;
	CString strCurrCmdOutFile;
	CString safeBatPath;

	UINT nTimeOut = 0;
	char* pOutData = NULL;

	int nOutSize = 1024 * 1024 *1;
	pOutData = new char[nOutSize];
	if (pOutData == NULL) {
		strMsg.Format("分配缓存内存失败");
		//m_pILog->PrintLog(LOGLEVEL_LOG, (LPSTR)(LPCTSTR)strMsg);
		goto __end;
	}

__RetryBurn:
	memset(pOutData, 0, nOutSize);
	
	strCurrCmdOutFile.Format("%s\\gene_ret_%d.bin",CComFunc::GetCurrentPath(),nIdx);
	if ((PathFileExists(strCurrCmdOutFile) == TRUE)) {
		DeleteFile(strCurrCmdOutFile);
	}
	////////////////////////////////////创建bat///////////////////////////
	strBatPath.Format("%s\\ac_tmp%d.bat", CComFunc::GetCurrentPath(), nIdx); //C:\\ac_tmp%d.bat
	safeBatPath.Format(_T("\"%s\""), strBatPath);

	if ((PathFileExists(strBatPath) == TRUE)) {
		DeleteFile(strBatPath);
	}

	if (BatFile.Open(strBatPath, CFile::modeCreate | CFile::modeNoTruncate | CFile::modeWrite | CFile::shareDenyNone, NULL) == FALSE) {
		strMsg.Format("创建bat文件失败，请确认路径是否正确，路径=%s, ErrCode=0x%x", strBatPath, GetLastError());
		//m_pILog->PrintLog(LOGLEVEL_LOG, (LPSTR)(LPCTSTR)strMsg);
		DeleteFile(strBatPath);
		goto __end;
	}
	///////////////////////////////////////
	strLogDataPath.Format("%s", CComFunc::GetCurrentPath());

	if (!PathIsDirectory(strLogDataPath)) {
		CreateDirectory(strLogDataPath, 0);//不存在文件夹则创建
	}

	strCurTime.Format("%s", GetCurTime('_'));
	strDay.Format("\\%s", strCurTime.Left(8));
	//strLogDataPath += strDay;

	//if (!PathIsDirectory(strLogDataPath)) {
	//	CreateDirectory(strLogDataPath, 0);//不存在文件夹则创建
	//}

	strDay.Format("\\%s_Log_SKT%d.data", strCurTime, nIdx );
	strLogDataPath += strDay;
	if ((PathFileExists(strLogDataPath) == TRUE)) {
		DeleteFile(strLogDataPath);
	}
	////////////////////////////////////////////////
	strCurCmd.Empty();

	//strTempDir.Format("%s\\OrgSoftWare\\SLM925\\bin%d", CComFunc::GetCurrentPath(), nIdx);
	strTempDir.Format("%s", CComFunc::GetCurrentPath()); //espsecure.exe的所在文件夹

	//strEachCmdLine.Format("espsecure.exe generate_flash_encryption_key --keylen 128 -l 256 my_aes_encryption_key.bin")

	strEachCmdLine.Format("cd /d %s && espsecure.exe generate_flash_encryption_key --keylen 128 -l 256 %s >> %s\r\n", 
		strTempDir , strCurrCmdOutFile, strLogDataPath);
	strCurCmd += strEachCmdLine;

	/*strCurCmd.Replace("(", "^(");
	strCurCmd.Replace(")", "^)");*/
	strCurCmd.Replace(_T("("), _T("^("));
	strCurCmd.Replace(_T(")"), _T("^)"));

	//strMsg.Format("Site:[%s]cmd指令内容%s", strDevSN, strCurCmd);
	//m_pILog->PrintLog(LOGLEVEL_LOG, (LPSTR)(LPCTSTR)strMsg);

	strMsg.Format("=5555==:>cmddata:%s", strCurCmd);
	OutputDebugString(strMsg);
	//////////////////////内容写入bat////////////
	nFileLen = strCurCmd.GetLength();
	memcpy(pFileData, (LPSTR)(LPCSTR)strCurCmd, nFileLen);
	BatFile.Write(pFileData, nFileLen);
	BatFile.Close();
	///////////////////////////////////////////////////////////////////////
	int nRetryPipe = 0;
	SECURITY_ATTRIBUTES sa; // 此结构体包含一个对象的安全描述符，并指定检索到制定这个结构的句柄是否是可继承的
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = NULL; // 安全描述符
	sa.bInheritHandle = TRUE; // 安全描述的对象能否被创建的进程继承，TRUE表示能被继承

	HANDLE h_read = INVALID_HANDLE_VALUE;
	HANDLE h_write = INVALID_HANDLE_VALUE;

__RetryOpenProc:

	if (h_read != INVALID_HANDLE_VALUE) {
		CloseHandle(h_read);
		h_read = INVALID_HANDLE_VALUE;
	}
	if (h_write != INVALID_HANDLE_VALUE) {
		CloseHandle(h_write);
		h_write = INVALID_HANDLE_VALUE;
	}

	if (!CreatePipe(&h_read, &h_write, &sa, 0)) {
		//strMsg.Format("Site:[%s][%d]创建管道失败", strDevSN, skt+1);
		//m_pILog->PrintLog(LOGLEVEL_LOG, (LPSTR)(LPCTSTR)strMsg);
		nRetryPipe++;

		if (nRetryPipe <= 10)
		{
			Sleep(200);
			goto __RetryOpenProc;
		}
		else {
			goto __end;
		}
	}

	nRetryPipe = 0;
	STARTUPINFO si/* = { sizeof(STARTUPINFO) }*/; // 此结构体用于指定新进程的主窗口特性 //si.cb = sizeof(STARTUPINFO);
	/*GetStartupInfo(&si);*/
	memset(&si, 0, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);
	si.wShowWindow = SW_HIDE; // 窗口设为隐藏
	si.hStdError = NULL;     //h_write; // 标识控制台窗口的缓存，可指定管道
	si.hStdOutput = h_write; // 同上
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;// 使用wShowWindow成员，使用hStdInput、hStdOutput、hStdError成员

	PROCESS_INFORMATION pi; // 此结构返回有关新进程及其主线程的信息
	/*pi = { 0 };*/
	memset(&pi, 0, sizeof(pi));
	OutputDebugString(strMsg);
	if (!CreateProcess((LPSTR)(LPCSTR)NULL,
		(LPSTR)(LPCSTR)/*strCurCmd*/safeBatPath.GetBuffer(),
		NULL, NULL, TRUE, NULL, NULL, NULL, &si, &pi)) {
			//strMsg.Format("Site:[%s]执行指令失败，指令内容%s, nIdx:%d, LastError:0x%X", strDevSN, strCurCmd, nIdx, GetLastError());
			//m_pILog->PrintLog(LOGLEVEL_LOG, (LPSTR)(LPCTSTR)strMsg);
			safeBatPath.ReleaseBuffer();
			if (nRetryOpenProcCnt > 10){
				nRtn = -1;
				goto __end;
			}
			else {
				nRetryOpenProcCnt++;
				Sleep(500);
				goto __RetryOpenProc;
			}
	}
	CloseHandle(h_write);
	safeBatPath.ReleaseBuffer();
	char buffer[1024] = { 0 };
	DWORD i = 0;

	strMsg.Format("=5555==:>cmd WaitForSingleObject");
	OutputDebugString(strMsg);

	int nWait = WaitForSingleObject(pi.hProcess,6*1000);///等待进程结束

	if (WAIT_TIMEOUT == nWait) {
		TerminateProcess(pi.hProcess, 1); // 终止挂起的进程
		//m_pILog->PrintLog(LOGLEVEL_LOG, "=====>[%s]烧录超时", strDevSN);
	}

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	CloseHandle(h_read);

	if ((PathFileExists(strLogDataPath) == FALSE)) { 
		//strMsg.Format("Site:[%s]日志生成失败", strDevSN);
		//m_pILog->PrintLog(LOGLEVEL_ERR, (LPSTR)(LPCTSTR)strMsg);
		goto __end;
	}
	////////////////////////////////////////////////////////////////////
	if (RetFile.Open(strCurrCmdOutFile, CFile::modeRead |CFile::typeBinary| CFile::shareExclusive, NULL) == FALSE) {
		//m_pILog->PrintLog(LOGLEVEL_ERR, "打开ACprint.prn文件失败，请确认是否已被占用");
		goto __end;
	}

	int nLen = RetFile.GetLength();
	if (nLen > 1024 || nLen <=0 ){
		goto __end;
	}

	RetFile.Read(pData, nLen);
	RetFile.Close();

	vOut.assign(pData, pData+nLen);
	////////////////////////////////////////////////////////////////////

__end:

	if (pOutData) {
		delete[] pOutData;
		pOutData = NULL;
	}

	if (BatFile.m_hFile != CFile::hFileNull) {
		BatFile.Close();
	}

	if (RetFile.m_hFile != CFile::hFileNull) {
		RetFile.Close();
	}

	if ((PathFileExists(strLogDataPath) == TRUE)) {
		DeleteFile(strLogDataPath);
	}

	if ((PathFileExists(strBatPath) == TRUE)) {
		DeleteFile(strBatPath);
	}

	return  nRtn;
}

INT CDlgSN_EverCreation_001::QuerySN(DWORD Idx,BYTE*pData,INT*pSize)
{
	INT Ret=0;
	CSerial lSerial;
	BYTE*pSNData=NULL;
	INT i,j,GroupCnt,SNSize,PreSNSize,BytesFillZero;
	std::vector<BYTE>vMulData;
	std::vector<BYTE>vBaseValue;
	std::vector<BYTE>vSNValue;
	Idx=Idx-1;
	GroupCnt=m_SN_EverCreation_001Cfg.GetGroupNum();
	lSerial<<GroupCnt;

	EnterCriticalSection(&m_csQuery);
	
	for(i=0;i<GroupCnt;++i){
		SNGROUP* pSnGroup=m_SN_EverCreation_001Cfg.GetGroup(i);
		lSerial<<pSnGroup->llSNStartAddr<<pSnGroup->dwSNLen;
		vMulData.clear();
		vBaseValue.clear();
		vSNValue.clear();
		/*ComTool::MultiBytesMul(ComTool::ENDIAN_LIT,(BYTE*)&Idx,sizeof(DWORD),(BYTE*)&pSnGroup->dwStep,sizeof(UINT),vMulData);
		if(pSnGroup->dwSNStyle==SNSTYLE_HEX){
			ComTool::Str2Hex(pSnGroup->strSNStartValue,ComTool::ENDIAN_LIT,vBaseValue);
		}
		else{
			ComTool::Str2Dec(pSnGroup->strSNStartValue,ComTool::ENDIAN_LIT,vBaseValue);
		}
		ComTool::MultiBytesAdd(ComTool::ENDIAN_LIT,vBaseValue,vMulData,vSNValue);*/

		///////////////////////
		/*if(pSnGroup->dwSNStyle==SNSTYLE_BCD){///计算结果要当10进制看也就是0x0A要被转为0x10
			CString strDEC;
			ComTool::Dec2Str(ComTool::ENDIAN_LIT,vSNValue,strDEC);
			ComTool::Str2Hex(strDEC,ComTool::ENDIAN_LIT,vSNValue);
		}

		BytesFillZero=pSnGroup->dwSNLen>(INT)vSNValue.size()?(pSnGroup->dwSNLen-(INT)vSNValue.size()):0;
		for(j=0;j<BytesFillZero;j++){///计算结果还未达到有效字节则高位补充0
			vSNValue.push_back((BYTE)0);
		}

		SNSize=pSnGroup->dwSNLen;///只取这么多字节
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
		if(pSnGroup->dwSNMode==SNMODE_BIN){///按照16进制解析结果
			if(pSnGroup->dwSNMSB==SNMSB_MSB){
				for(j=0;j<SNSize;j++){///转为大端模式存放
					pSNData[j]=vSNValue[SNSize-1-j];
				}
			}
			else{
				for(j=0;j<SNSize;j++){///小端存放
					pSNData[j]=vSNValue[j];
				}
			}
		}
		else{
			INT strLen;
			CString strSNValue;
			ComTool::Hex2Str(ComTool::ENDIAN_LIT,vSNValue,strSNValue);///获取计算结果的字符串
			strLen=strSNValue.GetLength();
			if(strLen<SNSize){
				for(j=SNSize-strLen;j>0;j--){///前面插入0
					strSNValue.Insert(-1,'0');
				}
				strLen=SNSize;
			}
			if(pSnGroup->dwSNMSB==SNMSB_MSB){
				for(j=0;j<SNSize;j++){///大端存放
					pSNData[j]=strSNValue.GetAt(j);
				}
			}
			else{
				for(j=0;j<SNSize;j++){///按照小端存放
					pSNData[j]=strSNValue.GetAt(strLen-1-j);
				}
			}
		}*/
		/////////////////////////////////////////
		if (true)
		{
			std::vector<BYTE> vData;
			RunExcuCmdLine(Idx, vData);
			lSerial.SerialInBuff(&vData[0], vData.size());
		}
		
		//////////////////////////////////////////
		//lSerial.SerialInBuff(pSNData,SNSize);
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

	LeaveCriticalSection(&m_csQuery);

	if(pSNData){
		delete[] pSNData;
	}
	return Ret;
}
