#pragma once

#include "../Com/Serial.h"

class CSNStepCfg
{
public:
	CSNStepCfg();
	~CSNStepCfg();

	BOOL SerialInCfgData(CSerial& lSerial);
	BOOL SerialOutCfgData(CSerial& lSerial);

	void SetStartAddress(UINT64 addr) { addr_ = addr; }
	UINT64 StartAddress() const { return addr_; }

	void SetByteSize(UINT32 bytes) { bytes_ = bytes; }
	UINT32 ByteSize() const { return bytes_; }

private:
	UINT64 addr_;
	UINT32 bytes_;
};
