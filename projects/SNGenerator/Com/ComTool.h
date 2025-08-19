#pragma once
#include <afxcmn.h>
#include <vector>

namespace ComTool
{
	enum{
		ENDIAN_BIG,	///大端
		ENDIAN_LIT,	///小端
	};

	/**********************************************
	@brief 将给定的字符串按照10进制进行转换，转换结果为16进制，例如字符串为255,则转换结果过为0xFF
	@param[in]	str			需要转换的字符串
	@param[in]	EndianType	结果数据存放格式
	@param[out] vDecDataOut	转换结果存放在vDecData中,EndianType=ENDIAN_BIG第一个字节为最高字节,否则第一个字节为最低字节
	@return
	TRUE	转换成功
	FALSE	包含非法字符，转换失败
	************************************************/
	BOOL Str2Dec(CString&str,UINT EndianType,std::vector<BYTE>&vDecData);

	/**********************************************
	@brief 将给定的字符串按照16进制进行转换，转换结果为16进制，例如字符串为255,则转换结果过为0x255
	@param[in]	str			需要转换的字符串
	@param[in]	EndianType	结果数据存放格式
	@param[out] vDecDataOut	转换结果存放在vDecData中,EndianType=ENDIAN_BIG第一个字节为最高字节,否则第一个字节为最低字节
	@return
	TRUE	转换成功
	FALSE	包含非法字符，转换失败
	************************************************/
	BOOL Str2Hex(CString&str,UINT EndianType,std::vector<BYTE>&vDecData);
	BOOL Str2HexNoTruncate( CString&str,UINT EndianType,std::vector<BYTE>&vDecDataOut);

	/**********************************************
	@brief 将给定的数据按照10进制进行转换输出
	@param[in] EndianType	指定pData的数据组织方式
	@param[in] pData	需要转换的数据，EndianType=ENDIAN_BIG时高字节在第一个字节，否则低字节在第一个字节
	@param[in] Size		转转的数据占几个字节
	@param[out] strData	输出的字符串
	@return
	TRUE	转换成功
	FALSE	包含非法字符，转换失败
	************************************************/
	BOOL Dec2Str(UINT EndianType,BYTE*pData,INT Size,CString& strData);

	/**********************************************
	@brief 将给定的数据按照10进制进行转换输出
	@param[in] EndianType	指定vDec的数据组织方式
	@param[in] vDec			需要转换的数据，EndianType=ENDIAN_BIG时高字节在第一个字节，否则低字节在第一个字节
	@param[out] strData		输出的字符串
	@return
	TRUE	转换成功
	FALSE	包含非法字符，转换失败
	************************************************/
	BOOL Dec2Str(UINT EndianType,std::vector<BYTE>&vDec,CString& strData);

	/**********************************************
	@brief 将给定的数据按照16进制进行转换输出
	@param[in] EndianType	决定vHex的数据组织方式 为ENDIAN_BIG时高字节在第一个字节，否则低字节在第一个字节
	@param[in] vHex			需要转换的数据
	@param[out] strData		输出的字符串
	@return
	TRUE	转换成功
	FALSE	包含非法字符，转换失败
	************************************************/
	BOOL Hex2Str(UINT EndianType,std::vector<BYTE>&vHex,CString& strData);

	/**********************************************
	@brief 将给定的数据按照16进制进行转换输出
	@param[in] pData	需要转换的数据，高字节在第一个字节
	@param[in] Size		转转的数据占几个字节
	@param[out] strData	输出的字符串
	@return
	TRUE	转换成功
	FALSE	包含非法字符，转换失败
	************************************************/
	BOOL Hex2Str(UINT EndianType,BYTE*pData,INT Size,CString& strData);


	/**********************************************
	@brief 将给定的两个16进制数据相乘
	@param[in] EndianType 决定pMul1，pMul2，vDecDataOut的数据组织方式
	@param[in] pMul1	乘数，EndianType=ENDIAN_BIG 高字节在第一个字节,否则低字节在第一个字节
	@param[in] Size1	乘数占几个字节
	@param[in] pMul2	被乘数，EndianType=ENDIAN_BIG 高字节在第一个字节,否则低字节在第一个字节
	@param[in] Size2	被乘数占几个字节
	@param[out] vDecDataOut	输出的乘积，EndianType=ENDIAN_BIG 高字节在第一个字节,否则低字节在第一个字节
	@return
	TRUE	成功
	FALSE	失败
	************************************************/
	BOOL MultiBytesMul(UINT EndianType,BYTE*pMul1,INT Size1,BYTE*pMul2,INT Size2,std::vector<BYTE>&vDecData);
	
	/**********************************************
	@brief 将给定的两个16进制数据相乘
	@param[in] EndianType 决定vMul1，vMul2，vDecDataOut的数据组织方式
	@param[in] vMul1	乘数，EndianType=ENDIAN_BIG 高字节在第一个字节,否则低字节在第一个字节
	@param[in] vMul2	被乘数，EndianType=ENDIAN_BIG 高字节在第一个字节,否则低字节在第一个字节
	@param[out] vDecDataOut	输出的乘积，EndianType=ENDIAN_BIG 高字节在第一个字节,否则低字节在第一个字节
	@return
	TRUE	成功
	FALSE	失败
	************************************************/
	BOOL MultiBytesMul(UINT EndianType,std::vector<BYTE>&vMul1,std::vector<BYTE>&vMul2,std::vector<BYTE>&vDecDataOut);

	/**********************************************
	@brief 将给定的两个16进制数据相乘
	@param[in] EndianType 决定pAdd1，pAdd2，vDecDataOut的数据组织方式
	@param[in] pAdd1	加数，EndianType=ENDIAN_BIG 高字节在第一个字节,否则低字节在第一个字节
	@param[in] Size1	加数占几个字节
	@param[in] pAdd2	被加数，EndianType=ENDIAN_BIG 高字节在第一个字节,否则低字节在第一个字节
	@param[in] Size2	被加数占几个字节
	@param[out] vDecDataOut	输出的和，EndianType=ENDIAN_BIG 高字节在第一个字节,否则低字节在第一个字节
	@return
	TRUE	成功
	FALSE	失败
	************************************************/
	BOOL MultiBytesAdd(UINT EndianType,BYTE*pAdd1,INT Size1,BYTE*pAdd2,INT Size2,std::vector<BYTE>&vDecDataOut);
	
	/**********************************************
	@brief 将给定的两个16进制数据相乘
	@param[in] EndianType 决定vAdd1，vAdd2，vDecDataOut的数据组织方式
	@param[in] vAdd1	加数，EndianType=ENDIAN_BIG 高字节在第一个字节,否则低字节在第一个字节
	@param[in] vAdd2	被加数，EndianType=ENDIAN_BIG 高字节在第一个字节,否则低字节在第一个字节
	@param[out] vDecDataOut	输出的和，EndianType=ENDIAN_BIG 高字节在第一个字节,否则低字节在第一个字节
	@return
	TRUE	成功
	FALSE	失败
	************************************************/
	BOOL MultiBytesAdd(UINT EndianType,std::vector<BYTE>&vAdd1,std::vector<BYTE>&vAdd2,std::vector<BYTE>&vDecDataOut);


};