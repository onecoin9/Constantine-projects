#pragma once
#include <afxcmn.h>
#include <vector>

namespace ComTool
{
	enum{
		ENDIAN_BIG,	///���
		ENDIAN_LIT,	///С��
	};

	/**********************************************
	@brief ���������ַ�������10���ƽ���ת����ת�����Ϊ16���ƣ������ַ���Ϊ255,��ת�������Ϊ0xFF
	@param[in]	str			��Ҫת�����ַ���
	@param[in]	EndianType	������ݴ�Ÿ�ʽ
	@param[out] vDecDataOut	ת����������vDecData��,EndianType=ENDIAN_BIG��һ���ֽ�Ϊ����ֽ�,�����һ���ֽ�Ϊ����ֽ�
	@return
	TRUE	ת���ɹ�
	FALSE	�����Ƿ��ַ���ת��ʧ��
	************************************************/
	BOOL Str2Dec(CString&str,UINT EndianType,std::vector<BYTE>&vDecData);

	/**********************************************
	@brief ���������ַ�������16���ƽ���ת����ת�����Ϊ16���ƣ������ַ���Ϊ255,��ת�������Ϊ0x255
	@param[in]	str			��Ҫת�����ַ���
	@param[in]	EndianType	������ݴ�Ÿ�ʽ
	@param[out] vDecDataOut	ת����������vDecData��,EndianType=ENDIAN_BIG��һ���ֽ�Ϊ����ֽ�,�����һ���ֽ�Ϊ����ֽ�
	@return
	TRUE	ת���ɹ�
	FALSE	�����Ƿ��ַ���ת��ʧ��
	************************************************/
	BOOL Str2Hex(CString&str,UINT EndianType,std::vector<BYTE>&vDecData);
	BOOL Str2HexNoTruncate( CString&str,UINT EndianType,std::vector<BYTE>&vDecDataOut);

	/**********************************************
	@brief �����������ݰ���10���ƽ���ת�����
	@param[in] EndianType	ָ��pData��������֯��ʽ
	@param[in] pData	��Ҫת�������ݣ�EndianType=ENDIAN_BIGʱ���ֽ��ڵ�һ���ֽڣ�������ֽ��ڵ�һ���ֽ�
	@param[in] Size		תת������ռ�����ֽ�
	@param[out] strData	������ַ���
	@return
	TRUE	ת���ɹ�
	FALSE	�����Ƿ��ַ���ת��ʧ��
	************************************************/
	BOOL Dec2Str(UINT EndianType,BYTE*pData,INT Size,CString& strData);

	/**********************************************
	@brief �����������ݰ���10���ƽ���ת�����
	@param[in] EndianType	ָ��vDec��������֯��ʽ
	@param[in] vDec			��Ҫת�������ݣ�EndianType=ENDIAN_BIGʱ���ֽ��ڵ�һ���ֽڣ�������ֽ��ڵ�һ���ֽ�
	@param[out] strData		������ַ���
	@return
	TRUE	ת���ɹ�
	FALSE	�����Ƿ��ַ���ת��ʧ��
	************************************************/
	BOOL Dec2Str(UINT EndianType,std::vector<BYTE>&vDec,CString& strData);

	/**********************************************
	@brief �����������ݰ���16���ƽ���ת�����
	@param[in] EndianType	����vHex��������֯��ʽ ΪENDIAN_BIGʱ���ֽ��ڵ�һ���ֽڣ�������ֽ��ڵ�һ���ֽ�
	@param[in] vHex			��Ҫת��������
	@param[out] strData		������ַ���
	@return
	TRUE	ת���ɹ�
	FALSE	�����Ƿ��ַ���ת��ʧ��
	************************************************/
	BOOL Hex2Str(UINT EndianType,std::vector<BYTE>&vHex,CString& strData);

	/**********************************************
	@brief �����������ݰ���16���ƽ���ת�����
	@param[in] pData	��Ҫת�������ݣ����ֽ��ڵ�һ���ֽ�
	@param[in] Size		תת������ռ�����ֽ�
	@param[out] strData	������ַ���
	@return
	TRUE	ת���ɹ�
	FALSE	�����Ƿ��ַ���ת��ʧ��
	************************************************/
	BOOL Hex2Str(UINT EndianType,BYTE*pData,INT Size,CString& strData);


	/**********************************************
	@brief ������������16�����������
	@param[in] EndianType ����pMul1��pMul2��vDecDataOut��������֯��ʽ
	@param[in] pMul1	������EndianType=ENDIAN_BIG ���ֽ��ڵ�һ���ֽ�,������ֽ��ڵ�һ���ֽ�
	@param[in] Size1	����ռ�����ֽ�
	@param[in] pMul2	��������EndianType=ENDIAN_BIG ���ֽ��ڵ�һ���ֽ�,������ֽ��ڵ�һ���ֽ�
	@param[in] Size2	������ռ�����ֽ�
	@param[out] vDecDataOut	����ĳ˻���EndianType=ENDIAN_BIG ���ֽ��ڵ�һ���ֽ�,������ֽ��ڵ�һ���ֽ�
	@return
	TRUE	�ɹ�
	FALSE	ʧ��
	************************************************/
	BOOL MultiBytesMul(UINT EndianType,BYTE*pMul1,INT Size1,BYTE*pMul2,INT Size2,std::vector<BYTE>&vDecData);
	
	/**********************************************
	@brief ������������16�����������
	@param[in] EndianType ����vMul1��vMul2��vDecDataOut��������֯��ʽ
	@param[in] vMul1	������EndianType=ENDIAN_BIG ���ֽ��ڵ�һ���ֽ�,������ֽ��ڵ�һ���ֽ�
	@param[in] vMul2	��������EndianType=ENDIAN_BIG ���ֽ��ڵ�һ���ֽ�,������ֽ��ڵ�һ���ֽ�
	@param[out] vDecDataOut	����ĳ˻���EndianType=ENDIAN_BIG ���ֽ��ڵ�һ���ֽ�,������ֽ��ڵ�һ���ֽ�
	@return
	TRUE	�ɹ�
	FALSE	ʧ��
	************************************************/
	BOOL MultiBytesMul(UINT EndianType,std::vector<BYTE>&vMul1,std::vector<BYTE>&vMul2,std::vector<BYTE>&vDecDataOut);

	/**********************************************
	@brief ������������16�����������
	@param[in] EndianType ����pAdd1��pAdd2��vDecDataOut��������֯��ʽ
	@param[in] pAdd1	������EndianType=ENDIAN_BIG ���ֽ��ڵ�һ���ֽ�,������ֽ��ڵ�һ���ֽ�
	@param[in] Size1	����ռ�����ֽ�
	@param[in] pAdd2	��������EndianType=ENDIAN_BIG ���ֽ��ڵ�һ���ֽ�,������ֽ��ڵ�һ���ֽ�
	@param[in] Size2	������ռ�����ֽ�
	@param[out] vDecDataOut	����ĺͣ�EndianType=ENDIAN_BIG ���ֽ��ڵ�һ���ֽ�,������ֽ��ڵ�һ���ֽ�
	@return
	TRUE	�ɹ�
	FALSE	ʧ��
	************************************************/
	BOOL MultiBytesAdd(UINT EndianType,BYTE*pAdd1,INT Size1,BYTE*pAdd2,INT Size2,std::vector<BYTE>&vDecDataOut);
	
	/**********************************************
	@brief ������������16�����������
	@param[in] EndianType ����vAdd1��vAdd2��vDecDataOut��������֯��ʽ
	@param[in] vAdd1	������EndianType=ENDIAN_BIG ���ֽ��ڵ�һ���ֽ�,������ֽ��ڵ�һ���ֽ�
	@param[in] vAdd2	��������EndianType=ENDIAN_BIG ���ֽ��ڵ�һ���ֽ�,������ֽ��ڵ�һ���ֽ�
	@param[out] vDecDataOut	����ĺͣ�EndianType=ENDIAN_BIG ���ֽ��ڵ�һ���ֽ�,������ֽ��ڵ�һ���ֽ�
	@return
	TRUE	�ɹ�
	FALSE	ʧ��
	************************************************/
	BOOL MultiBytesAdd(UINT EndianType,std::vector<BYTE>&vAdd1,std::vector<BYTE>&vAdd2,std::vector<BYTE>&vDecDataOut);


};