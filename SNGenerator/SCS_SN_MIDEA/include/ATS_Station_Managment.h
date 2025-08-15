#pragma once
#pragma warning(disable:4996)

#ifdef DLLEXPORT
#define DLLAPI __declspec(dllexport)
#else 
#define DLLAPI __declspec(dllimport)
#endif

#define CALLMETHOD __cdecl//__cdecl//__stdcall

#define _in_ 
#define _out_

/************************************************************************/
/* ��վ����                                                             */
/* 20160923                                                             */
/* Lory                                                               */
/************************************************************************/

//�������ݿ�
DLLAPI int CALLMETHOD TestConnect(_in_ int conn);

//дlog
DLLAPI int CALLMETHOD WriteLog(_in_ char *Errorstring);

//��ȡ�����ļ�·��
DLLAPI int CALLMETHOD Get_ExePath(_out_ char* exePath);

//��ѯ���ϱ���
DLLAPI int CALLMETHOD ATS_SelectMaterialCode(_in_  char * BatchNumber, _out_ char * MaterialCode);

//���������������
DLLAPI int CALLMETHOD ATS_InsertRoutMaintain(
	_in_  char * MaterialCode,
	_in_ char * ATSStationName,
	_in_  char * Test_Number,
	_in_ char * Test_NameVaule
	);

//��ѯ����վ����
DLLAPI int CALLMETHOD ATS_SelectATSStationName(_in_  char * MaterialCode, _out_ char * ATSStationName);

//��ѯSN�Ƿ���Բ���
DLLAPI int CALLMETHOD ATS_SelectTestYorN(
	_in_ char* MainSN,
	_in_ char* ATSStationName,
	_in_ char* MaterialCode,
	_out_ char* TestYorN
	);

DLLAPI int CALLMETHOD ATS_SelectTestYorN_Batch(
	_in_ char* MainSN,
	_in_ char* ATSStationName,
	_in_ char* MaterialCode,
	_in_ char* Batch,
	_out_ char* TestYorN
	);

DLLAPI int CALLMETHOD ATS_SelectTestYorN_Batch_LineName(
	_in_ char* MainSN,
	_in_ char* ATSStationName,
	_in_ char* MaterialCode,
	_in_ char* Batch,
	_in_ char* LineName,
	_in_ char* Value2,
	_in_ char* Value3,
	_out_ char* TestYorN
	);

//�ϴ���������
DLLAPI int CALLMETHOD HW_InsertDataRow(
	_in_ char *strmodel, 
	_in_ char *strbatch,
	_in_ char *strlocation, 
	_in_ char *strstation, 
	_in_ char *strsn, 
	_in_ int  iresult, 
	_in_ char *strkeys,
	_in_ char *strvalues
	);

DLLAPI int CALLMETHOD ATS_InsertDataRow_JIG(
	_in_ char *strmodel,
	_in_ char *strbatch,
	_in_ char *strlocation,
	_in_ char *strstation,
	_in_ char *strsn,
	_in_ int  iresult,
	_in_ char * errorinfo,
	_in_ char * JIG_Number,
	_in_ char *strkeys,
	_in_ char *strvalues
	);

DLLAPI int CALLMETHOD ATS_InsertDataRow_JIG_V2(
	_in_ char *strlinename,//�߱�����
	_in_ char *strbatch,//����
	_in_ char *strlocation,//���ϱ���
	_in_ char *strstation,//վ��
	_in_ char *strsn,//��վ����
	_in_ int  iresult,//���Խ��
	_in_ char * errorinfo,//������Ϣ
	_in_ char * JIG_Number,//���ܱ��
	_in_ char *strswversion,//����汾
	_in_ char *strremark, //��ע
	_in_ char *strkeys,//����
	_in_ char *strvalues//ֵ
	);

DLLAPI int CALLMETHOD ATS_InsertDataRow(
	_in_ char *strmodel,
	_in_ char *strbatch,
	_in_ char *strlocation,
	_in_ char *strstation,
	_in_ char *strsn,
	_in_ int  iresult,
	_in_ char * errorinfo,
	_in_ char *strkeys,
	_in_ char *strvalues
	);

//ֻ�ϴ���������
DLLAPI int CALLMETHOD MES_OnlyInsertDataRow(
	_in_ char *strmodel,
	_in_ char *strbatch,
	_in_ char *strlocation,
	_in_ char *strstation,
	_in_ char *strsn,
	_in_ int  iresult,
	_in_ char *strkeys,
	_in_ char *strvalues
	);

//�����������������
DLLAPI int CALLMETHOD  ATS_InsertErrorData(int Test_ID, char * Test_errorinfo);

//���������������
DLLAPI int CALLMETHOD  ATS_InsertJIGNumber(int Test_ID, char * JIGNumber);

//����Ƿ�Ϊ���������ϴ�������Ʒϵͳ//�����ӿڸ��£����ֻ��������
DLLAPI int CALLMETHOD  ATS_InsertFailCode(
	_in_ char* Product_Batch,//����
	_in_ char* Product_BarCode,//��վ����
	_in_ char* Product_Line_naem,//��������
	_in_ char* Product_StationName,//վ��
	_in_ int   Product_Repeat_Times=3 //�ز��������Ϊ����3
	);

//����Ƿ�Ϊ���������ϴ�������Ʒϵͳ
DLLAPI int CALLMETHOD  ATS_InsertFailCode_V2(
	_in_ char* Product_Batch,//����
	_in_ char* Product_BarCode,//��վ����
	_in_ char* Product_Line_naem,//��������
	_in_ char* Product_StationName,//վ��
	_in_ char* Product_MaterialCode,//�������/���ϱ���
	_in_ int   Product_Repeat_Times = 3 //�ز��������Ϊ����3
	);

//1���µĹ�վ���ݱ��в�������
DLLAPI int CALLMETHOD ATS_InsertTestData(
	int Test_ID,
	char *Test_Vaule
	);

DLLAPI int CALLMETHOD  ATS_InsertTestResult(
	_in_ char *strbatch,
	_in_ char *strsn,
	_in_ char *strstation,
	_in_ int  iresult
	);

/*

*/
DLLAPI int CALLMETHOD  ATS_InsertTestResult_V2(
	_in_ char *strbatch,//����
	_in_ char *strsn,//��վ����
	_in_ char *strstation,//վ��
	_in_ char *strlinename,//�߱�����
	_in_ char *strlocation,//���ϱ���
	_in_ char *strswversion,//����汾
	_in_ char *strremark,//��ע
	_in_ int  iresult//���Խ��
	);

DLLAPI int CALLMETHOD ATS_InsertDataRow_OFFLINE_V2(
	_in_ char *strlinename,//�߱�����
	_in_ char *strbatch,//����
	_in_ char *strlocation,//���ϱ���
	_in_ char *strstation,//վ��
	_in_ char *strsn,//��վ����
	_in_ int  iresult,//���Խ��
	_in_ char * errorinfo,//������Ϣ
	_in_ char * JIG_Number,//���ܱ��
	_in_ char *strswversion,//����汾
	_in_ char *strremark, //��ע
	_in_ char *strkeys,//����
	_in_ char *strvalues//ֵ
	);

/*
select mes_station_ctrl_test.inserttestresult_offline(p_tracenum       => '',--����
p_ser_no         => '',--ɨ����
p_seq_no         => '',--��վ
p_prod_line_name => '',--�����ߣ�������
p_item_code      => '',--���ϱ��루������
p_soft_ver       => '',--����汾��������
p_remark         => '',--��ע��������
P_result         => ''--���
) FROM dual
*/
DLLAPI int CALLMETHOD  ATS_OffLine_InsertResult_V2(
	_in_ char *strbatch,
	_in_ char *strsn,
	_in_ char *strstation,
	_in_ char *strlinename,
	_in_ char *strlocation,
	_in_ char *strswversion,
	_in_ char *strremark,
	_in_ int  iresult
	);

/************************************************************************/
/* ��վ����                                                             */
/* 20160923                                                             */
/* Lory                                                               */
/************************************************************************/

/************************************************************************/
/* �ϴ�����                                                             */
/* 20170104                                                             */
/* Lory                                                               */
/************************************************************************/


//������������
DLLAPI int CALLMETHOD InsertTwoData(
	_in_ char* TableName,
	_in_ char* FieldName,
	_in_ char* Value,
	_in_ char* FieldName2,
	_in_ char* Value2
	);


//�������ݣ���������
DLLAPI int CALLMETHOD UpdateDataTwoCondition(
	_in_ char* TableName,
	_in_ char* FieldName2Update,
	_in_ char* Value2Update,
	_in_ char* FieldNameOfCondition,
	_in_ char* ValueOfCondition,
	_in_ char* FieldNameOfCondition2,
	_in_ char* ValueOfCondition2
	);

//�������ݣ���������
DLLAPI int CALLMETHOD UpdateOneDataTwoCondition(
	_in_ char* TableName,
	_in_ char* Update_ColName,
	_in_ char* Update_ColVaule,
	_in_ char* FieldNameOfCondition,
	_in_ char* ValueOfCondition,
	_in_ char* FieldNameOfCondition2,
	_in_ char* ValueOfCondition2
	);

//��ȡ����,������򣬻�ȡ��һ�����
DLLAPI int CALLMETHOD GetDataOrderBy(
	_in_ char* TableName,
	_in_ char* FieldName,
	_out_ char* Value,
	_in_ int SizeOfValue,
	_in_ char* FieldNameOfCondition,
	_in_ char* ValueOfCondition,
	_in_ char* OrderByAscOrDesc//DESC ���� ASC ����
	);

//����һ������
DLLAPI int CALLMETHOD InsertOneRowData(
	_in_ char* INSERT_ColName,
	_in_ char* INSERT_ColVaule,
	_in_ char* TableName
	);

//����ָ���е�����
DLLAPI int CALLMETHOD UpdateOneRowData(
	_in_ char* Update_ColName,
	_in_ char* Update_ColVaule,
	_in_ char* TableName,
	_in_ char* FieldNameOfCondition,
	_in_ char* ValueOfCondition
	);

//��ѯָ���е�����
DLLAPI int CALLMETHOD SelectOneRowData(
	_in_ char* Select_ColName,
	_out_ char* Select_ColVaule,
	_in_ char* TableName,
	_in_ char* FieldNameOfCondition,
	_in_ char* ValueOfCondition
	);

//SLM-D MES wisillica �����ϴ�
DLLAPI int CALLMETHOD Insert_WisillicaData(
	_in_ char* Wisillica_Batch,
	_in_ char* Wisillica_SN,
	_in_ char* Wisillica_SWversion,
	_in_ char* Wisillica_RSSI,
	_in_ char* Wisillica_VCCorLDA_MAX,
	_in_ char* Wisillica_GNDorLDA_MIN,
	_in_ char* Wisillica_GPIO1orPIR,
	_in_ char* Wisillica_GPIO2,
	_in_ char* Wisillica_RX,
	_in_ char* Wisillica_TX,
	_in_ char* Wisillica_Channel1orSwitch1,
	_in_ char* Wisillica_Channel_50,
	_in_ char* Wisillica_Channel_100,
	_in_ char* Wisillica_channel2orSwitch2,
	_in_ char* Wisillica_Channe2_50,
	_in_ char* Wisillica_Channe2_100,
	_in_ char* Wisillica_RED,
	_in_ char* Wisillica_GREEN,
	_in_ char* Wisillica_BLUE,
	_in_ char* Wisillica_TEMPERATURE,
	_in_ char* Wisillica_HUMIDITY,
	_in_ char* Wisillica_okflag
	);
/************************************************************************/
/* �ϴ�����                                                             */
/* 20170104                                                               */
/* Lory                                                               */
/************************************************************************/

/************************************************************************/
/* MAC&KEY                                                             */
/* 2017411                                                               */
/* Lory                                                               */
/************************************************************************/

//ͨ��MAC�����Σ�����,��ȡSN
DLLAPI int CALLMETHOD Product_GetSNorMAC(
	_in_ char* WifiMAC,
	_in_ char* Product_Batch,
	_in_ char* Product_Type,
	_out_ char* SN_Value
	);

//��ȡ���εĵ�һ���������һ��MAC
DLLAPI int CALLMETHOD MES_GetMACorSN_OrderByData(
	_in_ char *StrBatch,
	_in_ char *StrType,
	_in_ char *OrderByAscOrDesc,
	_out_ char* SN_Value
	);

//ͨ��SN�����Σ�����,��ȡ����������MAC
DLLAPI int CALLMETHOD Product_GetTwoSNorMAC(
	_in_ char* WifiMAC,
	_in_ char* Product_Batch,
	_in_ char* Product_Type,
	_out_ char* SN_Value
	);

//��SN�����Σ�����,MAC�γ�һ����¼
//SN_Value_Two ���SN��
//SN_Value_One ���ONE��
//���в���ǿ�ƴ�д
//select mes_otherdep_call_pkg.SnMacBind_ZJ('HY1731034','ZJ_SN', 'R273050217234', '00:22:6C:87:E5:D0') from dual
DLLAPI int CALLMETHOD Product_SnMacBind(
	_in_ char* Product_Batch,
	_in_ char* Product_Type,
	_in_ char* SN_Value_One,
	_in_ char* SN_Value_Two
	);

//��SN�����µ�MAC��ֵ���γ�һ����¼
//select mes_otherdep_call_pkg.f_BindSN_SN('batch','type', 'sn', 'bind_sn') from dual;
DLLAPI int CALLMETHOD Product_SnBD_BindSN(
	_in_ char* Product_Batch,
	_in_ char* Product_Type,
	_in_ char* SN_Value,
	_in_ char* Bind_SN
	);

//��SN����ȡBindSN
//select mes_otherdep_call_pkg.getBindSnbySN('6C5AB5532276') from dual;
DLLAPI int CALLMETHOD Product_SnGet_BindSN(
	_in_ char* SN_Value,
	_out_ char* Bind_SN
	);

//�ù����룬��ȡMAC��������Դ��Ͷ��λ�Ĺ���ɨ��
DLLAPI int CALLMETHOD Product_OvercodeGet_MAC(
	_in_ char* Overcode_Value,
	_in_ char* Batch_Value,
	_out_ char* Scan_MAC
	);

//��BIND_SN�����Σ�MAC�����򣬻�ȡSN��
DLLAPI int CALLMETHOD Product_BindSNGet_NewSN(
	_in_ char* Customer_Value,
	_in_ char* Batch_Value,
	_in_ char* Type_Value,
	_in_ char* SNPFX_Value,
	_in_ int SN_Length,
	_in_ char* BIND_SN,
	_out_ char* New_SN
	);

// ��BindSN����ȡSN
// 	select sn from mes_keymac_maintain  where BIND_SN='740447A29003F';
// 	DLLAPI int CALLMETHOD Product_BindSN_GetSN(
// 		_out_ char* SN_Value,
// 		_out_ char* Product_Batch,
// 		_in_ char* Bind_SN
// 		);

//��ͨ��SN�����Σ�����,��ȡKEY
//select MES_OtherDep_Call_pkg.f_GetKEYBySN('OP1751002MA00001TL','OP1751002',  'KEY')from dual
DLLAPI int CALLMETHOD Product_BindSN_GetKEY(
	_in_ char* PSN_Value,
	_in_ char* Product_Batch,
	_in_ char* Product_Type,
	_out_ char* KEY_Value,
	_in_ int KEY_Value_Size
	);

//��ͨ��SN�����Σ�����,��ȡ�����Ƶ�KEY
//select MES_OtherDep_Call_pkg.f_GetBinKEYBySN('70C94EB00D21','STG181601','KEY') from dual
DLLAPI int CALLMETHOD Product_SN_GetBINKEY(
	_in_ char* PSN_Value,
	_in_ char* Product_Batch,
	_in_ char* Product_Type,
	_out_ char* KEY_Value,
	_in_ int KEY_Value_Size
	);

//��ͨ��SN�����Σ�����,��ȡ�����Ƶ�KEY
//select MES_OtherDep_Call_pkg.f_GetBinKEYBySN('70C94EB00D21','STG181601','KEY') from dual
DLLAPI int CALLMETHOD Product_SN_GetBINKEY_SaveFile(
	_in_ char* PSN_Value,
	_in_ char* Product_Batch,
	_in_ char* Product_Type,
	_in_ char* Product_SavePath,
	_out_ char* KEY_Value,
	_in_ int KEY_Value_Size
	);

//��ͨ��Bind SN�����Σ�����,��ȡ�����Ƶ�KEY
// select mes_otherdep_call_pkg.f_GetBinKEYByScancode('ABC','STG181601','KEY') from dual
DLLAPI int CALLMETHOD Product_BindSN_GetBINKEY(
	_in_ char* PSN_Value,
	_in_ char* Product_Batch,
	_in_ char* Product_Type,
	_out_ char* KEY_Value,
	_in_ int KEY_Value_Size
	);

//��ͨ��SN�����Σ�����,��ȡָ����KEY
//select MES_OtherDep_Call_pkg.f_GetALDDKEYBySN('0104IBW180101N000001', 'GTR180706','KEY') from dual
DLLAPI int CALLMETHOD Product_SN_GetKEY(
	_in_ char* PSN_Value,
	_in_ char* Product_Batch,
	_in_ char* Product_Type,
	_out_ char* KEY_Value,
	_in_ int KEY_Value_Size
	);

//��ȡKEY�����ݹ�ϣֵ
//select dbms_utility.get_hash_value(
//'2EFB1BDC30FF4707*304502210082780E7EE9A33C854C846C32FE244AEAFFE3D7C9DCC990993A98885AB84D102A0220537CA4973A2E54896DCC13FB0ABE547135F5CC1A630A8F1FB67B4E5141339F74',
//1,1000000000) as l_hash from dual
DLLAPI int CALLMETHOD Product_GetKEY_DBHashValue(
	_in_ char* KEY_Value,
	_out_ char* Hash_Value
	);

//��Ϊ���ƽӿ� 2020-08-14  ��ȡ��ӡ�ӿ�
//select mes_script_pkg.getScriptContent(p_trace_num =>'HW2036069',p_script_type =>'EQUIPMENT')from dual;      
//��һ���������� �ڶ������ͣ���EQUIPMENT����PRINT����
DLLAPI int CALLMETHOD Product_GetScriptContent(
	_in_ char* Product_Batch,
	_in_ char* Product_Type,
	_out_ char* KEY_Value,
	_in_ int KEY_Value_Size
	);

//2021-06-24  ��ȡKEY
//select mes_otherdep_call_pkg.f_getBinbyMAC('GTR212501','KEY','305075F60DD4') from dual    
//��һ���������� �ڶ������ͣ���EQUIPMENT����PRINT����
DLLAPI int CALLMETHOD Product_GetKEYBinbyMAC(
	_in_ char* Product_Batch,
	_in_ char* Product_Type,
	_in_ char* Product_MAC,
	_out_ char* KEY_Value,
	_in_ int KEY_Value_Size
	);

/************************************************************************/
/* MAC&KEY                                                             */
/* 2017411                                                               */
/* Lory                                                               */
/************************************************************************/
/************************************************************************/
/* ����򿨻����ݽӿ�                                                   */
/* 20170411                                                             */
/* Lory                                                               */
/************************************************************************/
DLLAPI int CALLMETHOD Ali_GetBatch(
_in_ char* ModelName,
_out_ char* BatchValue
);

DLLAPI int CALLMETHOD Ali_GetSNRange(
	_in_ char* Batch,
	_out_ char* SNRange_Value
	);
/************************************************************************/
/* ����򿨻����ݽӿ�                                                   */
/* 20170411                                                             */
/* Lory                                                               */
/************************************************************************/

/************************************************************************/
/* Ф�����ݿ�DLL��ֲ�����ĺ���                                         */
/* 20170415                                                             */
/* Lory                                                               */
/************************************************************************/
//����һ������
DLLAPI int CALLMETHOD InsertData(
	_in_ char* TableName,
	_in_ char* FieldName,
	_in_ char* Value
	);

//��������
DLLAPI int CALLMETHOD UpdateData(
	_in_ char* TableName,
	_in_ char* FieldName2Update,
	_in_ char* Value2Update,
	_in_ char* FieldNameOfCondition,
	_in_ char* ValueOfCondition
	);

//��ȡ����
DLLAPI int CALLMETHOD GetData(
	_in_ char* TableName,
	_in_ char* FieldName,
	_out_ char* Value,
	_in_ int SizeOfValue,
	_in_ char* FieldNameOfCondition,
	_in_ char* ValueOfCondition
	);

DLLAPI int CALLMETHOD GetDbTimeStamp(
	_out_ char* DbTimeStamp
	);

DLLAPI int CALLMETHOD GetDbTime(
	_out_ char* DbTime
	);

//SONY_BTSPK_ATS����SN�ظ�
DLLAPI int CALLMETHOD BtSpkCheckSN(
	_in_ char* SN,
	_out_ char* YorN
	);

//HW_C1���󶨽��
DLLAPI int CALLMETHOD p_CheckSeqResult(
	_in_ char* Batch,
	_in_ char* SN,
	_out_ char* YorN
	);

//��ѯSONY��Ʒ��Ϣ
DLLAPI int CALLMETHOD ATS_SelectSONYInfo(
	_in_  char * BatchNumber,
	_out_ char * SONYInfo
	);

/************************************************************************/
/* Ф�����ݿ�DLL��ֲ�����ĺ���                                           */
/* 20170415                                                             */
/* Lory                                                               */
/************************************************************************/

/************************************************************************/
/* ��׼����IT�ӿں���                                                  */
/* 20171113                                                             */
/* Lory                                                               */
/************************************************************************/
//DLLAPI int CALLMETHOD ATS_CALL_IT_SQL_API(
//	_in_  char * SQL_API,
//	_out_ char * Result_Str,
//	_in_ int length_max
//	);

/************************************************************************/
/* ��׼����IT�ӿں���                                                */
/* 20171113                                                             */
/* Lory                                                               */
/************************************************************************/