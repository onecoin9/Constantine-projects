#ifndef __I_OUTPUT_H__
#define __I_OUTPUT_H__
#include <iostream>
typedef enum {
	LOG_OUTPUT,
	LOG_WARNING,
	LOG_ERROR,
	LOG_CRITMESSAGE,
}LOGLEVEL;

//IOutput接口：用于显示信息
class IOutput
{
public:
	IOutput() {};
	virtual ~IOutput() {};

	//分级别记录日志
	virtual int32_t Log(int32_t iLevel, const char* strOutput) = 0;

	//打印日志
	virtual int32_t Log(const char* strOutput) = 0;

	//打印警告
	virtual int32_t Warning(const char* strOutput) = 0;

	//打印错误
	virtual int32_t Error(const char* strOutput) = 0;

	//弹出消息框显示信息
	virtual int32_t Message(const char* strOutput) = 0;

	//弹出选择框供选择
	virtual int32_t MsgChoose(const char* strOutput) = 0;

	//设置进度条大小
	virtual void SetProgress(uint32_t uiComplete) = 0;

	//设置进度条位置
	virtual void SetProgPos(uint32_t uiPos) = 0;

	//扩张功能，用于多线程停止
	virtual int32_t  Exit() = 0;

	//扩张功能，用于多线程检测用户是否点击退出按钮
	virtual bool BeCanceled() = 0;
};

#endif