#pragma once

//#include "LogMsg.h"
#include <iostream>
//#include "../ACException.h"
//#include <afxstr.h>
#include <windows.h>
#define CHARSET_MBCS		(0)
#define CHARSET_UTF8		(1)
#define CHARSET_UNICODE		(2)

#define SERIAL_MALLOC_SIZE (4096)

class CSerial/*:public CLogMsg*/
{
public:
	CSerial(void);
	CSerial(uint32_t CharSet);
	void SetCharSet(uint32_t CharSet){m_CharSet=CharSet;}
public:
	virtual ~CSerial(void);
	template <typename T> inline CSerial & operator<<(T tIn)
	{
		if(IncreaseBuffer(sizeof(tIn))!=0){
			/*CACException DevExcp;
			DevExcp.PrintLog(LOGLEVEL_ERR,"SerialIn Error");*/
			//throw DevExcp;
		}

		memcpy(m_pBuff+m_dwLen, (LPBYTE) &tIn, sizeof(tIn));
		m_dwLen += sizeof(tIn);
		return *this;
	}

	template <typename T> inline CSerial & operator >>(T &tOut)
	{
		if(!m_pBuff || sizeof(tOut)> m_dwLen){
			/*PrintLog(LOGLEVEL_ERR,"SerialOut Error: Buffer=%p,BytesResidue=%d OutBytes=%d",m_pBuff,m_dwLen,sizeof(tOut));
			CACException DevExcp;
			DevExcp.PrintLog(LOGLEVEL_ERR,"Serial >> Error");
			throw DevExcp;*/
		}
		memcpy((LPBYTE)&tOut,m_pBuff+m_dwStart,sizeof(tOut));
		if(DecreaseBuffer(sizeof(tOut))!=0){
			/*CACException DevExcp;
			DevExcp.PrintLog(LOGLEVEL_ERR,"SerialOut Error");
			throw DevExcp;*/
		}
		return *this;
	}


	template <> inline CSerial & operator<<(std::string tIn)
	{
		uint32_t uiLen =(uint32_t)tIn.size() + 1;

		if(IncreaseBuffer(uiLen)!=0){
			/*CACException DevExcp;
			DevExcp.PrintLog(LOGLEVEL_ERR,"SerialIn String Error");
			throw DevExcp;*/
		}

		memcpy(m_pBuff+m_dwLen, tIn.c_str(), uiLen);
		m_dwLen += uiLen;
		return *this;
	}

	template <> inline CSerial & operator>>(std::string &tOut)
	{
		uint32_t Size=(uint32_t)tOut.size() + 1;
		if(!m_pBuff || Size > m_dwLen){
			/*PrintLog(LOGLEVEL_ERR,"Serial String Error: Buffer=%p,BytesResidue=%d OutBytes=%d",m_pBuff,m_dwLen,Size);
			CACException DevExcp;
			DevExcp.PrintLog(LOGLEVEL_ERR,"SerialOut String  Error");
			throw DevExcp;*/
			return *this;
		}

		tOut.assign((LPCSTR)(m_pBuff+m_dwStart));
		if(DecreaseBuffer(Size)!=0){			
			/*CACException DevExcp;
			DevExcp.PrintLog(LOGLEVEL_ERR,"SerialOut String  Error");
			throw DevExcp;*/
		}
		return *this;
	}

	CSerial &operator<<(const std::string &tStrIn)
	{
		if(m_CharSet==CHARSET_MBCS){
			uint32_t uiLen=tStrIn.length()+1;
			IncreaseBuffer(uiLen);
			memcpy(m_pBuff+m_dwLen,(LPSTR)(LPCSTR)tStrIn.c_str(), uiLen);
			m_dwLen += uiLen;
		}
		else if(m_CharSet==CHARSET_UTF8){
			SerialStringInBuf_Utf8(tStrIn);
		}
		else if(m_CharSet==CHARSET_UNICODE){
			SerialStringInBuf_Unicode(tStrIn);
		}
		else{
			uint32_t uiLen=tStrIn.length()+1;
			IncreaseBuffer(uiLen);
			memcpy(m_pBuff+m_dwLen,(LPSTR)(LPCSTR)tStrIn.c_str(), uiLen);
			m_dwLen += uiLen;
		}
		return *this;
	}

	CSerial & operator>>(std::string &tStrOut)
	{
		if (!m_pBuff) {
			/*CACException DevExcp;
			DevExcp.PrintLog(LOGLEVEL_ERR,"SerialOut std::string  Error");
			throw DevExcp;*/
		}
		if(m_CharSet==CHARSET_MBCS){
			tStrOut = (LPSTR)(m_pBuff+m_dwStart);
			DecreaseBuffer(tStrOut.length() + 1);
		}
		else if(m_CharSet==CHARSET_UTF8){
			SerialStringOutBuf_Utf8(tStrOut);
		}
		else if(m_CharSet==CHARSET_UNICODE){
			SerialStringOutBuf_Unicode(tStrOut);
		}
		else{
			tStrOut = (LPSTR)(m_pBuff+m_dwStart);
			DecreaseBuffer(tStrOut.length() + 1);
		}
		
		return *this;
	}

	CSerial & SerialOutBuff(LPBYTE lpBuff, uint32_t uiLen);
	CSerial & SerialInBuff(LPBYTE lpBuff, uint32_t uiLen);


	int ReInit();
	LPBYTE GetBuffer();
	uint32_t length();

protected:
	int IncreaseBuffer(uint32_t dwLenAdd);
	int DecreaseBuffer(uint32_t dwLenDel);
	int SerialStringInBuf_Utf8(const std::string &tStrIn);
	int SerialStringOutBuf_Utf8(std::string&tStrOut);

	int SerialStringInBuf_Unicode( const std::string&tStrIn );
	int SerialStringOutBuf_Unicode(std::string&tStrOut);

private:
	uint32_t m_CharSet; ////序列化字符串时使用的字符集
	LPBYTE m_pBuff;			///存放数据的Buffer起始地址
	uint32_t m_dwStart;			///有效数据的起始位置
	uint32_t m_dwLen;			///有效数据长度
	uint32_t m_dwSize;			///Buffer的总长度
	CSerial(const CSerial&); ///不能使用拷贝构造函数
	CSerial& operator=(const CSerial&); ///不能使用赋值操作符
};
