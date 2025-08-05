#ifndef __I_OUTPUT_H__
#define __I_OUTPUT_H__
#include <iostream>
typedef enum {
	LOG_OUTPUT,
	LOG_WARNING,
	LOG_ERROR,
	LOG_CRITMESSAGE,
}LOGLEVEL;

//IOutput�ӿڣ�������ʾ��Ϣ
class IOutput
{
public:
	IOutput() {};
	virtual ~IOutput() {};

	//�ּ����¼��־
	virtual int32_t Log(int32_t iLevel, const char* strOutput) = 0;

	//��ӡ��־
	virtual int32_t Log(const char* strOutput) = 0;

	//��ӡ����
	virtual int32_t Warning(const char* strOutput) = 0;

	//��ӡ����
	virtual int32_t Error(const char* strOutput) = 0;

	//������Ϣ����ʾ��Ϣ
	virtual int32_t Message(const char* strOutput) = 0;

	//����ѡ���ѡ��
	virtual int32_t MsgChoose(const char* strOutput) = 0;

	//���ý�������С
	virtual void SetProgress(uint32_t uiComplete) = 0;

	//���ý�����λ��
	virtual void SetProgPos(uint32_t uiPos) = 0;

	//���Ź��ܣ����ڶ��߳�ֹͣ
	virtual int32_t  Exit() = 0;

	//���Ź��ܣ����ڶ��̼߳���û��Ƿ����˳���ť
	virtual bool BeCanceled() = 0;
};

#endif