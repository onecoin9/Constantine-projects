#ifndef _CRC32_COMM_H_
#define _CRC32_COMM_H_


#include "ICrcCheck.h"
/**
 * \defgroup common_services_crc32 CRC-32 calculation service
 *
 * See \ref common_services_crc32_quickstart.
 *
 * This service enables the user to calculate 32-bit CRC using the polynomial
 * defined in the IEEE 802.3 standard, with support for multiple data blocks
 * of arbitrary sizes, and any alignment in memory.
 *
 * @{
 */


 //! Type to contain 32-bit CRC.
typedef uint32_t crc32_t;

class CCrc32Comm {
public:
	CCrc32Comm() {
		ReInit();
	}
	void ReInit() {
		m_Crc32 = 0;
	}
	std::string GetName() {
		return std::string("CRC32_Comm");
	};
	void CalcSubRoutine(uint8_t* Buf, uint32_t Size);
	/// <summary>
	/// 需要传入4个至少4个字节
	/// </summary>
	/// <param name="pChecksum"></param>
	void GetChecksum(uint8_t* pChecksum, int32_t Size);

private:
	crc32_t m_Crc32;
};

#endif 