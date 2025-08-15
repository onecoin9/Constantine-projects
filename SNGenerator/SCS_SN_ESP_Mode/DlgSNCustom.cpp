// DlgSNCustom.cpp : 实现文件
//
#include "stdafx.h"
#include "SCS_SN_ESP_Mode.h"
#include "DlgSNCustom.h"

#include "../Com/aes.h"
#include "../Com/ComTool.h"
#include "../Com/ComFunc.h"
#include "../Com/cJson.h"

#include <functional>

extern unsigned char wrap_key1[];
extern unsigned char wrap_key2[];
static unsigned char wrap_key3[] = { 0xCB, 0xF0, 0xB2, 0x3B, 0xC8, 0xB1, 0x03, 0xD8 };

struct FileRAII {
	FileRAII(CStdioFile* file) : file_(file){}
	~FileRAII(){
		if(file_){
			file_->Close();
			file_ = NULL;
		}
	}

private:
	CStdioFile* file_;
};

struct PointRAII {
	PointRAII(ULONGLONG len) : enc_ptr(new unsigned char[len]), dec_ptr(new unsigned char[len * 2]), size(len) {}
	~PointRAII(){
		if(enc_ptr){
			delete[] enc_ptr;
			enc_ptr = NULL;
		}
		if(dec_ptr){
			delete[] dec_ptr;
			dec_ptr = NULL;
		}
	}

	unsigned char* enc_ptr;
	unsigned char* dec_ptr;
	ULONGLONG size;
};

// CDlgSNCustom 对话框

IMPLEMENT_DYNAMIC(CDlgSNCustom, CDialog)

CDlgSNCustom::CDlgSNCustom(DRVSNCFGPARA *pSnCfgPara,CWnd* pParent /*=NULL*/)
	: CDialog(CDlgSNCustom::IDD, pParent)
	, sn_cfg_para_(pSnCfgPara)
	, batch_start_(true)
	, last_ret_("first")
	, en_wsht_(0)
{
	CString bak_dir;
	bak_dir.Format("%s\\sngen\\esp", CComFunc::GetCurrentPath());
	CreateDirectory(bak_dir, NULL);
}

CDlgSNCustom::~CDlgSNCustom()
{
}



BOOL CDlgSNCustom::InitCtrls(CSerial& lSerial)
{
	if(lSerial.GetLength() == 0)
		return TRUE;

	if(!sn_cfg_.SerialOutCfgData(lSerial))
		return FALSE;
	
	ShowUI(sn_cfg_.GetCustomCfg());
	return TRUE;
}



BOOL CDlgSNCustom::GetCtrls(CSerial& lSerial)
{
	SaveCfgFromUI(sn_cfg_.GetCustomCfg());
	return sn_cfg_.SerialInCfgData(lSerial);
}



INT CDlgSNCustom::QuerySN(DWORD Idx, BYTE* pData, INT* pSize)
{
	if (en_wsht_ && !wsht_dlg_.GetFlag()) {
		AfxMessageBox("Please set WorkSheet first!", MB_OK | MB_ICONERROR);
		callback_("LotEndReq");
		return -1;
	}

	tSNCustomCfg *cfg = NULL, *exe_save_cfg = sn_cfg_.GetCustomCfg(), ui_cfg;
	if(exe_save_cfg->esp_exe_path.IsEmpty()){
		SaveCfgFromUI(&ui_cfg);
		cfg = &ui_cfg;
	}
	else cfg = exe_save_cfg;

	CreatLog(cfg->log_path);
	if(!PathFileExists(cfg->esp_exe_path)){
		file_log_.PrintLog(ERR, "Query SN fail, esp exe file is not exist");
		return -1;
	}

	file_log_.PrintLog(LOG, "exe path: %s, index: %d, Query SN...", cfg->esp_exe_path, Idx);
	CString out_path;
	if (!ExecuteCmd(Idx, cfg, out_path)) {
		file_log_.PrintLog(ERR, "Query SN fail, execute esp exe fail");
		return -1;
	}

	CString dec_path, dec_err;
	if (!DecodeSerialFile(cfg, out_path, dec_path, dec_err)) {
		file_log_.PrintLog(ERR, "Query SN fail, decode dat file fail, %s", dec_err);
		return -1;
	}

	BakFile(cfg, out_path, Idx);
	INT parse_ret = ParseSerial((cfg->serial_enc ? dec_path : out_path), pData, pSize);
	if (cfg->serial_enc)
		DeleteFile(dec_path);
	return parse_ret;
}



INT CDlgSNCustom::TellResult(DWORD Idx, INT IsPass)
{
	last_ret_ = IsPass ? "pass" : "fail";
	return 0;
}



INT CDlgSNCustom::PreLoad()
{
	return 0;
}



INT CDlgSNCustom::SetProgramInfo(const char* info_json) {
	prog_info_.clear();
	cJSON* json = cJSON_Parse(info_json);
	if (json == NULL)
		return -1;

	cJSON* site_arr = cJSON_GetObjectItem(json, "SiteInfo");
	if (site_arr == NULL)
		return -1;

	int count = cJSON_GetArraySize(site_arr);
	for (int i = 0; i < count; ++i) {
		cJSON* site = cJSON_GetArrayItem(site_arr, i);
		CString dev_sn = cJSON_GetObjectItem(site, "SiteSN")->valuestring;

		std::vector<CString> skt_vec;
		cJSON* skt = cJSON_GetObjectItem(site, "SlotInfo");
		for (int j = 0; j < 8; ++j) {
			cJSON* item = cJSON_GetArrayItem(skt, i);
			skt_vec.push_back(cJSON_GetObjectItem(item, "SN")->valuestring);
		}
		prog_info_[dev_sn] = skt_vec;
	}
	cJSON_Delete(json);
	return 0;
}



bool CDlgSNCustom::ShowWorkSheetDlg() {
	return en_wsht_ ? wsht_dlg_.DoModal() == IDOK : true;
}



void CDlgSNCustom::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgSNCustom, CDialog)
	ON_BN_CLICKED(IDC_SEL_BTN1, &CDlgSNCustom::OnBnClickedSelBtn1)
	ON_BN_CLICKED(IDC_SEL_BTN2, &CDlgSNCustom::OnBnClickedSelBtn2)
	ON_BN_CLICKED(IDC_SEL_BTN3, &CDlgSNCustom::OnBnClickedSelBtn3)
END_MESSAGE_MAP()


// CDlgSNCustom 消息处理程序

BOOL CDlgSNCustom::OnInitDialog()
{
	CDialog::OnInitDialog();
	CString ini_path;
	ini_path.Format("%s\\sngen\\SCS_SN_ESP_Mode.ini", CComFunc::GetCurrentPath());
	en_wsht_ = GetPrivateProfileInt("SpecialRule", "enable", 0, ini_path);
	GetDlgItem(IDC_SR_TEXT)->ShowWindow(en_wsht_);
	GetDlgItem(IDC_SR_EDIT)->ShowWindow(en_wsht_);

	CHAR tmp_buf[MAX_PATH] = { 0 };
	GetPrivateProfileString("SpecialRule", "rule", "", tmp_buf, MAX_PATH, ini_path);
	GetDlgItem(IDC_SR_EDIT)->SetWindowText(tmp_buf);

	GetSiteMap();
	return TRUE;
}



void CDlgSNCustom::OnBnClickedSelBtn1()
{ 
    CFileDialog dlg(TRUE, "exe", NULL, 0, "应用程序(*.exe)|*.exe", this);   
    if (IDOK == dlg.DoModal()){   
        GetDlgItem(IDC_EEF_EDIT)->SetWindowText(dlg.GetPathName());
    }  
}



void CDlgSNCustom::OnBnClickedSelBtn2()
{
    CFileDialog dlg(FALSE, "dat", "serial", OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, "输出文件(*.dat)", this);   
    if (IDOK == dlg.DoModal()){   
		GetDlgItem(IDC_SDF_EDIT)->SetWindowText(dlg.GetPathName());
    }
}



void CDlgSNCustom::OnBnClickedSelBtn3()
{
	CFileDialog dlg(FALSE, "txt", "serial", OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, "输出文件(*.txt)", this);   
    if (IDOK == dlg.DoModal()){   
		GetDlgItem(IDC_SLF_EDIT)->SetWindowText(dlg.GetPathName());
    }
}



void CDlgSNCustom::CreatLog(CString log_path)
{
	if (log_path.IsEmpty())
		return;

	WorkSheetReplace(log_path);
	file_log_.SetLogFile(log_path);
}



void CDlgSNCustom::ShowUI(tSNCustomCfg* cfg){
	GetDlgItem(IDC_EEF_EDIT)->SetWindowText(cfg->esp_exe_path);
	GetDlgItem(IDC_ACLP_EDIT)->SetWindowText(cfg->exe_param);
	GetDlgItem(IDC_SDF_EDIT)->SetWindowText(cfg->data_path);
	GetDlgItem(IDC_SLF_EDIT)->SetWindowText(cfg->log_path);
	GetDlgItem(IDC_KEY_EDIT)->SetWindowText(cfg->key_path);

	CString str;
	if(cfg->first_num != 0){
		str.Format("%d", cfg->first_num);
		GetDlgItem(IDC_FSN_EDIT)->SetWindowText(str);
	}
	if(cfg->last_num != 0){
		str.Format("%d", cfg->last_num);
		GetDlgItem(IDC_LSN_EDIT)->SetWindowText(str);
	}

	((CButton*)GetDlgItem(IDC_CM_CHECK))->SetCheck(cfg->cmp_mode);
	((CButton*)GetDlgItem(IDC_RTAST_CHECK))->SetCheck(cfg->t02_as_str);
	((CButton*)GetDlgItem(IDC_DDFBCEE_CHECK))->SetCheck(cfg->delete_before);
	((CButton*)GetDlgItem(IDC_ENC_CHK))->SetCheck(cfg->serial_enc);
}



void CDlgSNCustom::SaveCfgFromUI(tSNCustomCfg* cfg){
	GetDlgItem(IDC_EEF_EDIT)->GetWindowText(cfg->esp_exe_path);
	GetDlgItem(IDC_ACLP_EDIT)->GetWindowText(cfg->exe_param);
	GetDlgItem(IDC_SDF_EDIT)->GetWindowText(cfg->data_path);
	GetDlgItem(IDC_SLF_EDIT)->GetWindowText(cfg->log_path);
	GetDlgItem(IDC_KEY_EDIT)->GetWindowText(cfg->key_path);

	CString str;
	GetDlgItem(IDC_FSN_EDIT)->GetWindowText(str);
	cfg->first_num = atoi(str.GetString());
	GetDlgItem(IDC_LSN_EDIT)->GetWindowText(str);
	cfg->last_num = atoi(str.GetString());

	cfg->cmp_mode = !!((CButton*)GetDlgItem(IDC_CM_CHECK))->GetCheck();
	cfg->t02_as_str = !!((CButton*)GetDlgItem(IDC_RTAST_CHECK))->GetCheck();
	cfg->delete_before = !!((CButton*)GetDlgItem(IDC_DDFBCEE_CHECK))->GetCheck();
	cfg->serial_enc = !!((CButton*)GetDlgItem(IDC_ENC_CHK))->GetCheck();
}



bool CDlgSNCustom::ExecuteCmd(DWORD Idx, tSNCustomCfg* cfg, CString& out_path) {
	CString cmd;
	cmd.Format(" -N%d", Idx);
	if(batch_start_){
		cmd.Append(" -F");
		batch_start_ = false;
	}
	if(cfg->last_num != 0){
		CString str; str.Format(" -E%d", cfg->last_num);
		cmd.Append(str);
	}
	if (cfg->serial_enc) {
		cmd.Append(" -K");
	}
	HandleExeParam(cfg, cmd);
	FormatOutPath(cfg, cmd, out_path);
	file_log_.PrintLog(LOG, "exe command: %s", cmd);
	file_log_.PrintLog(LOG, "out dat file path: %s", out_path);

	if (cfg->delete_before) {
		DeleteFile(out_path);
		DeleteFile(KeyPath(cfg, out_path));
	}

	STARTUPINFO si;
	memset(&si, 0, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);
	si.wShowWindow = SW_HIDE;
	si.hStdError = NULL;
	si.hStdOutput = NULL;
	si.dwFlags = STARTF_USESHOWWINDOW;

	PROCESS_INFORMATION pi;
	if (!CreateProcess(cfg->esp_exe_path, cmd.GetBuffer(0), NULL, NULL, TRUE, NULL, NULL, NULL, &si, &pi)) {
		file_log_.PrintLog(ERR, "Call esp exe failed");
		return false;
	}
	
	WaitForSingleObject(pi.hProcess, INFINITE);///等待进程结束
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
	InjectEsplog(cfg, out_path);
	return true;
}



INT CDlgSNCustom::ParseSerial(const CString& out_path, BYTE* pData, INT* pSize){
	CStdioFile file;
	FileRAII raii(&file);
	if (!file.Open(out_path, CFile::typeBinary | CFile::modeRead | CFile::shareDenyNone, NULL)) {
		file_log_.PrintLog(ERR, "Open serial file failed, path: %s", out_path);
		return -1;
	}

	CString line;
	while (file.ReadString(line)) {
		if (!RowCheck(line))
			continue;

		if (line.Find("T05") != -1) {
			AfxMessageBox("T05: fatal error message, stopping automatic", MB_OK | MB_ICONERROR);
			if (callback_)
				callback_("LotEndReq");
			return -1;
		}

		static CString format_code;
		if(line.Find("T03") != -1){
			format_code = line.Right(line.GetLength() - line.Find(':') - 1);
			file_log_.PrintLog(LOG, "translation format: %s", format_code);
		}

		if(line.CompareNoCase("T04") == 0)
			return ParseFormat(file, format_code, pData, pSize);
	}

	file_log_.PrintLog(ERR, "Parse serial file failed, not find T04, no content");
	return -1;
}



INT CDlgSNCustom::ParseFormat(CStdioFile& file, const CString& format_code, BYTE* pData, INT* pSize){
	format_type ft = GetFormatType(format_code);
	char flag = GetSplitCharset(format_code);
	UINT linear = 0, segment = 0;

	CString line;
	UINT group_count = 0;
	std::vector<std::pair<UINT64, CString>> sn_group;
	while(file.ReadString(line)){
		if (!RowCheck(line))
			continue;

		CString log_sn;
		std::pair<UINT64, CString> content;
		switch (ft) {
		case ascii_hex:
			content = FormatASCIIHex(line, group_count, log_sn, flag);
			break;
		case intel_hex:
			content = FormatIntelHex(line, group_count, log_sn, linear, segment);
			break;
		case motorola:
			content = FormatMotorola(line, group_count, log_sn);
			break;
		default:
			return -1;
		}

		if(!log_sn.IsEmpty()){
			sn_group.push_back(content);
			file_log_.PrintLog(LOG, "Parse sn data: %s", log_sn);
			break;
		}
	}

	CSerial lSerial;
	lSerial << group_count;
	for(UINT i = 0; i < sn_group.size(); ++i){
		lSerial << sn_group[i].first << sn_group[i].second.GetLength();
		lSerial.SerialInBuff((BYTE*)sn_group[i].second.GetBuffer(), sn_group[i].second.GetLength());
	}

	if(lSerial.GetLength() > *pSize){
		*pSize = lSerial.GetLength();
		file_log_.PrintLog(LOG, "SN buff size is not enough");
		return -2;
	}

	memcpy(pData, lSerial.GetBuffer(), lSerial.GetLength());
	return lSerial.GetLength();
}



bool CDlgSNCustom::DecodeSerialFile(tSNCustomCfg* cfg, const CString& out_path, CString& dec_path, CString& err) {
	if (!cfg->serial_enc)
		return true;

	CString key_path = KeyPath(cfg, out_path);
	const unsigned char wrap_key4[8] = { 0xFC, 0xAF, 0xA1, 0x13, 0x05, 0x7D, 0xC7, 0xD9 };
	unsigned char wrap_key[32] = { 0 };
	memcpy(wrap_key, wrap_key1, 8);
	memcpy(wrap_key + 8, wrap_key2, 8);
	memcpy(wrap_key + 16, wrap_key3, 8);
	memcpy(wrap_key + 24, wrap_key4, 8);

	CFile trans_file;
	if (!trans_file.Open(key_path, CFile::modeRead | CFile::typeBinary)) {
		err.Format("Open wrapping-key file failed!");
		return false;
	}

	UINT64 enc_key_len = trans_file.GetLength();
	std::vector<unsigned char> enc_key(enc_key_len, '\0');
	unsigned char dec_key[32];
	trans_file.Read(&enc_key[0], enc_key_len);
	trans_file.Close();

	aes_init();
	if (unwrapfile256(wrap_key, NULL, &enc_key[0], dec_key) != 32) {
		err.Format("Unwrapping transport key failed!");
		return false;
	}

	CFile enc_file;
	if (!enc_file.Open(out_path, CFile::modeRead | CFile::typeBinary)) {
		err.Format("Open encode serial-file failed!");
		return false;
	}

	PointRAII ptr(enc_file.GetLength());
	enc_file.Read(ptr.enc_ptr, ptr.size);
	enc_file.Close();

	int olen = decodefile256(dec_key, ptr.enc_ptr, ptr.size, ptr.dec_ptr, ptr.size * 2);
	if (olen <= 0) {
		err.Format("Decode serial-file failed!");
		return false;
	}

	dec_path.Format("%s.dec", out_path);
	CFile dec_file;
	dec_file.Open(dec_path, CFile::modeCreate | CFile::modeWrite | CFile::typeBinary);
	dec_file.Write(ptr.dec_ptr, olen);
	dec_file.Close();
	return true;
}



void CDlgSNCustom::FormatOutPath(tSNCustomCfg* cfg, CString& cmd, CString& out_path) {
	out_path = cfg->data_path;
	WorkSheetReplace(out_path);

	INT pos = cmd.Find("--data-file=");
	if (pos != -1) {
		INT pos2 = cmd.Find(' ', pos);
		if (pos2 == -1)
			pos2 = cmd.GetLength();
		if (out_path.IsEmpty())
			out_path.Format("%s\\%s", cfg->esp_exe_path.Left(cfg->esp_exe_path.ReverseFind('\\')), cmd.Mid(pos + 12, pos2 - pos - 12));
		cmd.Delete(pos, pos2 - pos);
	}
	else {
		if (out_path.IsEmpty())
			out_path.Format("%s\\serial.dat", cfg->esp_exe_path.Left(cfg->esp_exe_path.ReverseFind('\\')));
	}

	cmd.Append(" --data-file=\"");
	cmd.Append(out_path);
	cmd.Append("\"");
}



void CDlgSNCustom::HandleExeParam(tSNCustomCfg* cfg, CString& cmd) {
	if(cfg->exe_param.IsEmpty())
		return;

	CString param;
	param.Format(" %s", cfg->exe_param);
	/*INT pos = param.Find("--data-file=");
	if (pos != -1) {
		INT end_pos = param.Find(' ', pos);
		CString remove_str = (end_pos == -1 ? param.Mid(pos + 12) : param.Mid(pos + 12, param.Find(' ', pos) - pos - 12));
		param.Replace(remove_str, "");
	}*/
	WorkSheetReplace(param);

	CString site_info = sn_cfg_para_->pSiteSN;
	CString dev_sn, site_name;
	INT pos = site_info.Find('(');
	if (pos != -1) {
		site_name = site_info.Mid(pos + 1);
		site_name.Remove(')');
		dev_sn = site_info.Mid(0, pos);
	}

	CString str;
	str.Format("--site-id=%s", dev_sn);
	param.Replace("$SITE_ID", str);

	ProgInfo::iterator iter = prog_info_.find(dev_sn);
	str.Format("--socket-id=%s", (iter == prog_info_.end() ? "" : iter->second[sn_cfg_para_->AdapIdx]));
	param.Replace("$SOCKET_ID", str);

	str.Format("--site=%d", GetSiteIdx(site_name));
	param.Replace("$SITE", str);

	str.Format("--socket=%d", sn_cfg_para_->AdapIdx);
	param.Replace("$SOCKET", str);

	str.Format("--adaptor=%s", sn_cfg_para_->pAdapName);
	param.Replace("$ADAPTOR", str);
	param.Replace("$ADAPTER", str);

	str.Format("--result=%s", last_ret_);
	param.Replace("$RESULT", str);

	cmd.Append(param);
}



void CDlgSNCustom::WorkSheetReplace(CString& str) {
	if (en_wsht_) {
		WorkSheet wsheet = wsht_dlg_.GetWorkSheetParam();
		if (!wsheet.work_order.IsEmpty())
			str.Replace("[Work_Order]", wsheet.work_order);
		if (!wsheet.asset_num.IsEmpty())
			str.Replace("[Asset_Number]", wsheet.asset_num);
		if (!wsheet.emp_num.IsEmpty())
			str.Replace("[Employee_Number]", wsheet.emp_num);
		if (!wsheet.prog_name.IsEmpty())
			str.Replace("[Program_Name]", wsheet.prog_name);
		if (!wsheet.prog_id.IsEmpty())
			str.Replace("[Program_ID]", wsheet.prog_id);
	}
}



std::pair<UINT64, CString> CDlgSNCustom::FormatASCIIHex(const CString& line, UINT& group_count, CString& log_sn, char flag){
	static UINT64 addr = 0;
	CString sn;
	INT pos = line.Find("$A");
	if(pos == -1){
		std::vector<CString> res = SplitCString(line, flag);
		for(size_t i = 0; i < res.size(); ++i){
			UINT8 one_byte = strtol(res[i], 0, 16);
			sn += one_byte;
			log_sn.Append(res[i]);
		}
		++group_count;
	}
	else if(pos != -1) addr = strtol(line.Mid(pos + 2, line.ReverseFind(',') - pos - 2), 0, 16);
	return std::make_pair(addr, sn);
}



std::pair<UINT64, CString> CDlgSNCustom::FormatIntelHex(const CString& line, UINT& group_count, CString& log_sn, UINT& linear, UINT& segment){
	if(line.GetAt(0) != ':')
		return std::make_pair(0, "");

	UINT addr = 0, byte_count = strtol(line.Mid(1, 2), 0, 16), record_type = strtol(line.Mid(7, 2), 0, 16);
	CString sn;
	switch (record_type){
	case 0:
	{
		addr = linear + segment + strtol(line.Mid(3, 4), 0, 16);
		for(UINT i = 0; i < byte_count; ++i){
			CString str_obyte = line.Mid(9 + 2 * i, 2);
			UINT8 one_byte = strtol(str_obyte, 0, 16);
			sn += one_byte;
			log_sn.Append(str_obyte);
		}
		++group_count;
		break;
	}
	case 2:
	{
		CString offset = line.Mid(9, 4);
		offset.Append("0");
		segment = strtol(offset, 0, 16);
		break;
	}
	case 4:
	{
		CString offset = line.Mid(9, 4);
		offset.Append("0000");
		linear = strtol(offset, 0, 16);
		break;
	}
	default:
		break;
	}
	return std::make_pair(addr, sn);
}



std::pair<UINT64, CString> CDlgSNCustom::FormatMotorola(const CString& line, UINT& group_count, CString& log_sn){
	UINT s = line.Mid(1, 1).GetAt(0) - '0';
	if(s == 8 || s == 9)
		return std::make_pair(0, "");

	UINT byte_count = strtol(line.Mid(2, 2), 0, 16) - 5;
	UINT64 addr = strtol(line.Mid(4, 2 * s + 2), 0, 16);

	CString sn;
	for(UINT j = 0; j < byte_count; ++j){
		CString str_obyte = line.Mid(2 + 2 + 2 * s + 2 + j * 2, 2);
		UINT8 one_byte = strtol(str_obyte, 0, 16);
		sn += one_byte;
		log_sn.Append(str_obyte);
	}
	++group_count;
	return std::make_pair(addr, sn);
}



format_type CDlgSNCustom::GetFormatType(const CString& format_code){
	if(format_code == "50" || format_code == "51" || format_code == "52" || format_code == "53" || format_code == "55" || format_code == "56" || 
		format_code == "57" || format_code == "58")
	{
		return ascii_hex;
	}
	else if(format_code == "82" || format_code == "87" || format_code == "95"){
		return motorola;
	}
	else if(format_code == "83" || format_code == "88" || format_code == "99"){
		return intel_hex;
	}
	else{
		file_log_.PrintLog(LOG, "Not support this translation format now, fail");
		return error;
	}
}



char CDlgSNCustom::GetSplitCharset(const CString& format_code) const{
	char flag = 'E';
	if(format_code == "50" || format_code == "55")
		flag = ' ';
	else if(format_code == "51" || format_code == "56")
		flag = '%';
	else if(format_code == "52" || format_code == "57")
		flag = '\'';
	else if(format_code == "53" || format_code == "58")
		flag = ',';
	return flag;
}



CString CDlgSNCustom::KeyPath(tSNCustomCfg* cfg, const CString& out_path) const {
	CString key_path;
	key_path.Format("%s\\wrapped.key", out_path.Mid(0, out_path.ReverseFind('\\')));
	if (!PathFileExists(key_path))
		key_path.Format("%s\\wrapped.key", cfg->esp_exe_path.Mid(0, cfg->esp_exe_path.ReverseFind('\\')));
	return key_path;
}



void CDlgSNCustom::BakFile(tSNCustomCfg* cfg, const CString& out_path, DWORD idx) {
	CString cur_time = CurrentTime();
	CString bak_path;
	bak_path.Format("%s\\sngen\\esp\\serial_%d_%s.dat", CComFunc::GetCurrentPath(), idx, cur_time);
	CopyFile(out_path, bak_path, TRUE);

	if (!cfg->serial_enc)
		return;

	CString key_bak;
	key_bak.Format("%s\\sngen\\esp\\key_%d_%s.dat", CComFunc::GetCurrentPath(), idx, cur_time);
	MoveFile(KeyPath(cfg, out_path), key_bak);
}



void CDlgSNCustom::InjectEsplog(tSNCustomCfg* cfg, const CString& out_path) {
	CString esp_log;
	esp_log.Format("%s\\esp.log", out_path.Left(out_path.ReverseFind('\\')));
	if (!PathFileExists(esp_log))
		esp_log.Format("%s\\esp.log", cfg->esp_exe_path.Mid(0, cfg->esp_exe_path.ReverseFind('\\')));

	CFile esp_file;
	if (!esp_file.Open(esp_log, CFile::modeRead))
		return;
	UINT64 len = esp_file.GetLength();
	std::vector<char> buff(len + 1, '\0');
	esp_file.Read(&buff[0], len);
	esp_file.Close();
	DeleteFile(esp_log);
	file_log_.PrintLog(LOG, &buff[0]);
}



bool CDlgSNCustom::RowCheck(CString& line) const{
	if (line.IsEmpty())
		return false;
	line.Remove('\r');
	if (line.IsEmpty())
		return false;
	return true;
}



std::vector<CString> CDlgSNCustom::SplitCString(const CString& str, char flag) const{
	UINT slow = 0, fast = 0;
	std::vector<CString> res;
	while(slow != str.GetLength()){
		fast = str.Find(flag, slow);
		res.push_back(str.Mid(slow, fast - slow));
		slow = fast + 1;
	}
	return res;
}



CString CDlgSNCustom::CurrentTime() const{
	CTime cur_time = CTime::GetCurrentTime();
	return cur_time.Format("%Y%m%d_%H%M%S");
}



void CDlgSNCustom::GetSiteMap(){
	CString strSiteMap;
	CHAR TmpBuf[MAX_PATH];
	CString strIniFile;
	memset(TmpBuf,0,MAX_PATH);
	strIniFile.Format("%s\\mes\\StdMes.ini",CComFunc::GetCurrentPath());
	GetPrivateProfileString("Config", "SitesAutoMap", "", TmpBuf, MAX_PATH, strIniFile);
	if(TmpBuf[0] == 0x00){
		strSiteMap = "";
	}
	else{
		strSiteMap.Format("%s",TmpBuf);
	}
	GetSiteAutoMap(strSiteMap);
}



INT CDlgSNCustom::GetSiteAutoMap(const CString& strSiteAutoMap) {
	CHAR ch;
	INT Ret = 0, i, Cnt;
	INT BracketCnt = 0;
	INT Comma = 0, Pos;
	CString strSiteAilas;
	CString strSiteIdx;
	CString strSiteMap;
	UINT nSiteIdx;
	if (strSiteAutoMap != "") {
		Cnt = strSiteAutoMap.GetLength();
		for (i = 0; i < Cnt; ++i) {
			ch = strSiteAutoMap.GetAt(i);
			if (ch == '<') {
				strSiteMap = "";
				BracketCnt++;
				Comma = 0;
			}
			else if (ch == '>') {
				Pos = strSiteMap.Find(',');
				if (Pos == -1) {
					Ret = -1; goto __end;
				}
				strSiteAilas = strSiteMap.Mid(0, Pos);
				strSiteIdx = strSiteMap.Mid(Pos + 1, 100);
				if (strSiteAilas == "" || strSiteIdx == "") {
					Ret = -1; goto __end;
				}
				sscanf((LPSTR)(LPCSTR)strSiteIdx, "%d", (INT *)&nSiteIdx);
				site_idx_[strSiteAilas] = nSiteIdx;
				BracketCnt--;
			}
			else {
				strSiteMap += ch;
			}
		}
	}
	if (BracketCnt != 0) {
		Ret = -1;
	}
__end:
	return Ret;
}



INT CDlgSNCustom::GetSiteIdx(const CString& DevAlias){
	INT SiteIdx = -1;
	if (site_idx_.find(DevAlias)  != site_idx_.end()) {
		return site_idx_[DevAlias];
	}
	return SiteIdx;
}
