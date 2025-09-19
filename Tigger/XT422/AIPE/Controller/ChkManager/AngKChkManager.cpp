#include "AngKChkManager.h"
#include <QDebug>

AngKChkManager::AngKChkManager(QString libName, QObject *parent)
	: QObject(parent)
{
	m_chkLib.setFileName(libName);
}

AngKChkManager::~AngKChkManager()
{
	if (m_chkLib.isLoaded())
		m_chkLib.unload();
}

bool AngKChkManager::LoadChk()
{
	return m_chkLib.load();
}

int AngKChkManager::GetCheckSumName()
{
	if (m_chkLib.isLoaded()) {
		DllGetCkSumName GetCkSumNameFunction = (DllGetCkSumName)m_chkLib.resolve("DllGetCkSumName");

		// Call the function from the DLL
		if (GetCkSumNameFunction) {
			char* buff = new char[64];
			memset((char*)buff, 0, 64);
			int byte = GetCkSumNameFunction(buff, sizeof(buff));
			return byte;
		}
		else {
		}
	}
	return -1;
}

uint64_t AngKChkManager::CalCheckSum(CHECKSUMPARAM* param)
{
	if (m_chkLib.isLoaded()) {
		DllCheckSum CheckSumFunction = (DllCheckSum)m_chkLib.resolve("DllCheckSum");
		if (CheckSumFunction) {
			int checkSum = CheckSumFunction(param);
			return checkSum;
		}
	}
	return 0;
}
