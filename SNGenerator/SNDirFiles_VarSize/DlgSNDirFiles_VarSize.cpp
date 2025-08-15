// DlgSNDirFiles_VarSize.cpp : 实现文件
//

#include "stdafx.h"
#include "SNDirFiles_VarSize.h"
#include "DlgSNDirFiles_VarSize.h"
#include "../Com/ComFunc.h"
#include "../Com/cJSON.h"

enum{
	TAG_SNAddr,
	TAG_SNSize,
	TAG_SNSeparateFile,
	TAG_SNExampleFile,
	TAG_SNFilesDir,
};

// CDlgSNDirFiles_VarSize 对话框

IMPLEMENT_DYNAMIC(CDlgSNDirFiles_VarSize, CDialog)




CDlgSNDirFiles_VarSize::CDlgSNDirFiles_VarSize(DRVSNCFGPARA *pSnCfgPara,CWnd* pParent /*=NULL*/)
	: CDialog(CDlgSNDirFiles_VarSize::IDD, pParent)
	, m_pSnCfgPara(pSnCfgPara)
	, m_SNTotalSize(0)
	, m_PassFileID(0)
{

}

CDlgSNDirFiles_VarSize::~CDlgSNDirFiles_VarSize()
{
	
}

extern CSNDirFiles_VarSizeApp theApp;
CString DllGetCurrentPath( void )
{
	TCHAR szFilePath[MAX_PATH + 1]; 
	TCHAR *pPos=NULL;
	CString str_url;
	GetModuleFileName(theApp.m_hInstance, szFilePath, MAX_PATH); 
	pPos=_tcsrchr(szFilePath, _T('\\'));
	if(pPos!=NULL){
		pPos[0] = 0;//删除文件名，只获得路径
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

BOOL CDlgSNDirFiles_VarSize::InitCtrls(CSerial& lSerial)
{
	BOOL Ret=TRUE;
	if(lSerial.GetLength()!=0){
		Ret=m_SnCfg.SerialInCfgData(lSerial);
	}
	else{
	}
	Ret=InitCtrlsValue(m_SnCfg);
	return Ret;
}

BOOL CDlgSNDirFiles_VarSize::GetCtrls(CSerial&lSerial)
{
	//UpdateData(TRUE);
	UINT i,j,MaxSize=0,Length;
	CString strSampleBinExtName;
	for(i=0;i<m_SnCfg.GetGroupNum();++i){
		m_SnCfg.GetGroup(i)->strMaxSize=m_strSNMaxSize;
		m_SnCfg.GetGroup(i)->strFillData = m_strFillData;
		strSampleBinExtName=CComFunc::GetFileExt(m_SnCfg.GetGroup(i)->strSampleBin);
		m_vFileSearcher[i].ScanFiles(m_SnCfg.GetGroup(i)->strSNDir,m_SnCfg.GetGroup(i)->dwSNLen,strSampleBinExtName,FALSE);//查询
		m_vFileSearcher[i].Sort();
		//Ret = m_vFileSearcher[i].GetListSize();
		for(j=0;j<m_vFileSearcher[i].GetListSize();++j){
			CString str=m_vFileSearcher[i].GetIdx(j);
			GetFileLength(m_vFileSearcher[i].GetIdx(j),Length);
			if (!m_strSNMaxSize.IsEmpty()){
				sscanf(m_strSNMaxSize,"%X",&MaxSize);
				if (Length > MaxSize){
					AfxMessageBox("GetCtrls error! Please Check The SN File Maximum Size And Fill The Size To SNMaxSize!");
					return FALSE;
				}
			}
		}
	}
	return m_SnCfg.SerialOutCfgData(lSerial);
}


INT CDlgSNDirFiles_VarSize::QuerySN(DWORD Idx,BYTE*pData,INT*pSize)
{
	INT Ret=0,MaxSize,FillData;
	CString strMsg;
	CSerial lSerial;
	BYTE*pTmpData=NULL;
	UINT Sum=0;
	UINT Length = 0;
	INT i,Cnt=m_SnCfg.GetGroupNum();

	if(m_SNTotalSize==0){
		strMsg.Format("QuerySN: SN数据大小为0，请确认\r\n");
		WriteLog(strMsg);
		Ret=-1; goto __end;
	}
	if(*pSize<m_SNTotalSize){///外部传入的空间太小，需要重新分配
		*pSize=m_SNTotalSize;///返回实际需要的大小
		Ret=-2; goto __end;
	}
	lSerial<<Cnt;
	for(i=0;i<Cnt;++i){
		SNGROUP *pGroup=m_SnCfg.GetGroup(i);
		CString FilePath=m_vFileSearcher[i].GetIdx(Idx-1);
		if(FilePath==""){
			strMsg.Format("QuerySN: 获取文件路径失败, ProgramIdx为%d 大于最大文件数%d\r\n",Idx,m_vFileSearcher[i].GetListSize());
			WriteLog(strMsg);
			Ret=-1; goto __end;
		}
		///序列化写入位置和长度
		lSerial<<pGroup->llSNStartAddr;
		if (m_strSNMaxSize.IsEmpty()){
			lSerial<<pGroup->dwSNLen;
			strMsg.Format("SNQuery: ProgramIdx=%d,写入地址为0x%I64X,长度为0x%X\r\n",Idx,pGroup->llSNStartAddr,pGroup->dwSNLen);
			WriteLog(strMsg);
		}else{
			sscanf(m_strSNMaxSize,"%X",&MaxSize);
			lSerial<<MaxSize;
			strMsg.Format("SNQuery: ProgramIdx=%d,写入地址为0x%I64X,长度为0x%X\r\n",Idx,pGroup->llSNStartAddr,MaxSize);
			WriteLog(strMsg);
		}

		if (m_strSNMaxSize.IsEmpty()){
			pTmpData=new BYTE[pGroup->dwSNLen];
		}else{

			if (pGroup->dwSNLen <= (UINT)MaxSize){
				pTmpData=new BYTE[MaxSize];
			}
			
			if (m_strFillData.IsEmpty()){
				memset(pTmpData,0xFF,MaxSize);
			}else{
				sscanf(m_strFillData,"%X",&FillData);
				memset(pTmpData,FillData,MaxSize);
			}
			
		}

		if(pTmpData){
			CFile File;
			Sum=0;
			if(File.Open(FilePath,CFile::modeRead,NULL)==FALSE){
				strMsg.Format("QuerySN: 打开文件失败, 文件名:%s\r\n",FilePath);
				WriteLog(strMsg);
				Ret=-1; goto __end;
			}

			if (m_strSNMaxSize.IsEmpty()){
				if(File.GetLength()!=pGroup->dwSNLen){
					strMsg.Format("QuerySN: 文件名:%s 长度不为0x%X,请确认\r\n",FilePath,pGroup->dwSNLen);
					WriteLog(strMsg);
					Ret=-1; goto __end;
				}
			}
			
			if(File.Read(pTmpData,File.GetLength())!=File.GetLength()){
				strMsg.Format("QuerySN: 读取文件%s 失败\r\n",FilePath);
				WriteLog(strMsg);
				Ret=-1; goto __end;
			}
			///序列化实际写入数据
			if (m_strSNMaxSize.IsEmpty()){
				lSerial.SerialInBuff(pTmpData,pGroup->dwSNLen);
				for(int j=0;j<pGroup->dwSNLen;++j){
					Sum+=pTmpData[j];
				}
				strMsg.Format("Sum值:0x%08X,文件名:%s\r\n",Sum,FilePath);
				WriteLog(strMsg);
			}else{
				lSerial.SerialInBuff(pTmpData,MaxSize);
				for(int j=0;j<MaxSize;++j){
					Sum+=pTmpData[j];
				}
				strMsg.Format("Sum值:0x%08X,文件名:%s\r\n",Sum,FilePath);
				WriteLog(strMsg);
			}

			delete[] pTmpData;
			pTmpData=NULL;
		}
	}

	if((INT)lSerial.GetLength()>*pSize){
		*pSize=lSerial.GetLength();///返回实际需要的大小 
		Ret=-2; goto __end;
	}
	else{
		memcpy(pData,lSerial.GetBuffer(),lSerial.GetLength());
		Ret=lSerial.GetLength();
	}

__end:
	if(pTmpData){
		delete[] pTmpData;
	}
	return Ret;
}
INT CDlgSNDirFiles_VarSize::RecordPassResult(CString FilePath)
{
	CString strFileName,strMsg;
	m_PassFileID++;
	SYSTEMTIME st;
	CString strTime;
	GetLocalTime(&st);
	strTime.Format("%4d%2d%2d-%2d%2d%2d.%3d",st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond,st.wMilliseconds);

	strMsg.Format("%05d\t%s\t%s\r\n",m_PassFileID,FilePath,strTime);
	WriteResult(strMsg);

	return 0;
}

INT CDlgSNDirFiles_VarSize::TellResult(DWORD Idx,INT IsPass)
{
	CString strMsg;
	BOOL Ret=TRUE;
	INT i,Cnt=m_SnCfg.GetGroupNum();

	strMsg.Format("TellResult: ProgramIdx=%d,IsPass=%s\r\n",Idx,IsPass?"TRUE":"FALSE");
	WriteLog(strMsg);
	if(IsPass==1 && Idx>=1){
		for(i=0;i<Cnt;++i){
			SNGROUP *pGroup=m_SnCfg.GetGroup(i);
			if(pGroup->strSeperated=="Yes"){
				CString FilePath=m_vFileSearcher[i].GetIdx(Idx-1);
				Ret=MoveFileToPass(FilePath,pGroup->strPassDir);
				if(Ret==FALSE){
					strMsg.Format("TellResult:移动文件失败,文件名:%s\r\n",FilePath);
					WriteLog(strMsg);			
				}
				else{
					strMsg.Format("TellResult:移动文件成功,文件名:%s\r\n",FilePath);
					WriteLog(strMsg);
					OpenResultLog(pGroup->strPassDir);
					RecordPassResult(FilePath);
				}
			}
			else if(pGroup->strSeperated=="Delete"){
				CString FilePath=m_vFileSearcher[i].GetIdx(Idx-1);
				Ret=DeleteFile(FilePath);
				if(Ret==FALSE){
					strMsg.Format("TellResult:删除文件失败,文件名:%s\r\n",FilePath);
					WriteLog(strMsg);
				}
				else{
					strMsg.Format("TellResult:删除文件成功,文件名:%s\r\n",FilePath);
					WriteLog(strMsg);
				}
			}
		}
	}
	return 0;
}

INT CDlgSNDirFiles_VarSize::OpenLog()
{
	CString strFile;
	CTime Time=CTime::GetCurrentTime();
	CString LogTime=Time.Format("%H-%M-%S");

	strFile.Format("%s\\..\\log\\SNDirFiles_VarSizeLog_%s.txt",DllGetCurrentPath(),LogTime);
	if(m_logFile.Open(strFile,CFile::modeCreate|CFile::modeWrite|CFile::shareDenyNone,NULL)==FALSE){
		return -1;
	}
	return 0;
}

INT CDlgSNDirFiles_VarSize::OpenResultLog(CString strPassDir)
{
	CString strFileName,strMsg;
	strFileName.Format("%s\\ProgramPassResult.txt",strPassDir);
	if(m_FilePassResult.m_hFile!=CFile::hFileNull){
		return 0;
	}
	if(m_FilePassResult.Open(strFileName,CFile::modeCreate|CFile::modeWrite|CFile::shareDenyNone,NULL)==FALSE){
		strMsg.Format("打开ProgramPassResult.txt失败，路径:%s",strFileName);
		WriteLog(strMsg);
	}
	return 0;
}

void CDlgSNDirFiles_VarSize::WriteLog(CString str)
{
	if(m_logFile.m_hFile!=CFile::hFileNull){
		m_logFile.WriteString(str);
		m_logFile.Flush();
	}
}

void CDlgSNDirFiles_VarSize::WriteResult(CString str)
{
	if(m_FilePassResult.m_hFile!=CFile::hFileNull){
		m_FilePassResult.WriteString(str);
		m_FilePassResult.Flush();
	}
}

void CDlgSNDirFiles_VarSize::CloseLog()
{
	if(m_logFile.m_hFile!=CFile::hFileNull){
		m_logFile.Flush();
		m_logFile.Close();
	}
}

void CDlgSNDirFiles_VarSize::CloseResult()
{
	if(m_FilePassResult.m_hFile!=CFile::hFileNull){
		m_FilePassResult.Flush();
		m_FilePassResult.Close();
	}
}

INT CDlgSNDirFiles_VarSize::CreatePassDir( const CString& DirName)
{
	INT Ret=0;
	CString strMsg;
	if(PathIsDirectory(DirName)==FALSE){//判断路径是否存在,存在则不再创建，否则要进行创建  
		if(CreateDirectory(DirName,NULL)==FALSE){
			UINT ErrCode=GetLastError();
			if(ErrCode==ERROR_ALREADY_EXISTS){
				Ret=0;
			}
			else{
				strMsg.Format("Create Pass Dir Failed,DirName:%s,ErrCode=%d\r\n",DirName,ErrCode);
				WriteLog(strMsg);
				Ret=-1;
			}
		}
	}
	
	return Ret;
}

BOOL  CDlgSNDirFiles_VarSize::MoveFileToPass(const CString&File,CString& DestDir)
{
	CString FileName=CComFunc::GetFileNameWithSuffix(File);
	CString DestFile;
	//CString LogMsg;
	DestFile.Format("%s\\%s",DestDir,FileName);
	//LogMsg.Format("原始文件:%s\r\n",File);
	//WriteLog(LogMsg);
	//LogMsg.Format("目标文件:%s\r\n",DestFile);
	//WriteLog(LogMsg);
	return MoveFile(File,DestFile);
	//DeleteFile(File);
}

INT CDlgSNDirFiles_VarSize::PreLoad()
{
	INT Ret=0,i,j,MaxSize =0;
	CString strMsg;
	CString strFileName;
	CString strSampleBinExtName="";
	CStdioFile FileListTmpFile;
	UINT Length = 0;
	m_SNTotalSize=0;

	UpdateData(TRUE);
	Ret=OpenLog();
	if(Ret!=0){
		Ret=-1; goto __end;
	}
	if(m_SnCfg.GetGroupNum()>GROUPCNT_MAX){
		strMsg.Format("错误:SNDirFiles_VarSize模式最大支持%d组,当前选择了%d组请修改对应的设置\r\n",GROUPCNT_MAX,m_SnCfg.GetGroupNum());
		WriteLog(strMsg);
		Ret=-1; goto __end;
	}
	for(i=0;i<m_SnCfg.GetGroupNum();++i){
		SNGROUP* pGroup=m_SnCfg.GetGroup(i);
		/*pGroup->strMaxSize =m_strSNMaxSize;
		pGroup->strFillData = m_strFillData;*/
		if(pGroup->strSNDir!=""){
			strFileName.Format("%s\\SNFileList.txt",pGroup->strSNDir);
			if(FileListTmpFile.Open(strFileName,CFile::modeCreate|CFile::modeWrite,NULL)==FALSE){
				Ret=-1;
				strMsg.Format("打开SNFileList.txt失败，路径:%s",strFileName);
				WriteLog(strMsg);
				goto __end;
			}
			strSampleBinExtName=CComFunc::GetFileExt(pGroup->strSampleBin);
			if(strSampleBinExtName==""){
				Ret=-1;
				WriteLog("获取SampleBin的后缀名失败\r\n");
				goto __end;
			}
			if(pGroup->strSeperated=="Yes"){///需要创建Pass文件夹
				pGroup->strPassDir.Format("%s\\Pass",pGroup->strSNDir);
				Ret=CreatePassDir(pGroup->strPassDir);
				if(Ret!=0){
					WriteLog("创建Pass文件夹失败\r\n");
					goto __end;
				}
			}
			m_vFileSearcher[i].ScanFiles(pGroup->strSNDir,pGroup->dwSNLen,strSampleBinExtName,FALSE);//查询
			m_vFileSearcher[i].Sort();
			//Ret = m_vFileSearcher[i].GetListSize();
			for(j=0;j<m_vFileSearcher[i].GetListSize();++j){
				CString str=m_vFileSearcher[i].GetIdx(j);
				GetFileLength(m_vFileSearcher[i].GetIdx(j),Length);
				if (!m_strSNMaxSize.IsEmpty()){
					sscanf(m_strSNMaxSize,"%X",&MaxSize);
					if (Length > MaxSize){
						Ret=-1;
						strMsg.Format("档案长度最大值为0x%X,文件:%s 长度为0x%X\r\n",MaxSize,str,Length);
						WriteLog(strMsg);
						goto __end;
					}
				}
				strMsg.Format("%05d,%s\r\n",j+1,str);
				FileListTmpFile.WriteString(strMsg);
				if (Length >MaxSize){
					MaxSize = Length;
				}
			}
			m_strSNMaxSize.Format("%X",MaxSize);
			FileListTmpFile.Close();
		}
	}
	
	m_SNTotalSize=m_SnCfg.GetSNTotalSize();
	//UpdateData(FALSE);
__end:
	if(FileListTmpFile.m_hFile!=CFile::hFileNull){
		FileListTmpFile.Close();
	}
	if(Ret>=0){
		WriteLog("PreLoad成功\r\n");
	}
	else{
		WriteLog("PreLoad失败\r\n");
	}
	
	return Ret;
}

void CDlgSNDirFiles_VarSize::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_ListCtrlST);
	DDX_Control(pDX, IDC_COMBO1, m_cmbSNGroup);
	DDX_Text(pDX, IDC_EDIT_MAXSIZE, m_strSNMaxSize);
	DDX_Text(pDX, IDC_EDIT_DATA, m_strFillData);
}


BEGIN_MESSAGE_MAP(CDlgSNDirFiles_VarSize, CDialog)
	ON_CBN_SELCHANGE(IDC_COMBO1, &CDlgSNDirFiles_VarSize::OnCbnSelchangeCombo1)
	//ON_BN_CLICKED(IDC_CHECK1, &CDlgSNDirFiles_VarSize::OnBnClickedCheck1)
	ON_BN_CLICKED(IDC_BUTTON1, &CDlgSNDirFiles_VarSize::OnBnClickedButton1)
	ON_EN_KILLFOCUS(IDC_EDIT_DATA, &CDlgSNDirFiles_VarSize::OnEnKillfocusEditData)
END_MESSAGE_MAP()

BOOL CDlgSNDirFiles_VarSize::GetFileLength(CString strFile,UINT& Length)
{
	CFile File;
	CString strErrMsg;
	if(File.Open(strFile,CFile::modeRead,NULL)==FALSE){
		strErrMsg.Format("Open %s Failed",strFile);
		MessageBox(strErrMsg);
		return FALSE;
	}

	Length=(UINT)File.GetLength();
	File.Close();

	return TRUE;
}

LRESULT CDlgSNDirFiles_VarSize::SetText(INT nRow,INT nColumn,CString&strText)
{
	if(strText.IsEmpty()){
		CString strErrMsg;
		strErrMsg.Format("SN Error: %s can't be empty",vHeader[nColumn]);
		MessageBox(strErrMsg);
		return -1;
	}
	SNGROUP* pSnGroup=m_SnCfg.GetGroup(nRow);
	if(pSnGroup==NULL){
		return 0;
	}
	UpdateData(TRUE);
	switch(nColumn){
		case TAG_SNSize:///不直接来源于输入
			//sscanf(strText,"%X",&pSnGroup->dwSNLen);
			break;
		case TAG_SNAddr:
			sscanf(strText,"%I64X",&pSnGroup->llSNStartAddr);
			break;
		case TAG_SNExampleFile:
			{
				UINT Length=0; 
				CString strSNLen;
				pSnGroup->strSampleBin=strText;
				GetFileLength(strText,Length);
				pSnGroup->dwSNLen=Length;
				strSNLen.Format("%X",Length);
				m_ListCtrlST.SetItemText(nRow,TAG_SNSize,strSNLen);
				
			}
			break;
		case TAG_SNFilesDir:
			pSnGroup->strSNDir=strText;
			break;
		case TAG_SNSeparateFile:
			pSnGroup->strSeperated=strText;
			break;
		default:
			break;
	}
	return 0;
}

LRESULT CALLBACK DlgSNSetText(void *Para,INT nRow,INT nColumn,CString&strText)
{
	CDlgSNDirFiles_VarSize*pDlgSN=(CDlgSNDirFiles_VarSize*)Para;
	if(pDlgSN){
		return pDlgSN->SetText(nRow,nColumn,strText);
	}
	return 0;
}


// CDlgSNDirFiles_VarSize 消息处理程序

BOOL CDlgSNDirFiles_VarSize::InitCtrlList()
{
	DWORD Style=m_ListCtrlST.GetStyle();
	Style |=LVS_EX_GRIDLINES;
	m_ListCtrlST.SetExtendedStyle(Style);
	m_ListCtrlST.RegistSetTextCallBack(DlgSNSetText,this);///注册数据改变回调函数
	m_ListCtrlST.RegistGetTextInfoCallBack(NULL,this);
	///设置有方格线

	m_ListCtrlST.SetHeight(0);///根据字体自己调整

	vHeader.clear();
	vHeader.push_back("Start Address(h)");
	vHeader.push_back("Size(h)");
	vHeader.push_back("Separate Files When Pass");
	vHeader.push_back("SN Example File");
	vHeader.push_back("SN Files Directory");
	m_ListCtrlST.InitColumnHeader(vHeader);

	std::vector<CString>CmbBoxData;
	CmbBoxData.push_back("Yes");
	CmbBoxData.push_back("No");
	CmbBoxData.push_back("Delete");
	m_ListCtrlST.SetInputTypeAsCombo(TAG_SNSeparateFile,CmbBoxData);

	m_ListCtrlST.SetInputTypeAsFileSelector(TAG_SNExampleFile,"Binary(*.bin)|*.bin|Data File(*.dat)|*.dat|All File(*.*)|*.*|");

	m_ListCtrlST.SetInputTypeAsFolderSelector(TAG_SNFilesDir);

	//m_ListCtrlST.SetInputTypeAsText(TAG_SNSize,CListEditST::DATATYPE_HEX,2);
	m_ListCtrlST.SetInputTypeAsText(TAG_SNAddr,CListEditST::DATATYPE_HEX,16);
	return TRUE;
}

BOOL CDlgSNDirFiles_VarSize::InitCtrlsValue(CSNDirFiles_VarSizeCfg& SNCfg)
{
	INT i=0,GroupCnt;
	CString strData;
	std::vector<CString>ItemData;
	GroupCnt=SNCfg.GetGroupNum();
	m_cmbSNGroup.SetCurSel(GroupCnt-1);
	m_ListCtrlST.DeleteAllItems();
	for(i=0;i<GroupCnt;++i){
		SNGROUP* pSnGroup=SNCfg.GetGroup(i);
		ItemData.clear();
		strData.Format("%I64X",pSnGroup->llSNStartAddr);
		ItemData.push_back(strData);
		strData.Format("%X",pSnGroup->dwSNLen);
		ItemData.push_back(strData);
		ItemData.push_back(pSnGroup->strSeperated);
		ItemData.push_back(pSnGroup->strSampleBin);
		ItemData.push_back(pSnGroup->strSNDir);
		m_ListCtrlST.AppendItem(ItemData);
		m_strFillData = pSnGroup->strFillData;
		m_strSNMaxSize = pSnGroup->strMaxSize;
	}
	
	UpdateData(FALSE);
	return TRUE;
}

BOOL CDlgSNDirFiles_VarSize::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化
	int i;
	CString strTmp;
	for(i=0;i<10;++i){
		strTmp.Format("%d",i+1);
		m_cmbSNGroup.AddString(strTmp);
	}
	InitCtrlList();
	CEdit *e_Port = (CEdit*)this->GetDlgItem(IDC_EDIT_DATA);
	e_Port->SetLimitText(2);
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CDlgSNDirFiles_VarSize::OnCbnSelchangeCombo1()
{
	// TODO: 在此添加控件通知处理程序代码
	INT CurGroup=m_cmbSNGroup.GetCurSel()+1;
	INT GroupCnt=m_SnCfg.GetGroupNum();
	if(CurGroup==GroupCnt)
		return;

	if(CurGroup>GroupCnt){
		m_SnCfg.AppendGroup(CurGroup-GroupCnt);
	}
	else{
		m_SnCfg.RemoveGroup(GroupCnt-CurGroup);
	}
	InitCtrlsValue(m_SnCfg);
}

INT CDlgSNDirFiles_VarSize::GetSNFeature(const char*strJsonIn, char* strJsonOut, INT SNSize)
{

	INT CountTemp = 0,CountIndex = 0,CountEnd = 0,CountTempTwo = 0;
	INT SNCountCanBeUse;
	std::string strBuildJson;

	UINT i,MaxSize=0;
	CString strSampleBinExtName;

	for(i=0;i<m_SnCfg.GetGroupNum();++i){
		SNGROUP* pGroup=m_SnCfg.GetGroup(i);
		if(pGroup->strSNDir!=""){
			strSampleBinExtName=CComFunc::GetFileExt(pGroup->strSampleBin);
			if(strSampleBinExtName==""){
				WriteLog("获取SampleBin的后缀名失败\r\n");
			}
			m_vFileSearcher[i].ScanFiles(pGroup->strSNDir,pGroup->dwSNLen,strSampleBinExtName,FALSE);
			m_vFileSearcher[i].Sort();
			SNCountCanBeUse = m_vFileSearcher[i].GetListSize();
		}
	}
	cJSON* RootBuild = cJSON_CreateObject();
	cJSON_AddNumberToObject(RootBuild, "SNCountCanBeUse", SNCountCanBeUse);

	strBuildJson = cJSON_Print(RootBuild);
	if(strBuildJson.size() < SNSize){
		memset(strJsonOut, 0, SNSize);
		strncpy(strJsonOut, strBuildJson.c_str(), strBuildJson.size());
	}else {return -1;}

	return 0;	
}
void CDlgSNDirFiles_VarSize::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码

	UINT i,j,MaxSize=0,Length;
	CString strSampleBinExtName;

	for(i=0;i<m_SnCfg.GetGroupNum();++i){
		SNGROUP* pGroup=m_SnCfg.GetGroup(i);
		if (pGroup->strSNDir.IsEmpty()||pGroup->strSampleBin.IsEmpty()){
			AfxMessageBox("Please Set  Example File And Files Directory First");
		}
		if(pGroup->strSNDir!=""){
			strSampleBinExtName=CComFunc::GetFileExt(pGroup->strSampleBin);
			if(strSampleBinExtName==""){
				WriteLog("获取SampleBin的后缀名失败\r\n");
			}
			m_vFileSearcher[i].ScanFiles(pGroup->strSNDir,pGroup->dwSNLen,strSampleBinExtName,FALSE);
			m_vFileSearcher[i].Sort();
			for(j=0;j<m_vFileSearcher[i].GetListSize();++j){
				CString str=m_vFileSearcher[i].GetIdx(j);
				GetFileLength(m_vFileSearcher[i].GetIdx(j),Length);
				if (Length >MaxSize){
					MaxSize = Length;
				}
			}
			m_strSNMaxSize.Format("%X",MaxSize);
			UpdateData(FALSE);
		}
	}
}

void CDlgSNDirFiles_VarSize::OnEnKillfocusEditData()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
}
