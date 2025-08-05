#include "Serial.h"
//#include "../ACException.h"
#include "ComTool.h"

CSerial::CSerial(void)
{
	m_CharSet=CHARSET_MBCS;
	m_dwLen=0;
	m_dwSize=0;
	m_dwStart=0;
	m_pBuff=NULL;
}

CSerial::CSerial( uint32_t CharSet )
{
	m_CharSet=CharSet;
	m_dwLen=0;
	m_dwSize=0;
	m_dwStart=0;
	m_pBuff=NULL;
}

CSerial::~CSerial(void)
{
	if (m_pBuff) {
		delete []m_pBuff;
		m_pBuff = NULL;
	}
}

int CSerial::IncreaseBuffer(uint32_t dwLenAdd)
{
	int Ret=0;
	//内存不足,重新分配内存
	if (m_dwStart + m_dwLen + dwLenAdd > m_dwSize) {
		m_dwSize = ((m_dwLen + dwLenAdd)+SERIAL_MALLOC_SIZE-1)/SERIAL_MALLOC_SIZE*SERIAL_MALLOC_SIZE; //计算新的大小

		if(m_dwSize==0){
			//PrintLog(LOGLEVEL_ERR,"IncreaseBuffer Error: dwSize=0");
			Ret=-1; goto __end;
		}

		LPBYTE lpBuff = new uint8_t[m_dwSize];
		if (!lpBuff) {
			//PrintLog(LOGLEVEL_ERR,"Append Buffer Error: Malloc New Buffer Failed");
			Ret=-1;goto __end;
		}	
		ZeroMemory(lpBuff,m_dwSize);
		if (m_pBuff) {	//避免最初未初始化m_pBuff的情况
			memcpy(lpBuff, m_pBuff + m_dwStart, m_dwLen);
			delete m_pBuff;
		}
		m_pBuff = lpBuff;
		m_dwStart = 0;
		MemoryBarrier(); ///内存屏障，这个地方一定要等new操作返回
	}

__end:
	return Ret;
}


int CSerial::DecreaseBuffer(uint32_t dwLenDel)
{
	int Ret=0;
	if(dwLenDel>m_dwLen){
		//PrintLog(LOGLEVEL_ERR,"Delete Buffer Error: Buffer Residue=%d, Delete Length=%d",m_dwLen,dwLenDel);
		Ret=-1;
		goto __end;
	}

	m_dwStart += dwLenDel;
	m_dwLen -= dwLenDel;

	if (m_dwStart > SERIAL_MALLOC_SIZE) {
		m_dwSize = (m_dwLen+SERIAL_MALLOC_SIZE-1)/SERIAL_MALLOC_SIZE* SERIAL_MALLOC_SIZE;
		if(m_dwSize==0){// Buffer中的数据已经全部串行化移出
			ReInit();
		}
		else{
			LPBYTE lpBuff = new uint8_t[m_dwSize];
			if (!lpBuff) {
				//PrintLog(LOGLEVEL_ERR,"Delete Buffer Error: Malloc New Buffer Failed");
				Ret=-1;
				goto __end;
			}
			ZeroMemory(lpBuff,m_dwSize);
			memcpy(lpBuff, m_pBuff + m_dwStart,m_dwLen);
			delete m_pBuff;
			m_pBuff = lpBuff;
			m_dwStart = 0;
			MemoryBarrier(); ///内存屏障，这个地方一定要等new操作返回
		}
		
	}
__end:
	return Ret;
}

///将字符串转为Unicode编码存放到内存中
///返回值 0 表示失败，1表示成功
int CSerial::SerialStringInBuf_Unicode( const std::string &tStrIn )
{
	int Ret=0;
	wchar_t *strUnicode=NULL;
	DWORD Size=256,SizeUsed;
	while(1){
		strUnicode=new wchar_t[Size];
		if(!strUnicode){
			//PrintLog(LOGLEVEL_ERR,"Serial String In Buffer, String=%s,CharSet=%d",tStrIn,m_CharSet);
			/*CACException DevExcp;
			DevExcp.PrintLog(LOGLEVEL_ERR,"SerialIn String  Error: Memory Alloc");
			throw DevExcp;*/
		}
		memset(strUnicode,0,Size*sizeof(wchar_t));
		Ret=ComTool::MByteToWChar((LPCSTR)tStrIn.c_str(),(LPWSTR)strUnicode,Size,SizeUsed);
		if(Ret==-1){///内存不足
			delete [] strUnicode;
			Size +=256;
			continue;
		}
		else if(Ret==1){///成功
			if(IncreaseBuffer(SizeUsed*sizeof(wchar_t))==0){///注意一个Unicode占2个字符，所以需要成倍
				memcpy(m_pBuff+m_dwLen,(uint8_t*)strUnicode, SizeUsed*sizeof(wchar_t));
				m_dwLen += SizeUsed*sizeof(wchar_t);
			}
			else{
				Ret=0;///失败
			}
			break;
		}
		else{///失败
			break;
		}
	}
	if(strUnicode){
		delete [] strUnicode;
	}
	return Ret;
}

///将内存中编码为Unicode的字符串提取出来
///返回值 0 表示失败，1表示成功
int CSerial::SerialStringOutBuf_Unicode( std::string &tStrOut )
{
	int Ret=1;
	int ByteCnt=0;
	uint32_t StartPos=m_dwStart;
	uint8_t *pStrMBCS=NULL;
	DWORD Size=256,SizeUsed=0;
	///得到UTF8编码的字符串长度
	for(StartPos=m_dwStart;StartPos<m_dwStart+m_dwLen-1;StartPos+=2){
		ByteCnt +=2;
		if(m_pBuff[StartPos]==0x00 && m_pBuff[StartPos+1]==0x00){
			break;
		}
	}
	if(ByteCnt==0){
		Ret=0; goto __end;
	}

	while(1){
		pStrMBCS=new uint8_t[Size];
		if(!pStrMBCS){
			Ret=0; goto __end;
		}
		memset(pStrMBCS,0,Size);
		Ret=ComTool::WCharToMByte((LPWSTR)(m_pBuff+m_dwStart),(LPSTR)pStrMBCS,Size,SizeUsed);
		if(Ret==-1){///内存不足
			delete[] pStrMBCS;
			Size +=256;
			continue;
		}
		else if(Ret==1){///转换成功
			tStrOut = (char*)pStrMBCS;
			DecreaseBuffer(ByteCnt);///去掉UniCode编码占用的长度
			break;
		}
		else{///转换失败
			break;
		}
	}

__end:
	if(pStrMBCS){
		delete[] pStrMBCS;
	}
	return Ret;
}

///将字符串转为UTF8编码存放到内存中
///返回值 0 表示失败，1表示成功
int CSerial::SerialStringInBuf_Utf8( const std::string &tStrIn )
{
	int Ret=0;
	uint8_t *strUtf8=NULL;
	DWORD Size=256,SizeUsed;
	while(1){
		strUtf8=new uint8_t[Size];
		if(!strUtf8){
			/*PrintLog(LOGLEVEL_ERR,"Serial String In Buffer, String=%s,CharSet=%d",tStrIn,m_CharSet);
			CACException DevExcp;
			DevExcp.PrintLog(LOGLEVEL_ERR,"SerialIn String  Error: Memory Alloc");
			throw DevExcp;*/
		}
		memset(strUtf8,0,Size);
		Ret=ComTool::MByteToUtf8((LPCSTR)tStrIn.c_str(),(LPSTR)strUtf8,Size,SizeUsed);
		if(Ret==-1){///内存不足
			delete [] strUtf8;
			Size +=256;
			continue;
		}
		else if(Ret==1){///成功
			if(IncreaseBuffer(SizeUsed)==0){
				memcpy(m_pBuff+m_dwLen,strUtf8, SizeUsed);
				m_dwLen += SizeUsed;
			}
			else{
				Ret=0;///失败
			}
			break;
		}
		else{///失败
			break;
		}
	}
	if(strUtf8){
		delete[] strUtf8;
	}
	return Ret;
}


///将内存中编码为Utf8的字符串提取出来
///返回值 0 表示失败，1表示成功
int CSerial::SerialStringOutBuf_Utf8( std::string &tStrOut )
{
	int Ret=1;
	int ByteCnt=0;
	uint32_t StartPos=m_dwStart;
	uint8_t *pStrMBCS=NULL;
	DWORD Size=256,SizeUsed=0;
	///得到UTF8编码的字符串长度
	for(StartPos=m_dwStart;StartPos<m_dwStart+m_dwLen;++StartPos){
		ByteCnt++;
		if(m_pBuff[StartPos]==0x00){
			break;
		}
	}

	if(ByteCnt==0){
		//PrintLog(LOGLEVEL_ERR,"Serial Out String Failed:  ByteCnt=0");
		Ret=0; goto __end;
	}
	
	while(1){
		pStrMBCS=new uint8_t[Size];
		if(!pStrMBCS){
			Ret=0; goto __end;
		}
		memset(pStrMBCS,0,Size);
		Ret=ComTool::Utf8ToMByte((LPCSTR)(m_pBuff+m_dwStart),(LPSTR)pStrMBCS,Size,SizeUsed);
		if(Ret==-1){///内存不足
			delete[] pStrMBCS;
			Size +=256;
			continue;
		}
		else if(Ret==1){///转换成功
			tStrOut = (char*)pStrMBCS;
			DecreaseBuffer(ByteCnt);///去掉UTF8编码占用的长度
			//CLogMsg::PrintDebugString("Serial String Out ByteCnt=%d,dwStart=%d,dwLen=%d",ByteCnt,m_dwStart,m_dwLen);
			break;
		}
		else{///转换失败
			//PrintLog(LOGLEVEL_ERR,"Serial Out String Failed: Translate UTF8 To MBCS Failed");
			break;
		}
	}

__end:
	if(pStrMBCS){
		delete[] pStrMBCS;
	}
	if(Ret!=1){
		//PrintLog(LOGLEVEL_ERR,"Serial Out String Failed,dwStart=%d,dwLen=%d",m_dwStart,m_dwLen);
	}
	return Ret;
}

CSerial & CSerial::SerialInBuff(LPBYTE lpBuff, uint32_t uiLen)
{	
	if(IncreaseBuffer(uiLen)!=0){
		/*CACException DevExcp;
		DevExcp.PrintLog(LOGLEVEL_ERR,"IncreaseBuffer Error");
		throw DevExcp;*/
	}
	memcpy(m_pBuff+m_dwLen, lpBuff, uiLen);
	m_dwLen += uiLen;
	return *this;
}

CSerial & CSerial::SerialOutBuff(LPBYTE lpBuff, uint32_t uiLen)
{
	if(!m_pBuff || uiLen> m_dwLen){
		/*PrintLog(LOGLEVEL_ERR,"Serial Out Buffer Error: Buffer=%p,BytesResidue=%d OutBytes=%d",m_pBuff,m_dwLen,uiLen);
		CACException DevExcp;
		DevExcp.PrintLog(LOGLEVEL_ERR,"SerialOutBuff Error");
		throw DevExcp;*/
	}
	memcpy(lpBuff, m_pBuff+m_dwStart, uiLen);

	if(DecreaseBuffer(uiLen)!=0){
		/*CACException DevExcp;
		DevExcp.PrintLog(LOGLEVEL_ERR,"SerialOutBuff Error");
		throw DevExcp;*/
	}
	return *this;
}


int CSerial::ReInit()
{
	if (m_pBuff) {
		delete []m_pBuff;
		m_pBuff = NULL;
	}
	m_dwLen  = 0;
	m_dwSize = 0;
	m_dwStart = 0;

	return 0;
}

LPBYTE CSerial::GetBuffer()
{
	return m_pBuff + m_dwStart;
}

uint32_t CSerial::length()
{
	return m_dwLen;
}