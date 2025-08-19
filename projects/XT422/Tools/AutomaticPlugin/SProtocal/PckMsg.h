#ifndef _PCKMSG_H_
#define _PCKMSG_H_

#include "../../ComStruct/Serial.h"

extern bool bConfigFileCheckCRC;

typedef CSerial CTSerial;
typedef struct tagPckMsg{
	uint16_t m_PFLAG;	///大端存放 'AS' 表示从AutoApp到StdMES， 'SA'表示从StdMES到AutoApp
	uint8_t m_PDU;		///功能码
	uint8_t m_PLEN;	///包长度
	uint8_t *m_pData;	///实际包数据
	uint8_t m_CRC;		///包的校验码
}tPckMsg;

class CPackMsg
{
public:
	CPackMsg();
	~CPackMsg();
	bool Serial(CTSerial&lSerial);
	bool UnSerial(CTSerial&lSerial);
	bool SetMsg(uint16_t PFlag,uint8_t Pdu,uint8_t PLen,uint8_t *pData);
	uint16_t GetPFLAG()const;
	uint8_t GetPDU() const;
	uint8_t*GetData(int &PLEN);
	uint8_t GetCRC() const;
	void ReInit();
private:
	tPckMsg m_PckMsg;
	void ReInitPckMsg(tPckMsg*pPckMsg);
	//计算CRC成功返回0并将CRC值赋值给CRC参数，失败返回-1
	int CalcCRC(tPckMsg*pPckMsg,uint8_t&CRC);
	CPackMsg& operator=(const CPackMsg&);///禁止赋值
	CPackMsg(const CPackMsg&);///禁止拷贝
};


#endif 