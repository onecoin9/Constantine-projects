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
/* 过站管理                                                             */
/* 20160923                                                             */
/* Lory                                                               */
/************************************************************************/

//连接数据库
DLLAPI int CALLMETHOD TestConnect(_in_ int conn);

//写log
DLLAPI int CALLMETHOD WriteLog(_in_ char *Errorstring);

//获取配置文件路径
DLLAPI int CALLMETHOD Get_ExePath(_out_ char* exePath);

//查询物料编码
DLLAPI int CALLMETHOD ATS_SelectMaterialCode(_in_  char * BatchNumber, _out_ char * MaterialCode);

//插入测试序列名称
DLLAPI int CALLMETHOD ATS_InsertRoutMaintain(
	_in_  char * MaterialCode,
	_in_ char * ATSStationName,
	_in_  char * Test_Number,
	_in_ char * Test_NameVaule
	);

//查询测试站名称
DLLAPI int CALLMETHOD ATS_SelectATSStationName(_in_  char * MaterialCode, _out_ char * ATSStationName);

//查询SN是否可以测试
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

//上传测试数据
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
	_in_ char *strlinename,//线别名称
	_in_ char *strbatch,//批次
	_in_ char *strlocation,//物料编码
	_in_ char *strstation,//站名
	_in_ char *strsn,//过站条码
	_in_ int  iresult,//测试结果
	_in_ char * errorinfo,//错误信息
	_in_ char * JIG_Number,//机架编号
	_in_ char *strswversion,//软件版本
	_in_ char *strremark, //备注
	_in_ char *strkeys,//列名
	_in_ char *strvalues//值
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

//只上传测试数据
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

//向结果集插入错误类型
DLLAPI int CALLMETHOD  ATS_InsertErrorData(int Test_ID, char * Test_errorinfo);

//向结果集插入机架码
DLLAPI int CALLMETHOD  ATS_InsertJIGNumber(int Test_ID, char * JIGNumber);

//检查是否为不良，并上传到不良品系统//不良接口更新，这个只能作废了
DLLAPI int CALLMETHOD  ATS_InsertFailCode(
	_in_ char* Product_Batch,//批次
	_in_ char* Product_BarCode,//过站条码
	_in_ char* Product_Line_naem,//线体名称
	_in_ char* Product_StationName,//站名
	_in_ int   Product_Repeat_Times=3 //重测次数，华为都是3
	);

//检查是否为不良，并上传到不良品系统
DLLAPI int CALLMETHOD  ATS_InsertFailCode_V2(
	_in_ char* Product_Batch,//批次
	_in_ char* Product_BarCode,//过站条码
	_in_ char* Product_Line_naem,//线体名称
	_in_ char* Product_StationName,//站名
	_in_ char* Product_MaterialCode,//组件编码/物料编码
	_in_ int   Product_Repeat_Times = 3 //重测次数，华为都是3
	);

//1向新的过站数据表中插入数据
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
	_in_ char *strbatch,//批次
	_in_ char *strsn,//过站条码
	_in_ char *strstation,//站名
	_in_ char *strlinename,//线别名称
	_in_ char *strlocation,//物料编码
	_in_ char *strswversion,//软件版本
	_in_ char *strremark,//备注
	_in_ int  iresult//测试结果
	);

DLLAPI int CALLMETHOD ATS_InsertDataRow_OFFLINE_V2(
	_in_ char *strlinename,//线别名称
	_in_ char *strbatch,//批次
	_in_ char *strlocation,//物料编码
	_in_ char *strstation,//站名
	_in_ char *strsn,//过站条码
	_in_ int  iresult,//测试结果
	_in_ char * errorinfo,//错误信息
	_in_ char * JIG_Number,//机架编号
	_in_ char *strswversion,//软件版本
	_in_ char *strremark, //备注
	_in_ char *strkeys,//列名
	_in_ char *strvalues//值
	);

/*
select mes_station_ctrl_test.inserttestresult_offline(p_tracenum       => '',--批次
p_ser_no         => '',--扫描码
p_seq_no         => '',--工站
p_prod_line_name => '',--生产线（新增）
p_item_code      => '',--物料编码（新增）
p_soft_ver       => '',--软件版本（新增）
p_remark         => '',--备注（新增）
P_result         => ''--结果
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
/* 过站管理                                                             */
/* 20160923                                                             */
/* Lory                                                               */
/************************************************************************/

/************************************************************************/
/* 上传数据                                                             */
/* 20170104                                                             */
/* Lory                                                               */
/************************************************************************/


//插入两个数据
DLLAPI int CALLMETHOD InsertTwoData(
	_in_ char* TableName,
	_in_ char* FieldName,
	_in_ char* Value,
	_in_ char* FieldName2,
	_in_ char* Value2
	);


//更新数据，两个条件
DLLAPI int CALLMETHOD UpdateDataTwoCondition(
	_in_ char* TableName,
	_in_ char* FieldName2Update,
	_in_ char* Value2Update,
	_in_ char* FieldNameOfCondition,
	_in_ char* ValueOfCondition,
	_in_ char* FieldNameOfCondition2,
	_in_ char* ValueOfCondition2
	);

//更新数据，两个条件
DLLAPI int CALLMETHOD UpdateOneDataTwoCondition(
	_in_ char* TableName,
	_in_ char* Update_ColName,
	_in_ char* Update_ColVaule,
	_in_ char* FieldNameOfCondition,
	_in_ char* ValueOfCondition,
	_in_ char* FieldNameOfCondition2,
	_in_ char* ValueOfCondition2
	);

//获取数据,结果排序，获取第一个结果
DLLAPI int CALLMETHOD GetDataOrderBy(
	_in_ char* TableName,
	_in_ char* FieldName,
	_out_ char* Value,
	_in_ int SizeOfValue,
	_in_ char* FieldNameOfCondition,
	_in_ char* ValueOfCondition,
	_in_ char* OrderByAscOrDesc//DESC 降序 ASC 升序
	);

//插入一行数据
DLLAPI int CALLMETHOD InsertOneRowData(
	_in_ char* INSERT_ColName,
	_in_ char* INSERT_ColVaule,
	_in_ char* TableName
	);

//更新指定列的数据
DLLAPI int CALLMETHOD UpdateOneRowData(
	_in_ char* Update_ColName,
	_in_ char* Update_ColVaule,
	_in_ char* TableName,
	_in_ char* FieldNameOfCondition,
	_in_ char* ValueOfCondition
	);

//查询指定列的数据
DLLAPI int CALLMETHOD SelectOneRowData(
	_in_ char* Select_ColName,
	_out_ char* Select_ColVaule,
	_in_ char* TableName,
	_in_ char* FieldNameOfCondition,
	_in_ char* ValueOfCondition
	);

//SLM-D MES wisillica 数据上传
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
/* 上传数据                                                             */
/* 20170104                                                               */
/* Lory                                                               */
/************************************************************************/

/************************************************************************/
/* MAC&KEY                                                             */
/* 2017411                                                               */
/* Lory                                                               */
/************************************************************************/

//通过MAC，批次，类型,获取SN
DLLAPI int CALLMETHOD Product_GetSNorMAC(
	_in_ char* WifiMAC,
	_in_ char* Product_Batch,
	_in_ char* Product_Type,
	_out_ char* SN_Value
	);

//获取批次的第一个或者最后一个MAC
DLLAPI int CALLMETHOD MES_GetMACorSN_OrderByData(
	_in_ char *StrBatch,
	_in_ char *StrType,
	_in_ char *OrderByAscOrDesc,
	_out_ char* SN_Value
	);

//通过SN，批次，类型,获取两个连续的MAC
DLLAPI int CALLMETHOD Product_GetTwoSNorMAC(
	_in_ char* WifiMAC,
	_in_ char* Product_Batch,
	_in_ char* Product_Type,
	_out_ char* SN_Value
	);

//将SN，批次，类型,MAC形成一条记录
//SN_Value_Two 存放SN列
//SN_Value_One 存放ONE列
//所有参数强制大写
//select mes_otherdep_call_pkg.SnMacBind_ZJ('HY1731034','ZJ_SN', 'R273050217234', '00:22:6C:87:E5:D0') from dual
DLLAPI int CALLMETHOD Product_SnMacBind(
	_in_ char* Product_Batch,
	_in_ char* Product_Type,
	_in_ char* SN_Value_One,
	_in_ char* SN_Value_Two
	);

//将SN，更新到MAC的值后，形成一条记录
//select mes_otherdep_call_pkg.f_BindSN_SN('batch','type', 'sn', 'bind_sn') from dual;
DLLAPI int CALLMETHOD Product_SnBD_BindSN(
	_in_ char* Product_Batch,
	_in_ char* Product_Type,
	_in_ char* SN_Value,
	_in_ char* Bind_SN
	);

//用SN，获取BindSN
//select mes_otherdep_call_pkg.getBindSnbySN('6C5AB5532276') from dual;
DLLAPI int CALLMETHOD Product_SnGet_BindSN(
	_in_ char* SN_Value,
	_out_ char* Bind_SN
	);

//用过度码，获取MAC，数据来源，投入位的关联扫描
DLLAPI int CALLMETHOD Product_OvercodeGet_MAC(
	_in_ char* Overcode_Value,
	_in_ char* Batch_Value,
	_out_ char* Scan_MAC
	);

//用BIND_SN，批次，MAC，规则，获取SN，
DLLAPI int CALLMETHOD Product_BindSNGet_NewSN(
	_in_ char* Customer_Value,
	_in_ char* Batch_Value,
	_in_ char* Type_Value,
	_in_ char* SNPFX_Value,
	_in_ int SN_Length,
	_in_ char* BIND_SN,
	_out_ char* New_SN
	);

// 用BindSN，获取SN
// 	select sn from mes_keymac_maintain  where BIND_SN='740447A29003F';
// 	DLLAPI int CALLMETHOD Product_BindSN_GetSN(
// 		_out_ char* SN_Value,
// 		_out_ char* Product_Batch,
// 		_in_ char* Bind_SN
// 		);

//用通过SN，批次，类型,获取KEY
//select MES_OtherDep_Call_pkg.f_GetKEYBySN('OP1751002MA00001TL','OP1751002',  'KEY')from dual
DLLAPI int CALLMETHOD Product_BindSN_GetKEY(
	_in_ char* PSN_Value,
	_in_ char* Product_Batch,
	_in_ char* Product_Type,
	_out_ char* KEY_Value,
	_in_ int KEY_Value_Size
	);

//用通过SN，批次，类型,获取二进制的KEY
//select MES_OtherDep_Call_pkg.f_GetBinKEYBySN('70C94EB00D21','STG181601','KEY') from dual
DLLAPI int CALLMETHOD Product_SN_GetBINKEY(
	_in_ char* PSN_Value,
	_in_ char* Product_Batch,
	_in_ char* Product_Type,
	_out_ char* KEY_Value,
	_in_ int KEY_Value_Size
	);

//用通过SN，批次，类型,获取二进制的KEY
//select MES_OtherDep_Call_pkg.f_GetBinKEYBySN('70C94EB00D21','STG181601','KEY') from dual
DLLAPI int CALLMETHOD Product_SN_GetBINKEY_SaveFile(
	_in_ char* PSN_Value,
	_in_ char* Product_Batch,
	_in_ char* Product_Type,
	_in_ char* Product_SavePath,
	_out_ char* KEY_Value,
	_in_ int KEY_Value_Size
	);

//用通过Bind SN，批次，类型,获取二进制的KEY
// select mes_otherdep_call_pkg.f_GetBinKEYByScancode('ABC','STG181601','KEY') from dual
DLLAPI int CALLMETHOD Product_BindSN_GetBINKEY(
	_in_ char* PSN_Value,
	_in_ char* Product_Batch,
	_in_ char* Product_Type,
	_out_ char* KEY_Value,
	_in_ int KEY_Value_Size
	);

//用通过SN，批次，类型,获取指定的KEY
//select MES_OtherDep_Call_pkg.f_GetALDDKEYBySN('0104IBW180101N000001', 'GTR180706','KEY') from dual
DLLAPI int CALLMETHOD Product_SN_GetKEY(
	_in_ char* PSN_Value,
	_in_ char* Product_Batch,
	_in_ char* Product_Type,
	_out_ char* KEY_Value,
	_in_ int KEY_Value_Size
	);

//获取KEY的数据哈希值
//select dbms_utility.get_hash_value(
//'2EFB1BDC30FF4707*304502210082780E7EE9A33C854C846C32FE244AEAFFE3D7C9DCC990993A98885AB84D102A0220537CA4973A2E54896DCC13FB0ABE547135F5CC1A630A8F1FB67B4E5141339F74',
//1,1000000000) as l_hash from dual
DLLAPI int CALLMETHOD Product_GetKEY_DBHashValue(
	_in_ char* KEY_Value,
	_out_ char* Hash_Value
	);

//华为定制接口 2020-08-14  获取打印接口
//select mes_script_pkg.getScriptContent(p_trace_num =>'HW2036069',p_script_type =>'EQUIPMENT')from dual;      
//第一个参数批次 第二个类型（‘EQUIPMENT’或‘PRINT’）
DLLAPI int CALLMETHOD Product_GetScriptContent(
	_in_ char* Product_Batch,
	_in_ char* Product_Type,
	_out_ char* KEY_Value,
	_in_ int KEY_Value_Size
	);

//2021-06-24  获取KEY
//select mes_otherdep_call_pkg.f_getBinbyMAC('GTR212501','KEY','305075F60DD4') from dual    
//第一个参数批次 第二个类型（‘EQUIPMENT’或‘PRINT’）
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
/* 阿里打卡机数据接口                                                   */
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
/* 阿里打卡机数据接口                                                   */
/* 20170411                                                             */
/* Lory                                                               */
/************************************************************************/

/************************************************************************/
/* 肖工数据库DLL移植过来的函数                                         */
/* 20170415                                                             */
/* Lory                                                               */
/************************************************************************/
//插入一个数据
DLLAPI int CALLMETHOD InsertData(
	_in_ char* TableName,
	_in_ char* FieldName,
	_in_ char* Value
	);

//更新数据
DLLAPI int CALLMETHOD UpdateData(
	_in_ char* TableName,
	_in_ char* FieldName2Update,
	_in_ char* Value2Update,
	_in_ char* FieldNameOfCondition,
	_in_ char* ValueOfCondition
	);

//获取数据
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

//SONY_BTSPK_ATS表检查SN重复
DLLAPI int CALLMETHOD BtSpkCheckSN(
	_in_ char* SN,
	_out_ char* YorN
	);

//HW_C1检查绑定结果
DLLAPI int CALLMETHOD p_CheckSeqResult(
	_in_ char* Batch,
	_in_ char* SN,
	_out_ char* YorN
	);

//查询SONY产品信息
DLLAPI int CALLMETHOD ATS_SelectSONYInfo(
	_in_  char * BatchNumber,
	_out_ char * SONYInfo
	);

/************************************************************************/
/* 肖工数据库DLL移植过来的函数                                           */
/* 20170415                                                             */
/* Lory                                                               */
/************************************************************************/

/************************************************************************/
/* 标准调用IT接口函数                                                  */
/* 20171113                                                             */
/* Lory                                                               */
/************************************************************************/
//DLLAPI int CALLMETHOD ATS_CALL_IT_SQL_API(
//	_in_  char * SQL_API,
//	_out_ char * Result_Str,
//	_in_ int length_max
//	);

/************************************************************************/
/* 标准调用IT接口函数                                                */
/* 20171113                                                             */
/* Lory                                                               */
/************************************************************************/