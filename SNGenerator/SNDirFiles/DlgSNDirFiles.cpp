// DlgSNDirFiles.cpp : 实现文件
//

#include "stdafx.h"
#include "SNDirFiles.h"
#include "DlgSNDirFiles.h"
#include "../Com/ComFunc.h"

enum{
	TAG_SNAddr,
	TAG_SNSize,
	TAG_SNSeparateFile,
	TAG_SNExampleFile,
	TAG_SNFilesDir,
};

// CDlgSNDirFiles 对话框

IMPLEMENT_DYNAMIC(CDlgSNDirFiles, CDialog)




CDlgSNDirFiles::CDlgSNDirFiles(DRVSNCFGPARA *pSnCfgPara,CWnd* pParent /*=NULL*/)
	: CDialog(CDlgSNDirFiles::IDD, pParent)
	, m_pSnCfgPara(pSnCfgPara)
	, m_SNTotalSize(0)
{

}

CDlgSNDirFiles::~CDlgSNDirFiles()
{
	
}

extern CSNDirFilesApp theApp;
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

BOOL CDlgSNDirFiles::InitCtrls(CSerial& lSerial)
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

BOOL CDlgSNDirFiles::GetCtrls(CSerial&lSerial)
{
	return m_SnCfg.SerialOutCfgData(lSerial);
}


INT CDlgSNDirFiles::QuerySN(DWORD Idx,BYTE*pData,INT*pSize)
{
	INT Ret=0;
	CString strMsg;
	CSerial lSerial;
	BYTE*pTmpData=NULL;
	UINT Sum=0;
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
		lSerial<<pGroup->dwSNLen;

		strMsg.Format("SNQuery: ProgramIdx=%d,写入地址为0x%I64X,长度为0x%X\r\n",Idx,pGroup->llSNStartAddr,pGroup->dwSNLen);
		WriteLog(strMsg);
		
		pTmpData=new BYTE[pGroup->dwSNLen];
		if(pTmpData){
			CFile File;
			Sum=0;
			if(File.Open(FilePath,CFile::modeRead,NULL)==FALSE){
				strMsg.Format("QuerySN: 打开文件失败, 文件名:%s\r\n",FilePath);
				WriteLog(strMsg);
				Ret=-1; goto __end;
			}

			if(File.GetLength()!=pGroup->dwSNLen){
				strMsg.Format("QuerySN: 文件名:%s 长度不为0x%X,请确认\r\n",FilePath,pGroup->dwSNLen);
				WriteLog(strMsg);
				Ret=-1; goto __end;
			}

			if(File.Read(pTmpData,pGroup->dwSNLen)!=pGroup->dwSNLen){
				strMsg.Format("QuerySN: 读取文件%s 失败\r\n",FilePath);
				WriteLog(strMsg);
				Ret=-1; goto __end;
			}
			///序列化实际写入数据
			lSerial.SerialInBuff(pTmpData,pGroup->dwSNLen);

			for(int j=0;j<pGroup->dwSNLen;++j){
				Sum+=pTmpData[j];
			}
			strMsg.Format("Sum值:0x%08X,文件名:%s\r\n",Sum,FilePath);
			WriteLog(strMsg);

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

INT CDlgSNDirFiles::TellResult(DWORD Idx,INT IsPass)
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

INT CDlgSNDirFiles::OpenLog()
{
	CString strFile;
	CTime Time=CTime::GetCurrentTime();
	CString LogTime=Time.Format("%H-%M-%S");
	strFile.Format("%s\\..\\log\\SNDirFilesLog_%s.txt",DllGetCurrentPath(),LogTime);
	if(m_logFile.Open(strFile,CFile::modeCreate|CFile::modeWrite|CFile::shareDenyNone,NULL)==FALSE){
		return -1;
	}
	return 0;
}

void CDlgSNDirFiles::WriteLog(CString str)
{
	if(m_logFile.m_hFile!=CFile::hFileNull){
		m_logFile.WriteString(str);
		m_logFile.Flush();
	}
}

void CDlgSNDirFiles::CloseLog()
{
	if(m_logFile.m_hFile!=CFile::hFileNull){
		m_logFile.Flush();
		m_logFile.Close();
	}
}

INT CDlgSNDirFiles::CreatePassDir( const CString& DirName)
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

BOOL  CDlgSNDirFiles::MoveFileToPass(const CString&File,CString& DestDir)
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

INT CDlgSNDirFiles::PreLoad()
{
	INT Ret=0,i,j;
	CString strMsg;
	CString strFileName;
	CString strSampleBinExtName="";
	CStdioFile FileListTmpFile;
	m_SNTotalSize=0;
	
	Ret=OpenLog();
	if(Ret!=0){
		Ret=-1; goto __end;
	}
	if(m_SnCfg.GetGroupNum()>GROUPCNT_MAX){
		strMsg.Format("错误:SNDirFiles模式最大支持%d组,当前选择了%d组请修改对应的设置\r\n",GROUPCNT_MAX,m_SnCfg.GetGroupNum());
		WriteLog(strMsg);
		Ret=-1; goto __end;
	}
	for(i=0;i<m_SnCfg.GetGroupNum();++i){
		SNGROUP* pGroup=m_SnCfg.GetGroup(i);
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
			m_vFileSearcher[i].ScanFiles(pGroup->strSNDir,pGroup->dwSNLen,strSampleBinExtName);
			m_vFileSearcher[i].Sort();
			for(j=0;j<m_vFileSearcher[i].GetListSize();++j){
				CString str=m_vFileSearcher[i].GetIdx(j);
				strMsg.Format("%05d,%s\r\n",j+1,str);
				FileListTmpFile.WriteString(strMsg);
			}
			FileListTmpFile.Close();
		}
	}
	m_SNTotalSize=m_SnCfg.GetSNTotalSize();
__end:
	if(FileListTmpFile.m_hFile!=CFile::hFileNull){
		FileListTmpFile.Close();
	}
	if(Ret==0){
		WriteLog("PreLoad成功\r\n");
	}
	else{
		WriteLog("PreLoad失败\r\n");
	}
	return Ret;
}

void CDlgSNDirFiles::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_ListCtrlST);
	DDX_Control(pDX, IDC_COMBO1, m_cmbSNGroup);
}


BEGIN_MESSAGE_MAP(CDlgSNDirFiles, CDialog)
	ON_CBN_SELCHANGE(IDC_COMBO1, &CDlgSNDirFiles::OnCbnSelchangeCombo1)
END_MESSAGE_MAP()

BOOL CDlgSNDirFiles::GetFileLength(CString strFile,UINT& Length)
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

LRESULT CDlgSNDirFiles::SetText(INT nRow,INT nColumn,CString&strText)
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
	CDlgSNDirFiles*pDlgSN=(CDlgSNDirFiles*)Para;
	if(pDlgSN){
		return pDlgSN->SetText(nRow,nColumn,strText);
	}
	return 0;
}


// CDlgSNDirFiles 消息处理程序

BOOL CDlgSNDirFiles::InitCtrlList()
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

BOOL CDlgSNDirFiles::InitCtrlsValue(CSNDirFilesCfg& SNCfg)
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
	}
	return TRUE;
}

BOOL CDlgSNDirFiles::OnInitDialog()
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
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CDlgSNDirFiles::OnCbnSelchangeCombo1()
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
