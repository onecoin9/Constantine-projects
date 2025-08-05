#pragma once

#include <QObject>
#include <QLibrary>
#include "ChecksumDef.h"

/// <summary>
/// 获取可以使用的Chk校验名称
/// </summary>
typedef int (*DllGetCkSumName)(char*, uint32_t);

typedef uint64_t(*DllCheckSum)(CHECKSUMPARAM* param);

class AngKChkManager : public QObject
{
	Q_OBJECT

public:
	AngKChkManager(QString libName, QObject *parent = nullptr);
	~AngKChkManager();

	bool LoadChk();

	int GetCheckSumName();

	uint64_t CalCheckSum(CHECKSUMPARAM* param);
private:
	QLibrary m_chkLib;
};
