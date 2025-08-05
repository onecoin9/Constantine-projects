#pragma once
#include <string>
#include <vector>
#include "Serial.h"



namespace ComTool
{
	enum{
		ENDIAN_BIG =1,	///大端
		ENDIAN_LIT =2,	///小端
		PREHEADZERO_NEED=0x0004,///在转换的过程中需要前导0
	};

	/**********************************************
	@brief 将给定的字符串按照10进制进行转换，转换结果为16进制，例如字符串为255,则转换结果过为0xFF
	@param[in]	str			需要转换的字符串
	@param[in]	EndianType	结果数据存放格式
	@param[out] vDecDataOut	转换结果存放在vDecData中,EndianType=ENDIAN_BIG第一个字节为最高字节,否则第一个字节为最低字节
	@return
	true	转换成功
	false	包含非法字符，转换失败
	************************************************/
	bool Str2Dec(std::string &str,uint32_t EndianType,std::vector<uint8_t>&vDecData);

	/**********************************************
	@brief 将给定的字符串按照16进制进行转换，转换结果为16进制，例如字符串为255,则转换结果过为0x255
	@param[in]	str			需要转换的字符串
	@param[in]	EndianType	结果数据存放格式
	@param[out] vDecDataOut	转换结果存放在vDecData中,EndianType=ENDIAN_BIG第一个字节为最高字节,否则第一个字节为最低字节
	@return
	true	转换成功
	false	包含非法字符，转换失败
	************************************************/
	bool Str2Hex(std::string&str,uint32_t EndianType,std::vector<uint8_t>&vDecData);

	/**********************************************
	@brief 将给定的数据按照10进制进行转换输出
	@param[in] EndianType	指定pData的数据组织方式
	@param[in] pData	需要转换的数据，EndianType=ENDIAN_BIG时高字节在第一个字节，否则低字节在第一个字节
	@param[in] Size		转转的数据占几个字节
	@param[out] strData	输出的字符串
	@return
	true	转换成功
	false	包含非法字符，转换失败
	************************************************/
	bool Dec2Str(uint32_t EndianType,uint8_t*pData,int Size,std::string& strData);

	std::string GetCurTime(char Seperator);

	/**********************************************
	@brief 将给定的数据按照10进制进行转换输出
	@param[in] EndianType	指定vDec的数据组织方式
	@param[in] vDec			需要转换的数据，EndianType=ENDIAN_BIG时高字节在第一个字节，否则低字节在第一个字节
	@param[out] strData		输出的字符串
	@return
	true	转换成功
	false	包含非法字符，转换失败
	************************************************/
	bool Dec2Str(uint32_t EndianType,std::vector<uint8_t>&vDec,std::string& strData);

	/**********************************************
	@brief 将给定的数据按照16进制进行转换输出
	@param[in] EndianType	决定vHex的数据组织方式 为ENDIAN_BIG时高字节在第一个字节，否则低字节在第一个字节
	@param[in] vHex			需要转换的数据
	@param[out] strData		输出的字符串
	@return
	true	转换成功
	false	包含非法字符，转换失败
	************************************************/
	bool Hex2Str(uint32_t EndianType,std::vector<uint8_t>&vHex,std::string& strData);
	bool Hex2StrNew(uint32_t EndianType,std::vector<uint8_t>&vHex,std::string& strData);

	/**********************************************
	@brief 将给定的数据按照16进制进行转换输出
	@param[in] pData	需要转换的数据，高字节在第一个字节
	@param[in] Size		转转的数据占几个字节
	@param[out] strData	输出的字符串
	@return
	true	转换成功
	false	包含非法字符，转换失败
	************************************************/
	bool Hex2Str(uint32_t EndianType,uint8_t*pData,int Size,std::string& strData);
	bool Hex2StrNew(uint32_t EndianType,uint8_t*pData,int Size,std::string& strData);


	/**********************************************
	@brief 将给定的两个16进制数据相乘
	@param[in] EndianType 决定pMul1，pMul2，vDecDataOut的数据组织方式
	@param[in] pMul1	乘数，EndianType=ENDIAN_BIG 高字节在第一个字节,否则低字节在第一个字节
	@param[in] Size1	乘数占几个字节
	@param[in] pMul2	被乘数，EndianType=ENDIAN_BIG 高字节在第一个字节,否则低字节在第一个字节
	@param[in] Size2	被乘数占几个字节
	@param[out] vDecDataOut	输出的乘积，EndianType=ENDIAN_BIG 高字节在第一个字节,否则低字节在第一个字节
	@return
	true	成功
	false	失败
	************************************************/
	bool MultiBytesMul(uint32_t EndianType,uint8_t*pMul1,int Size1,uint8_t*pMul2,int Size2,std::vector<uint8_t>&vDecData);
	
	/**********************************************
	@brief 将给定的两个16进制数据相乘
	@param[in] EndianType 决定vMul1，vMul2，vDecDataOut的数据组织方式
	@param[in] vMul1	乘数，EndianType=ENDIAN_BIG 高字节在第一个字节,否则低字节在第一个字节
	@param[in] vMul2	被乘数，EndianType=ENDIAN_BIG 高字节在第一个字节,否则低字节在第一个字节
	@param[out] vDecDataOut	输出的乘积，EndianType=ENDIAN_BIG 高字节在第一个字节,否则低字节在第一个字节
	@return
	true	成功
	false	失败
	************************************************/
	bool MultiBytesMul(uint32_t EndianType,std::vector<uint8_t>&vMul1,std::vector<uint8_t>&vMul2,std::vector<uint8_t>&vDecDataOut);

	/**********************************************
	@brief 将给定的两个16进制数据相乘
	@param[in] EndianType 决定pAdd1，pAdd2，vDecDataOut的数据组织方式
	@param[in] pAdd1	加数，EndianType=ENDIAN_BIG 高字节在第一个字节,否则低字节在第一个字节
	@param[in] Size1	加数占几个字节
	@param[in] pAdd2	被加数，EndianType=ENDIAN_BIG 高字节在第一个字节,否则低字节在第一个字节
	@param[in] Size2	被加数占几个字节
	@param[out] vDecDataOut	输出的和，EndianType=ENDIAN_BIG 高字节在第一个字节,否则低字节在第一个字节
	@return
	true	成功
	false	失败
	************************************************/
	bool MultiBytesAdd(uint32_t EndianType,uint8_t*pAdd1,int Size1,uint8_t*pAdd2,int Size2,std::vector<uint8_t>&vDecDataOut);
	
	/**********************************************
	@brief 将给定的两个16进制数据相乘
	@param[in] EndianType 决定vAdd1，vAdd2，vDecDataOut的数据组织方式
	@param[in] vAdd1	加数，EndianType=ENDIAN_BIG 高字节在第一个字节,否则低字节在第一个字节
	@param[in] vAdd2	被加数，EndianType=ENDIAN_BIG 高字节在第一个字节,否则低字节在第一个字节
	@param[out] vDecDataOut	输出的和，EndianType=ENDIAN_BIG 高字节在第一个字节,否则低字节在第一个字节
	@return
	true	成功
	false	失败
	************************************************/
	bool MultiBytesAdd(uint32_t EndianType,std::vector<uint8_t>&vAdd1,std::vector<uint8_t>&vAdd2,std::vector<uint8_t>&vDecDataOut);

	std::string GetChksumName(uint32_t uChksumType);

	//-------------------------------------------------------------------------------------
	//Description:
	// This function maps a character string to a wide-character (Unicode) string
	//
	//Parameters:
	// lpcszStr: [in] Pointer to the character string to be converted
	// lpwszStr: [out] Pointer to a buffer that receives the translated string.
	// dwSize: [in] Size of the buffer
	// SizeUsed: [out] Size of the buffer be used when translate successfully   ///多少个2个字节单位包含2个字节的0结尾
	//
	//Return Values:
	// 1: Succeed
	// 0: Failed
	// -1：Need More Buffer
	//
	//Example:
	// MByteToWChar(szA,szW,sizeof(szW)/sizeof(szW[0]),SizeUsed);
	//---------------------------------------------------------------------------------------
	int WCharToMByte(LPCWSTR lpcwszStr, LPSTR lpszStr, DWORD dwSize,DWORD& SizeUsed);

	//-------------------------------------------------------------------------------------
	//Description:
	// This function maps a wide-character string to a new character string
	//
	//Parameters:
	// lpcwszStr: [in] Pointer to the character string to be converted
	// lpszStr: [out] Pointer to a buffer that receives the translated string.
	// dwSize: [in] Size of the buffer
	// SizeUsed: [out] Size of the buffer be used when translate successfully   ///多少个字节单位包含一个字节0结尾

	//
	//Return Values:
	// 1: Succeed
	// 0: Failed
	// -1：Need More Buffer
	//
	//Example:
	// MByteToWChar(szW,szA,sizeof(szA)/sizeof(szA[0]),SizeUsed);
	//---------------------------------------------------------------------------------------
	int MByteToWChar(LPCSTR lpcszStr, LPWSTR lpwszStr, DWORD dwSize,DWORD& SizeUsed);

	//-------------------------------------------------------------------------------------
	//Description:
	// This function maps a character string to a new utf8 character string
	//
	//Parameters:
	// lpcszStr: [in] Pointer to the character string to be converted
	// lpszStr: [out] Pointer to a buffer that receives the translated string.
	// dwSize: [in] Size of the lpszStr buffer
	// SizeUsed: [out] Size of the buffer be used when translate successfully   ///多少个字节单位包含一个字节0结尾
	//
	//Return Values:
	// 1: Succeed
	// 0: Failed
	// -1：Need More Buffer
	//
	//Example:
	// MByteToUtf8(szA,szUTF8,sizeof(szUTF8)/sizeof(szUTF8[0]),SizeUsed);
	//---------------------------------------------------------------------------------------
	int MByteToUtf8(LPCSTR lpcszStr, LPSTR lpszStr, DWORD dwSize,DWORD& SizeUsed);


	//-------------------------------------------------------------------------------------
	//Description:
	// This function maps a utf8  character string to a new utf8 character string
	//
	//Parameters:
	// lpcszStr: [in] Pointer to the character string to be converted
	// lpszStr: [out] Pointer to a buffer that receives the translated string.
	// dwSize: [in] Size of the lpszStr buffer
	// SizeUsed: [out] Size of the buffer be used when translate successfully   ///多少个字节单位包含一个字节0结尾

	//
	//Return Values:
	// 1: Succeed
	// 0: Failed
	// -1：Need More Buffer
	//
	//Example:
	// Utf8ToMByte(szUTF8,szA,sizeof(szA)/sizeof(szA[0]),SizeUsed);
	//---------------------------------------------------------------------------------------
	int Utf8ToMByte(LPCSTR lpcszStr, LPSTR lpszStr, DWORD dwSize,DWORD& SizeUsed);

	///提取参数将{}去掉，并在各个成员之间加上\0
	///成功返回true，失败返回false
	bool ExtractParaSet(std::string&strParaData,CSerial&lSerial);

	void SaveDataToFile(CSerial&lSerial,std::string strDestFile);
	void SaveDataToFile(uint8_t*pData,int Size,std::string strDestFile);

	std::string GetCurrentPath(void);

	//==================================================================
	//函数名： Split
	//功能：   Split a cstring to cstring array. "1234 5678" to ["1234","5678"]
	//输入参数：
	//返回值：
	//==================================================================
	void Split(std::string source, std::vector<std::string>& dest, std::string division);
};