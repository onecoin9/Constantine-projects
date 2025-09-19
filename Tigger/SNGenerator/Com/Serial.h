#pragma once
#include <iostream>
#include <atlstr.h>

#define SERIAL_MALLOC_SIZE (4096)

enum eLogLevel{
	LOGLEVEL_LOG,
	LOGLEVEL_WARNING,
	LOGLEVEL_ERR,
	LOGLEVEL_CRIST,

	LOG_NOSHOW=0x10000000,	///直接输出到log文件中，不打印到屏幕上
};

class CACException
{
public:
	CACException(){};
	void PrintLog( INT LogLevel,const char *fmt,... ){};
	virtual ~CACException(void){};
};

class CSerial
{
public:
	CSerial(void);
public:
	void PrintLog( INT LogLevel,const char *fmt,... ){};
	virtual ~CSerial(void);
	template <typename T> inline CSerial & operator<<(T tIn)
	{
		if(IncreaseBuffer(sizeof(tIn))!=0){
			CACException DevExcp;
			DevExcp.PrintLog(LOGLEVEL_ERR,"SerialIn Error");
			throw DevExcp;
		}

		memcpy(m_pBuff+m_dwLen, (LPBYTE) &tIn, sizeof(tIn));
		m_dwLen += sizeof(tIn);
		return *this;
	}

	template <typename T> inline CSerial & operator >>(T &tOut)
	{
		if(!m_pBuff || sizeof(tOut)> m_dwLen){
			PrintLog(LOGLEVEL_ERR,"SerialOut Error: Buffer=%p,BytesResidue=%d OutBytes=%d",m_pBuff,m_dwLen,sizeof(tOut));
			CACException DevExcp;
			DevExcp.PrintLog(LOGLEVEL_ERR,"Serial >> Error");
			throw DevExcp;
		}
		memcpy((LPBYTE)&tOut,m_pBuff+m_dwStart,sizeof(tOut));
		if(DecreaseBuffer(sizeof(tOut))!=0){
			CACException DevExcp;
			DevExcp.PrintLog(LOGLEVEL_ERR,"SerialOut Error");
			throw DevExcp;
		}
		return *this;
	}


	template <> inline CSerial & operator<<(std::string tIn)
	{
		UINT uiLen =(UINT)tIn.size() + 1;

		if(IncreaseBuffer(uiLen)!=0){
			CACException DevExcp;
			DevExcp.PrintLog(LOGLEVEL_ERR,"SerialIn String Error");
			throw DevExcp;
		}

		memcpy(m_pBuff+m_dwLen, tIn.c_str(), uiLen);
		m_dwLen += uiLen;
		return *this;
	}

	template <> inline CSerial & operator>>(std::string &tOut)
	{
		UINT Size=(UINT)tOut.size() + 1;
		if(!m_pBuff || Size > m_dwLen){
			PrintLog(LOGLEVEL_ERR,"Serial String Error: Buffer=%p,BytesResidue=%d OutBytes=%d",m_pBuff,m_dwLen,Size);
			CACException DevExcp;
			DevExcp.PrintLog(LOGLEVEL_ERR,"SerialOut String  Error");
			throw DevExcp;
			return *this;
		}

		tOut.assign((LPCSTR)(m_pBuff+m_dwStart));
		if(DecreaseBuffer(Size)!=0){			
			CACException DevExcp;
			DevExcp.PrintLog(LOGLEVEL_ERR,"SerialOut String  Error");
			throw DevExcp;
		}
		return *this;
	}

	CSerial &operator<<(const CString &tStrIn)
	{
		UINT uiLen=tStrIn.GetLength()+1;
		IncreaseBuffer(uiLen);
		memcpy(m_pBuff+m_dwLen,(LPSTR)(LPCSTR)tStrIn, uiLen);
		m_dwLen += uiLen;
		return *this;
	}

	CSerial & operator>>(CString &tStrOut)
	{
		if (!m_pBuff) {
			CACException DevExcp;
			DevExcp.PrintLog(LOGLEVEL_ERR,"SerialOut CString  Error");
			throw DevExcp;
		}
		tStrOut.Format("%s",(LPSTR)(m_pBuff+m_dwStart));
		DecreaseBuffer(tStrOut.GetLength() + 1);
		return *this;
	}

	CSerial & SerialOutBuff(LPBYTE lpBuff, UINT uiLen);
	CSerial & SerialInBuff(LPBYTE lpBuff, UINT uiLen);

	INT ReInit();
	LPBYTE GetBuffer();
	UINT GetLength();

protected:
	INT CSerial::IncreaseBuffer(UINT dwLenAdd);
	INT CSerial::DecreaseBuffer(UINT dwLenDel);
private:
	LPBYTE m_pBuff;			///存放数据的Buffer起始地址
	UINT m_dwStart;			///有效数据的起始位置
	UINT m_dwLen;			///有效数据长度
	UINT m_dwSize;			///Buffer的总长度
	CSerial(const CSerial&); ///不能使用拷贝构造函数
	CSerial& operator=(const CSerial&); ///不能使用赋值操作符
};
