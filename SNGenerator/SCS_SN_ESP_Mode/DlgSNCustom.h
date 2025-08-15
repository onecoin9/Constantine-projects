#pragma once

#include "stdint.h"
#include "../Com/Serial.h"
#include "../../MultiAprog/ComStruct/DrvSNCfg.h"
#include "SNCustomCfg.h"
#include "DlgWorkSheet.h"
#include "LogFile.h"
#include "afxwin.h"

#include <map>
#include <vector>

// CDlgSNCustom 对话框
enum format_type{
	error = 0,
	ascii_hex,
	intel_hex,
	motorola
};

typedef void (*MessageDumpCallback)(const char*);


class CDlgSNCustom : public CDialog
{
	DECLARE_DYNAMIC(CDlgSNCustom)

	typedef std::map<CString, std::vector<CString>> ProgInfo;

public:
	CDlgSNCustom(DRVSNCFGPARA *pSnCfgPara,CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlgSNCustom();

	BOOL InitCtrls(CSerial& lSerial);
	BOOL GetCtrls(CSerial& lSerial);
	INT QuerySN(DWORD Idx, BYTE* pData, INT* pSize);
	INT TellResult(DWORD Idx, INT IsPass);
	INT PreLoad();
	INT SetProgramInfo(const char* info_json);
	void SetMessageDumpCallback(MessageDumpCallback callback) { callback_ = callback; }

	bool ShowWorkSheetDlg();

// 对话框数据
	enum { IDD = IDD_DLGSNCUSTOM };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();

	afx_msg void OnBnClickedSelBtn1();
	afx_msg void OnBnClickedSelBtn2();
	afx_msg void OnBnClickedSelBtn3();

	DECLARE_MESSAGE_MAP()

private:
	void CreatLog(CString log_path);
	void ShowUI(tSNCustomCfg* cfg);
	void SaveCfgFromUI(tSNCustomCfg* cfg);
	bool ExecuteCmd(DWORD Idx, tSNCustomCfg* cfg, CString& out_path);
	INT ParseSerial(const CString& out_path, BYTE* pData, INT* pSize);
	INT ParseFormat(CStdioFile& file, const CString& format_code, BYTE* pData, INT* pSize);
	bool DecodeSerialFile(tSNCustomCfg* cfg, const CString& out_path, CString& dec_path, CString& err);

	void FormatOutPath(tSNCustomCfg* cfg,  CString& cmd, CString& out_path);
	void HandleExeParam(tSNCustomCfg* cfg, CString& cmd);
	void WorkSheetReplace(CString& str);

	std::pair<UINT64, CString> FormatASCIIHex(const CString& line, UINT& group_count, CString& log_sn, char flag);
	std::pair<UINT64, CString> FormatIntelHex(const CString& line, UINT& group_count, CString& log_sn, UINT& linear, UINT& segment);
	std::pair<UINT64, CString> FormatMotorola(const CString& line, UINT& group_count, CString& log_sn);
	format_type GetFormatType(const CString& format_code);
	char GetSplitCharset(const CString& format_code) const;

	CString KeyPath(tSNCustomCfg* cfg, const CString& out_path) const;
	void BakFile(tSNCustomCfg* cfg, const CString& out_path, DWORD idx);
	void InjectEsplog(tSNCustomCfg* cfg, const CString& out_path);
	bool RowCheck(CString& line) const;
	std::vector<CString> SplitCString(const CString& str, char flag) const;
	CString CurrentTime() const;
	void GetSiteMap();
	INT GetSiteAutoMap(const CString& strSiteAutoMap);
	INT GetSiteIdx(const CString& DevAlias);

private:
	CLogFile file_log_;
	CSNCustomCfg sn_cfg_;
	DRVSNCFGPARA* sn_cfg_para_;
	ProgInfo prog_info_;
	bool batch_start_;
	CString last_ret_;
	MessageDumpCallback callback_;

	INT en_wsht_;
	CDlgWorkSheet wsht_dlg_;
	std::map<CString, UINT> site_idx_;
};
