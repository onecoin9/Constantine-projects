
#include "Serialize.h"


CSerial::CSerial()
{
	m_pBuff  = NULL;
	m_uiLen  = 0;
	m_uiSize = 0;
	m_uiStart = 0;
}

CSerial::~CSerial()
{
	if (m_pBuff) {
		delete m_pBuff;
		m_pBuff = NULL;
	}
}

int CSerial::ReInit()
{
	if (m_pBuff) {
		delete m_pBuff;
		m_pBuff = NULL;
	}
	m_uiLen  = 0;
	m_uiSize = 0;
	m_uiStart = 0;

	return 0;
}

unsigned char* CSerial::GetBuffer()
{
	return m_pBuff + m_uiStart;
}

unsigned int   CSerial::GetLength()
{
	return m_uiLen;
}


CSerial & CSerial::SerialInBuff(unsigned char* lpBuff, unsigned int uiLen)
{	
	AddBuff(uiLen);

	memcpy(m_pBuff+m_uiLen, lpBuff, uiLen);
	m_uiLen += uiLen;
	
	return *this;
}

CSerial & CSerial::SerialOutBuff(unsigned char* lpBuff, unsigned int uiLen)
{
	if (!m_pBuff) {
		throw BUFF_SERIAL_ERR;
	}
	
	memcpy(lpBuff, m_pBuff+m_uiStart, uiLen);

	DelBuff(uiLen);
	
	return *this;
}

int CSerial::AddBuff(unsigned int uiLenAdd)
{
	//内存不足,重新分配内存
	if (m_uiStart + m_uiLen + uiLenAdd > m_uiSize) {
		m_uiSize = ((m_uiLen + uiLenAdd) / SERIAL_MALLOC_SIZE + 1) 
			* SERIAL_MALLOC_SIZE;	//计算新的大小
		
		unsigned char* lpBuff = new unsigned char[m_uiSize];
		if (!lpBuff) {
			throw BUFF_SERIAL_ERR;
		}
		
		if (m_pBuff) {	//避免最初未初始化m_pBuff的情况
			memcpy(lpBuff, m_pBuff + m_uiStart, m_uiLen);
			delete m_pBuff;
		}
		m_pBuff = lpBuff;
		m_uiStart = 0;
	}

	return 0;
}

int CSerial::DelBuff(unsigned int uiLenDel)
{
	if (uiLenDel > m_uiLen) {
		uiLenDel = m_uiLen;
	}

	m_uiStart += uiLenDel;
	m_uiLen  -= uiLenDel;

	if (m_uiStart > SERIAL_MALLOC_SIZE) {
		m_uiSize = (m_uiLen/SERIAL_MALLOC_SIZE + 1)
			* SERIAL_MALLOC_SIZE;
		
		unsigned char* lpBuff = new unsigned char[m_uiSize];
		if (!lpBuff) {
			throw BUFF_SERIAL_ERR;
		}
		
		memcpy(lpBuff, m_pBuff + m_uiStart, m_uiLen);
		delete m_pBuff;
		m_pBuff = lpBuff;
		m_uiStart = 0;
	}

	return 0;
}