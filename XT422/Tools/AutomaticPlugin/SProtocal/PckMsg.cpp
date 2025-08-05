
#include "PckMsg.h"

bool bConfigFileCheckCRC = true;

CPackMsg::CPackMsg()
{
	memset(&m_PckMsg,0,sizeof(tPckMsg));
}

void CPackMsg::ReInitPckMsg(tPckMsg*pPckMsg)
{
	if(pPckMsg->m_pData){
		delete[] pPckMsg->m_pData;
	}
	memset(pPckMsg,0,sizeof(tPckMsg));
}

CPackMsg::~CPackMsg()
{
	ReInitPckMsg(&m_PckMsg);
}

int CPackMsg::CalcCRC(tPckMsg*pPckMsg,uint8_t&CRC)
{
	int i;
	CRC=0;
	CRC +=(pPckMsg->m_PFLAG>>8)&0xFF;
	CRC +=(pPckMsg->m_PFLAG)&0xFF;
	CRC +=pPckMsg->m_PDU;
	CRC +=pPckMsg->m_PLEN;
	for(i=0;i<pPckMsg->m_PLEN;++i){
		CRC +=pPckMsg->m_pData[i];
	}
	return 0;
}

bool CPackMsg::Serial( CTSerial&lSerial )
{
	bool Ret=true;
	uint8_t CRCCalc=0;
	tPckMsg*pPckMsg=&m_PckMsg;
	ReInitPckMsg(pPckMsg);
	try{
		uint8_t TmpMByte;
		uint8_t TmpLByte;
		lSerial>>TmpMByte>>TmpLByte;
		pPckMsg->m_PFLAG=((uint16_t)TmpMByte<<8)|TmpLByte;
		uint8_t len = 0;
		lSerial >> pPckMsg->m_PDU >> len;
		pPckMsg->m_PLEN = len;
		if(pPckMsg->m_PLEN>0){
			pPckMsg->m_pData=new uint8_t[pPckMsg->m_PLEN];
			if(!pPckMsg->m_pData){
				Ret=false; goto __end;
			}
		}
		lSerial.SerialOutBuff(pPckMsg->m_pData,pPckMsg->m_PLEN);
		lSerial>>pPckMsg->m_CRC;
		CalcCRC(pPckMsg,CRCCalc);

		
		if(bConfigFileCheckCRC && CRCCalc!=pPckMsg->m_CRC){  //测试时暂时不考虑CRC比对，
			ReInitPckMsg(pPckMsg);
			Ret=false; goto __end;
		}

	}
	catch (...){
		Ret=false;
	}
__end:
	return Ret;
}

bool CPackMsg::SetMsg(uint16_t PFlag,uint8_t Pdu,uint8_t PLen,uint8_t *pData)
{
	bool Ret=true;
	tPckMsg*pPckMsg=&m_PckMsg;
	ReInitPckMsg(pPckMsg);
	pPckMsg->m_PFLAG=PFlag;
	pPckMsg->m_PDU=Pdu;
	pPckMsg->m_PLEN=PLen;
	if(PLen>0){
		pPckMsg->m_pData=new uint8_t[PLen];
		if(!pPckMsg->m_pData){
			Ret=false; goto __end;
		}
		memcpy(pPckMsg->m_pData,pData,PLen);
	}
__end:
	return Ret;
}

bool CPackMsg::UnSerial( CTSerial&lSerial )
{
	bool Ret=true;
	tPckMsg*pPckMsg=&m_PckMsg;
	CalcCRC(pPckMsg,pPckMsg->m_CRC);
	try{
		uint8_t Tmp;
		Tmp=(pPckMsg->m_PFLAG>>8)&0xFF;
		lSerial<<Tmp;
		Tmp=(pPckMsg->m_PFLAG)&0xFF;
		lSerial<<Tmp<<pPckMsg->m_PDU<<pPckMsg->m_PLEN;
		if(pPckMsg->m_PLEN>0){
			if(pPckMsg->m_pData==NULL){
				Ret=false; goto __end;
			}
			else{
				lSerial.SerialInBuff(pPckMsg->m_pData,pPckMsg->m_PLEN);
			}
		}
		lSerial<<pPckMsg->m_CRC;
	}
	catch (...){
		Ret=false;
	}

__end:
	return Ret;
}

uint16_t CPackMsg::GetPFLAG()const
{
	return m_PckMsg.m_PFLAG;
}

uint8_t CPackMsg::GetPDU() const
{
	return m_PckMsg.m_PDU;
}

uint8_t* CPackMsg::GetData(int &PLEN)
{
	PLEN = m_PckMsg.m_PLEN;
	return m_PckMsg.m_pData;
}

uint8_t CPackMsg::GetCRC() const
{
	return m_PckMsg.m_CRC;
}

void CPackMsg::ReInit()
{
	ReInitPckMsg(&m_PckMsg);
}