#include "Serial.h"

CSerial::CSerial(void)
{
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

INT CSerial::IncreaseBuffer(UINT dwLenAdd)
{
	INT Ret=0;
	//内存不足,重新分配内存
	if (m_dwStart + m_dwLen + dwLenAdd > m_dwSize) {
		m_dwSize = ((m_dwLen + dwLenAdd)+SERIAL_MALLOC_SIZE-1)/SERIAL_MALLOC_SIZE*SERIAL_MALLOC_SIZE; //计算新的大小

		if(m_dwSize==0){
			PrintLog(LOGLEVEL_ERR,"IncreaseBuffer Error: dwSize=0");
			Ret=-1; goto __end;
		}

		LPBYTE lpBuff = new BYTE[m_dwSize];
		if (!lpBuff) {
			PrintLog(LOGLEVEL_ERR,"Append Buffer Error: Malloc New Buffer Failed");
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


INT CSerial::DecreaseBuffer(UINT dwLenDel)
{
	INT Ret=0;
	if(dwLenDel>m_dwLen){
		PrintLog(LOGLEVEL_ERR,"Delete Buffer Error: Buffer Residue=%d, Delete Length=%d",m_dwLen,dwLenDel);
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
			LPBYTE lpBuff = new BYTE[m_dwSize];
			if (!lpBuff) {
				PrintLog(LOGLEVEL_ERR,"Delete Buffer Error: Malloc New Buffer Failed");
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

CSerial & CSerial::SerialInBuff(LPBYTE lpBuff, UINT uiLen)
{	
	if(IncreaseBuffer(uiLen)!=0){
		CACException DevExcp;
		DevExcp.PrintLog(LOGLEVEL_ERR,"IncreaseBuffer Error");
		throw DevExcp;
	}
	memcpy(m_pBuff+m_dwLen, lpBuff, uiLen);
	m_dwLen += uiLen;
	return *this;
}

CSerial & CSerial::SerialOutBuff(LPBYTE lpBuff, UINT uiLen)
{
	if(!m_pBuff || uiLen> m_dwLen){
		PrintLog(LOGLEVEL_ERR,"Serial Out Buffer Error: Buffer=%p,BytesResidue=%d OutBytes=%d",m_pBuff,m_dwLen,uiLen);
		CACException DevExcp;
		DevExcp.PrintLog(LOGLEVEL_ERR,"SerialOutBuff Error");
		throw DevExcp;
	}
	memcpy(lpBuff, m_pBuff+m_dwStart, uiLen);

	if(DecreaseBuffer(uiLen)!=0){
		CACException DevExcp;
		DevExcp.PrintLog(LOGLEVEL_ERR,"SerialOutBuff Error");
		throw DevExcp;
	}
	return *this;
}


INT CSerial::ReInit()
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

UINT CSerial::GetLength()
{
	return m_dwLen;
}