#ifndef __SERIALIZE_H__
#define __SERIALIZE_H__

#define SERIAL_MALLOC_SIZE 4096
#define BUFF_SERIAL_ERR   -1
#include <iostream>
#include <QString>
using namespace std;
class CSerial
{
public:
	CSerial();
	~CSerial();

public:
	unsigned char* GetBuffer();

	unsigned int   GetLength();

	int    ReInit();

	CSerial & SerialInBuff(unsigned char* lpBuff, unsigned int uiLen);
	CSerial & SerialOutBuff(unsigned char* lpBuff, unsigned int uiLen);
	template <typename T> inline CSerial & operator<<(T tIn)
	{
		AddBuff(sizeof(tIn));
		
		memcpy(m_pBuff+m_uiLen, (unsigned char*) &tIn, sizeof(tIn));
		m_uiLen += sizeof(tIn);
		
		return *this;
	}
	
	template <typename T> inline CSerial & operator>>(T &tOut)
	{
		if (!m_pBuff) {
			throw BUFF_SERIAL_ERR;
		}
		
		memcpy((unsigned char*) &tOut, m_pBuff+m_uiStart, min((unsigned int)sizeof(tOut), m_uiLen));
		DelBuff(min((unsigned int)sizeof(tOut), m_uiLen));
			
		return *this;
	}

	template <> inline CSerial & operator<<(QString tIn)
	{
		unsigned int uiLen = tIn.size() + 1;

		AddBuff(uiLen);
		
		memcpy(m_pBuff+m_uiLen, tIn.toLocal8Bit().data(), uiLen);
		m_uiLen += uiLen;

		return *this;
	}

	template <> inline CSerial & operator>>(QString &tOut)
	{
		if (!m_pBuff) {
			throw BUFF_SERIAL_ERR;
		}
		
		tOut = ((const char*)(m_pBuff+m_uiStart));
		
		DelBuff(tOut.size() + 1);

		return *this;
	}

	CSerial &operator<<(QString tStrIn)
	{
		unsigned int uiLen=tStrIn.length()+1;
		AddBuff(uiLen);
		memcpy(m_pBuff+m_uiLen,tStrIn.toLocal8Bit().data(), uiLen);
		m_uiLen += uiLen;
		return *this;
	}

	CSerial & operator>>(QString &tStrOut)
	{
		if (!m_pBuff) {
			throw BUFF_SERIAL_ERR;
		}
		tStrOut = (char*)(m_pBuff+m_uiStart);
		DelBuff(tStrOut.length() + 1);

		return *this;
	}


	
protected:
	int AddBuff(unsigned int uiLenAdd);
	int DelBuff(unsigned int uiLenDel);
	
private:
	unsigned char* m_pBuff;
	unsigned int   m_uiStart;
	unsigned int   m_uiLen;
	unsigned int   m_uiSize;
	CSerial(const CSerial&); ///涓嶈兘浣跨敤鎷疯礉鏋勯€犲嚱鏁?
	CSerial& operator=(const CSerial&); ///涓嶈兘浣跨敤璧嬪€兼搷浣滅
};



#endif