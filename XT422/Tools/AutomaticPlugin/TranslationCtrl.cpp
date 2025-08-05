
#include "TranslationCtrl.h"
#include <map>
#include <QString>
#include "ComStruct/ComTool.h"
#include <string>

using std::string;

enum LanguageType {
	english,
	chinese
};

LanguageType g_ltype = english;

typedef std::map<int, std::pair<string, string>> ErrorType;
ErrorType g_emap = std::map<int, std::pair<string, string>>();

static void InitErrorMap() {
	g_emap[0] = std::make_pair("Machine task initialization success", string("设置自动化任务成功"));
	g_emap[1] = std::make_pair("Programmer is not ready", string("烧录机未Ready"));
	g_emap[2] = std::make_pair("Machine is running", string("设备自动运行中"));
	g_emap[3] = std::make_pair("Communication in process, please wait", string("前次通信处理进行中"));
	g_emap[4] = std::make_pair("Non-one key start programming mode", string("当前模式为非一键烧录模式"));
	g_emap[5] = std::make_pair("Incorrect task configuration", string("任务数据不正确"));
	g_emap[6] = std::make_pair("Incorrect tray/tape position", string("托盘/卷带指定位置不正确"));
	g_emap[7] = std::make_pair("Programmer initialization fail", string("烧录器未初始化成功(CONN)"));
	g_emap[8] = std::make_pair("License validation fail", string("Lic验证失败"));
	g_emap[10] = std::make_pair("TCP communication fail", string("TCP通信失败"));
	g_emap[11] = std::make_pair("No programmed connected, please check programmer state", string("没有站点被连接,请确认站点是否打开并且已经连接到PC上"));
	g_emap[12] = std::make_pair("Programmer connection error", string("连接站点出错"));
	g_emap[13] = std::make_pair("Acquire programmer information error", string("获取连接站点信息错误"));
	g_emap[14] = std::make_pair("Please make sure the ISP HMI is running with StdMES mode", string("请确认自动化软件已经运行，并且处于StdMes模式下"));
	g_emap[15] = std::make_pair("Thread working now, please wait a minute", string("工作线程正在执行中，请稍候再次请求"));
	g_emap[98] = std::make_pair("No Automes Mode", string("非AutoMes模式"));
	g_emap[99] = std::make_pair("Incorrect Command format", string("命令格式不正确"));

}

void InitTranslation(const std::string& cur_path) {
	std::string ExePath = ComTool::GetCurrentPath();
	std::string ini_path = ExePath + "\\LocalCfg.ini";
	//FIXME, Only Support English Now.
	g_ltype = english;

	InitErrorMap();
}


string TranslateErrMsg(int err_code, bool* ret) {
	ErrorType::iterator iter = g_emap.find(err_code);
	if (iter == g_emap.end()) {
		if (ret) *ret = false;
		return g_ltype == english ? "Not find error message" : "未找到对应的错误信息";
	}
	if (ret) *ret = true;
	return g_ltype == english ? iter->second.first : iter->second.second;
}