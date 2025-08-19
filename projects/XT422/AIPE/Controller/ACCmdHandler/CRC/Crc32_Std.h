#ifndef _CRC32_STD_H_
#define _CRC32_STD_H_

#include "ICrcCheck.h"
//#include "GlobalDefine.h"
typedef struct _tagChkInfo {
	uint64_t chksum;	//program sum
	uint64_t sumlen;	//used when calculate chksum	
}_CHKINFO;

class CCrc32Std : public ICrcCheck
{
public:
	CCrc32Std() 
	{
		ReInit();
	};
	void ReInit() {
		memset(&m_ChkInfo, 0, sizeof(_CHKINFO));
	}
	std::string GetName() {
		return std::string("CRC32_Std");
	};
	void CalcSubRoutine(uint8_t* Buf, uint32_t Size);
	/// <summary>
	/// 需要传入4个至少4个字节
	/// </summary>
	/// <param name="pChecksum"></param>
	void GetChecksum(uint8_t*pChecksum,int32_t Size);
private:
	_CHKINFO m_ChkInfo;
};

#endif 