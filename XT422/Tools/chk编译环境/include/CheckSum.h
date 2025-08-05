#ifndef __CHECKSUM_H__
#define __CHECKSUM_H__

//#include "IOutput.h"
//#include "DataBuffer.h"
#include <string>
class IDataBuffer;
class IOutput;

//��־����
#define LOGERR (2)			//������־
#define LOGWARNING (1)		//������־
#define LOGNORMAL (0)		//������־
//*******************************************************************//
//�˴���˳���ܸı䣬�����µ�checksum typeʱ��ֻ��˳����������,
//ͬʱfirmware������Ӧ�޸�.
//*******************************************************************//
typedef enum {
	CHECKSUM_NONE   = 0,
	CHECKSUM_BYTE	= 1,
	CHECKSUM_WORD	= 2,
	CHECKSUM_CRC16	= 3,
	CHECKSUM_CRC32  = 4,
}CheckSumType;

typedef struct CheckSumParam{
	uint32_t		 uiCheckSumType;	//�ⲿ����ʱ�ǿͻ�ѡ��Checksum�����±꣬
	//��ΧΪ0-DllGetCkSumName���ص�ֵ,
	//����DllCheckSum֮���ڲ���Ҫ�����޸�ʵ�ʶ�Ӧ�����ͣ�CHECKSUM_WORD��
	
	uint32_t		 uiAgIc;			//������ʹ�õ�IC�㷨���
	const unsigned char  *pcSpcBuffer;		//ConfigBuffer������ָ��
	uint32_t		 uiSpcBuffLen;		//ָ��pcSpcBuffer����ʹ�õ�����ֽ���
	IDataBuffer *pDataBuffer;		//����Buffer����ָ�룬����ֱ��ʹ���ڲ��ĺ�����
	//�����Ҫ���ݣ���ͨ��ReadBuffer�������ö�ȡ
	IOutput     *pOutput;			//��־�������ָ�룬����ֱ��ʹ���ڲ��ĺ���
	//�����Ҫ��ӡ��־��ͨ��PrintLog��������
	uint64_t		 uiVChkSum;			//���ڱ���verify checksum �̶�Ϊcrc16����
	std::string blockJsonString;
}CHECKSUMPARAM;



extern "C" {
	///////////////////////////////////////////
	///@brief : DllGetCkSumName ��ȡChk֧�ֵ�Checksum����
	///@param[in] lpBuff : �ⲿ������ַ����������
	///@param[in] uiLen : lpBuff����ʹ�õĳ���
	///@return : ����֧�ֵ����͸���
	///@note : ÿ�����ͺ�����Ҫ��һ��0x00��Ϊ������Checksum���ƵĽ�����
	///ʾ������������֧��Byte��Word���������ͣ���lpBuff������Ϊ"Byte.Word.",����.Ϊ0x00
	///����ֵΪ2
	///////////////////////////////////////////
	__declspec(dllexport) uint32_t  DllGetCkSumName(char *lpBuff, uint32_t uiLen);

	///////////////////////////////////////////
	///@brief : DllCheckSum ���Buffer��У��ֵ
	///@param[in] pParam���ⲿ����Ľṹ��������ο�CHECKSUMPARAM�ṹ��˵��
	///@return : ����õ���Checksum��ֵ
	///////////////////////////////////////////
	__declspec(dllexport) uint64_t DllCheckSum(CHECKSUMPARAM *pParam);
	
	///////////////////////////////////////////
	///@brief : Std_Checksum_Cal ������CommonChk.lib�ṩ�Ľӿڣ����ڰ�������Checksum
	///@param[in] pParam���ⲿ����Ľṹ��������ο�CHECKSUMPARAM�ṹ��˵��
	///@param[in] adrPos: Buffer����ʼ��ַ
	///@param[in] dwLen: ����ĳ���
	///@param[in] dwCheckSum : Checksum��ʼֵ, ��һ�γ�ʼֵΪ0
	///@param[in] dwMaskValue : Maskֵ��unsigned char,CRC16,CRC32�����ڼ���֮ǰ���ֽ��������ֵ�����������
	///							WORD�����ڼ���֮ǰ����2�ֽ����ֵ���������
	///@param[in,out] dwSumCount: CRC32����ʱ�����Ҫ��ת��ͨ����ֵ��ŵ�ǰ�Ѿ�������ֽ�����
	///								�ⲿ�贫�룬��һ�γ�ʼֵΪ0, CRC32����ʱ���Ҫ����Get_CRC32sum
	///@param[in] showprocessFlag : �Ƿ���Ҫ��ʾ���ȣ�0��ʾ����ʾ��1��ʾ��ʾ
	///@return :����õ���Checksumֵ
	///////////////////////////////////////////
	__declspec(dllexport) uint64_t Std_Checksum_Cal(const  CHECKSUMPARAM * param, unsigned long adrPos, unsigned long dwLen,
		uint64_t dwCheckSum, unsigned long dwMaskValue, unsigned long *dwSumCount,unsigned long showprocessFlag);

	///////////////////////////////////////////
	///@brief : Std_VFCRC16_Checksum_Cal ������CommonChk.lib�ṩ�Ľӿڣ����ڰ�������Verify��Checksum
	///				�̶�ΪCRC16��ʽ
	///�����뷵��ֵ�ο� Std_Checksum_Cal
	//////////////////////////////////////////////////
	__declspec(dllexport) unsigned long Std_VFCRC16_Checksum_Cal(CHECKSUMPARAM * param, unsigned long adrPos, unsigned long dwLen,
		unsigned long uiVChkSum, unsigned long dwMaskValue, unsigned long *dwSumCount,unsigned long showprocessFlag);


	///////////////////////////////////////////
	///@brief : Std_VFCRC64_Checksum_Cal ������CommonChk.lib�ṩ�Ľӿڣ����ڰ�������Verify��Checksum
	/// 			�̶�ΪCRC64��ʽ
	///�����뷵��ֵ�ο� Std_Checksum_Cal �η�����δ��ʵ�ʺ�����Ӧ
	//////////////////////////////////////////////////
	__declspec(dllexport) uint64_t Std_VFCRC64_Checksum_Cal(CHECKSUMPARAM * param, unsigned long adrPos, unsigned long dwLen,
		uint64_t uiVChkSum, unsigned long dwMaskValue, unsigned long *dwSumCount,unsigned long showprocessFlag);
	
	///////////////////////////////////////////
	///@brief : Get_CRC32sum ��ȡCRC32������Checksumֵ���ڵ�����Std_Checksum_Cal֮����Ҫ���øú���
	///@param[in] pParam���ⲿ����Ľṹ��������ο�CHECKSUMPARAM�ṹ��˵��
	///@param[in] dwSumCount �� Std_Checksum_Calͳ�Ƶ��Ѿ������Checksum�ֽ���
	///@return : CRC32������У��ֵ
	///////////////////////////////////////////
  __declspec(dllexport) unsigned long Get_CRC32sum(unsigned long CRC32,unsigned long *dwSumCount);

  ///////////////////////////////////////////
  ///@brief : PrintLog ��ӡ��־
  ///@param[in] pParam���ⲿ����Ľṹ��������ο�CHECKSUMPARAM�ṹ��˵��
  ///@param[in] LogLevel �� ��־����LOGNORMAL��
  ///@param[in] Msg �� ��Ϣ�ַ�����\0��β
  ///@return : ��
  ///////////////////////////////////////////
  __declspec(dllexport) void PrintLog(const  CHECKSUMPARAM * param,int32_t LogLevel,char*Msg);

  ///////////////////////////////////////////
  ///@brief : ReadBuffer��ȡ����Buffer������
  ///@param[in] pParam���ⲿ����Ľṹ��������ο�CHECKSUMPARAM�ṹ��˵��
  ///@param[in] StartAddr: ��ʼBuffer��ַ
  ///@param[in] pData:���ݴ��λ��
  ///@param[in] Size����ȡ���ֽ���
  ///@return ��ʵ�ʶ�ȡ���ֽ���
  ///////////////////////////////////////////
  __declspec(dllexport) int32_t ReadBuffer(const  CHECKSUMPARAM * param,uint64_t StartAddr,unsigned char*pData,int32_t Size);
  __declspec(dllexport) int32_t WriteBuffer(const  CHECKSUMPARAM * param,uint64_t StartAddr,unsigned char*pData,int32_t Size);

  

 //���ý������ܳ���
  __declspec(dllexport) void SetProgress(const  CHECKSUMPARAM * param,uint32_t uiComplete);

   //���ý�����λ��
   __declspec(dllexport) void SetProgPos(const  CHECKSUMPARAM * param,uint32_t uiPos);

   
};


typedef uint32_t (*fnDllGetCkSumName)(char * lpBuff, uint32_t uiLen);
typedef uint64_t (*fnDllCheckSum)(CHECKSUMPARAM *pParam);

#define FUNC_NAME "DllCheckSum"
#define GET_CHECKSUM_NAME "DllGetCkSumName"

#endif