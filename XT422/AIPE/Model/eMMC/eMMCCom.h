#pragma once

#include <QObject>
typedef unsigned char       	INT8U;
typedef char                	INT8S;
typedef unsigned short      	INT16U;
typedef short               	INT16S;
typedef unsigned int        	INT32U;
typedef int                 	INT32S;
#define PACK_ALIGN

#define PART_IDX_USER		(0x0000)
#define PART_IDX_BOOT1		(0x0001)
#define PART_IDX_BOOT2		(0x0002)
#define PART_IDX_RPMB		(0x0003)
#define PART_IDX_GPP1		(0x0004)
#define PART_IDX_GPP2		(0x0005)
#define PART_IDX_GPP3		(0x0006)
#define PART_IDX_GPP4		(0x0007)
#define PART_IDX_EXTCSD		(0x0008)
#define PART_IDX_ENHANCED	(0x0009)

#define BYTE_WR_REL_SET (167)
#define BYTE_FW_CONFIG (169)
#define BYTE_BOOT_WP (173)
#define BYTE_PARTITION_CFG (179)
#define BYTE_BOOT_CONFIG_PROT (178)
#define BYTE_BOOT_BUS_CONDITIONS (177)
#define BYTE_RST_n_FUNCTION (162)
#define BYTE_BKOPS_EN  (163)
#define BYTE_PARTITION_SETTING_COMPLETED (155)
#define BYTE_PARTITIONS_ATTRIBUTE (156)
#define BYTE_SECURE_REMOVAL_TYPE  (16)

#define PARTOPT_COPYTYPE_ALL	(0)
#define PARTOPT_COPYTYPE_PART	(1)
typedef struct tagPartOpt {
	uint		CopyOpt : 1;		///全拷贝为 PARTOPT_COPYTYPE_ALL 部分拷贝为1
	uint64_t	StartAddr;			///部分拷贝的起始地址
	uint64_t	Size;				///部分拷贝的大小
}PARTOPT, * PPARTOPT;

//emmc 这些位设置已经被设置完成，正常流程为读取IC获取值，然后填入设置EXTCSD
typedef struct mmc_raw_extcsd{
	INT8U ext_security_err;	///EXT_SECURITY_ERR [505]  indicates to the host the extended security error 
	INT8U s_cmd_set;		///S_CMD_SET [504] defines the command sets supported by the Device .   
	INT8U hpi_fatures;		///HPI_FEATURES  [503]  if the HPI feature is supported and which implementation is used by the device. 
	INT8U bkops_support;		///BKOPS_SUPPORT  [502]  indicates if the background operations feature is supported by the device.  
	INT8U max_packed_reads;	///MAX_PACKED_READS [501] describes the maximum number of commands that can be packed inside a packed read command
	INT8U max_packed_writes;	///MAX_PACKED_WRITES[500]
	INT8U data_tag_support;	///DATA_TAG_SUPPORT [499]  ndicates if the Data Tag mechanism features are supported.  
	INT8U tag_unit_size;	 ///TAG_UNIT_SIZE [498]  is used by the host to calculate the size of a Tag Uni t in Bytes. 
	INT8U tag_res_size;	///TAG_RES_SIZE [ 497] 
	INT8U context_capabilities;	///CONTEXT_CAPABILITIES [496] describes the capabilities of context management.  
	INT8U large_unit_size_M1;		///LARGE_UNIT_SIZE_M1[495]  describes the size of the Large Unit, minus one . 
	INT8U ext_support;	///EXT_SUPPORT[494]
	INT32U cache_size;	///CACHE_SIZE [ 252: 249] indicates the existence and size
	INT8U generic_cmd6_time;	///GENERIC_CMD6_TIME [248] indicates the default maximum timeout for a SWITCH command (CMD6)
	INT8U power_off_long_time;	///POWER_OFF_LONG_TIME [247]  
	INT8U bkops_status;	///BKOPS_STATUS [246]   indicates the level of background operations urgency.   
	INT32U correctly_prg_sectores_num; ///CORRECTLY_PRG_SECTORS_NUM [245:242] indicates how many sectors were successfully programmed
	INT8U ini_timeout_pa;	///INI_TIMEOUT_PA  [241]   the maximum initialization timeout during the first power up 
	INT8U pwr_cl_ddr_52_360;///PWR_CL_DDR_52_360 [239] 
	INT8U pwr_cl_ddr_52_195;///PWR_CL_DDR_52_195 [238]
	INT8U pwr_cl_200_195;///PWR_CL_200_195 [237]
	INT8U pwr_cl_200_130;///PWR_CL_200_130 [236]
	INT8U min_perf_ddr_w_8_52;///MIN_PERF_DDR_W_8_52  [235]
	INT8U min_pref_ddr_r_8_52;///MIN_PERF_DDR_R_8_52 [234]
	INT8U trim_mult;			///TRIM_MULT [232]  calculate the TRIM and DISCARD function timeout. 
	INT8U sec_feature_support;	///SEC_FEATURE_SUPPORT [231]  allows the host to determine which secure data management features are supported 
	INT8U sec_erase_mult;	///SEC_ERASE_MULT [230]  calculate Secure_Erase function timeout. 
	INT8U sec_trim_mult;	///SEC_TRIM_MULT [229] calculate Secure_TRIM function timeout. 
	INT8U boot_info;		///BOOT_INFO [228]
	INT8U boot_size_mult;	///BOOT_SIZE_MULT [226] 
	INT8U acc_size;		///ACC_SIZE [225]  This register defines one or multiple of programmable boundary unit which is programmed at the same  time.
	INT8U hc_erase_grp_size;	///HC_ERASE_GRP_SIZE [224]  defines the erase -unit size for high -capacity memory. 
	INT8U erase_timeout_mult;///ERASE_TIMEOUT_MULT [223]  calculate erase timeout for high -capacity erase operations and defines the timeout  value for the erase operation of one erase group.  
	INT8U rel_wr_sec_c;	///REL_WR_SEC_C [222]   register shall be set to 1 and has no impact on the reliable write operation.  
	INT8U hc_wp_grp_size;	///HC_WP_GRP_SIZE [221]   the write protect group size for high -capacity memory.
	INT8U s_c_vcc;		///S_C_VCC[220] define the max VCC current consumption during the Sleep state (slp). 
	INT8U s_c_vccq;		///S_C_VCCQ[219]   
	INT8U s_a_timeout;	///S_A_TIMEOUT [217] defines the max timeout value for state transitions from Standby state (stby) to Sleep state (slp)
	INT32U sec_count;	///SEC_COUNT [215:212]  calculate the device density LSB is byte 212
	INT8U min_pref_w_8_52;///MIN_PERF_W_8_52 [210]
	INT8U min_pref_r_8_52;///MIN_PERF_R_8_52 [209]
	INT8U min_pref_w_8_26_4_52;///MIN_PERF_W_8_26_4_52  [208]
	INT8U min_pref_r_8_26_4_52;///MIN_PERF_R_8_26_4_52 [207]
	INT8U min_pref_w_4_26;///MIN_PERF_W_4_26  [206]
	INT8U min_pref_r_4_26;///MIN_PERF_R_4_26  [205]
	INT8U pwr_cl_26_360;///PWR_CL_26_360 [203]
	INT8U pwr_cl_52_360;///PWR_CL_52_360 [202]
	INT8U pwr_cl_26_195;///PWR_CL_26_195 [201]
	INT8U pwr_cl_52_195;///PWR_CL_52_195  [200]
	INT8U partition_switch_time;	///PARTITION_SWITCH_TIME  [199]
	INT8U out_of_interrupt_time;	///OUT_OF_INTERRUPT_TIME [198]
	INT8U driver_strength;///DRIVER_STRENGTH [197]
	INT8U device_type;	///DEVICE_TYPE [196]
	INT8U csd_structure;	///CSD_STRUCTURE [194]
	INT8U ext_csd_rev;	///EXT_CSD_REV  [192]
	INT8U cmd_set;		///CMD_SET [191]
	INT8U cmd_set_rev;	///CMD_SET_REV [189]
	INT8U power_class;	///POWER_CLASS [187]
	INT8U hs_timing;		///HS_TIMING [185]
	INT8U bus_width;		///BUS_WIDTH	[183]
	INT8U erased_mem_cont;	///ERASED_MEM_CONT [181]
	INT8U partition_config; ///PARTITION_CONFIG [179]
	INT8U boot_config_prot;///BOOT_CONFIG_PROT [178]
	INT8U boot_bus_conditions;//BOOT_BUS_CONDITIONS  [177]
	INT8U erase_group_def;///ERASE_GROUP_DEF [175]
	INT8U boot_wp_status;///BOOT_WP_STATUS [174]
	INT8U boot_wp;///BOOT_WP  [173]
	INT8U user_wp;///USER_WP [171]
	INT8U fw_config;///FW_CONFIG [169]
	INT8U rpmb_size_mult;///RPMB_SIZE_MULT  [168]
	INT8U wr_rel_set;///WR_REL_SET [167]
	INT8U wr_rel_param;///WR_REL_PARAM [166]
	INT8U sanitize_start;///SANITIZE_START [165]
	INT8U bkops_start;///BKOPS_START  [164]
	INT8U bkops_en;///BKOPS_EN [163]
	INT8U rst_n_function;///RST_n_FUNCTION  [162]
	INT8U hpi_mgmt;///HPI_MGMT  [161]
	INT8U partitioning_support;///PARTITIONING_SUPPORT [160]
	INT8U max_enh_sizz_mult[3];///MAX_ENH_SIZE_MULT  [159:157]
	INT8U partitions_attribute;///PARTITIONS_ATTRIBUTE  [156]
	INT8U partition_setting_completed;///PARTITION_SETTING_COMPLETED [155]
	INT8U gp_size_mult[12];///GP_SIZE_MULT [153:143]   [0][1][2]为GPP1，对应ExtCSD[143][144][145], 
												///    [3][4][5]为GPP2, 对应ExtCSD[146][147][148]
												///    [6][7][8]为GPP2, 对应ExtCSD[149][150][151]
												///    [9][10][11]为GPP2, 对应ExtCSD[152][153][154]
	INT8U enh_size_mult[3];///ENH_SIZE_MULT [142:140]     [0]为140,1为141,2为142
	INT8U enh_start_addr[4];///ENH_START_ADDR  [139:136]  [0]为136,1为137,2为138,3为139
	INT8U sec_bad_blk_mgmnt; ///SEC_BAD_BLK_MGMNT [134]
	INT8U tcase_support;///TCASE_SUPPORT  [132]
	INT8U periodic_wakeup;///PERIODIC_WAKEUP [131]
	INT8U program_cid_csd_ddr_support;///PROGRAM_CID_CSD_DDR_SUPPORT [130]
	INT8U vendor_specific_field[64];///VENDOR_SPECIFIC_FIELD [127:64]
	INT8U native_sector_size;///NATIVE_SECTOR_SIZE [63]
	INT8U use_native_sector;///USE_NATIVE_SECTOR [62]
	INT8U data_sector_size;///DATA_SECTOR_SIZE  [61]
	INT8U ini_timeout_emu;///INI_TIMEOUT_EMU  [60]
	INT8U class_6_ctrl;///CLASS_6_CTRL [59]
	INT8U dyncap_needed;///DYNCAP_NEEDED [58]
	INT8U execption_events_ctrl[2];///EXCEPTION_EVENTS_CTRL [57:56]
	INT8U execption_events_status[2];///EXCEPTION_EVENTS_ST [55:54]
	INT8U ext_partitions_attribute[2];///EXT_PARTITIONS_ATTRIBUTE [53:52] [0]为52,1为53
	INT8U context_conf[15];///CONTEXT_CONF [51:37]
	INT8U packed_command_status;///PACKED_COMMAND_STATUS [36]
	INT8U packed_failure_index;///PACKED_FAILURE_INDEX [35]
	INT8U power_off_notification;///POWER_OFF_NOTIFICATION [34]
	INT8U cache_ctrl;///CACHE_CTRL [33]
	INT8U flush_cache;	///FLUSH_CACHE [32]
}PACK_ALIGN  EXTCSD; 

typedef struct mmc_raw_csd {
	INT32U csd_structure : 2;	///CSD_STRUCTURE[127:126]  		describes  the version of the CSD structure.	
	INT32U spec_vers : 4;		///SPEC_VERS[125:122]			System specification version  
	INT32U tacc : 8;			///TAAC [119:112]				Data read access-time 1 
	INT32U nsac : 8;			///NSAC[111:104] 				defines the typical case for the clock dependent factor of the data access time
	INT32U tran_speed : 8;		///TRAN_SPEED [103:96] 			Maximum bus clock frequency definition 
	INT32U ccc : 12;			///CCC [95:84] 					Card command classes 		
	INT32U read_bl_len : 4;		///READ_BL_LEN [83:80]			indicate the supported  maximum read data block length
	INT32U read_bl_partial : 1;	///READ_BL_PARTIAL [79] 			whether partial block sizes can be used in block read commands
	INT32U write_blk_misalign : 1;	///WRITE_BLK_MISALIGN [78]
	INT32U read_blk_misalign : 1;	///READ_BLK_MISALIGN[77]
	INT32U dsr_imp : 1;		///DSR_IMP [76]  				defines if the configurable driver stage is integrated on the Device
	INT32U c_size : 12;			///C_SIZE[73:62]				is used to compute the device  capacity for  devices up to 2GB of density
	INT32U vdd_r_curr_min : 3;	///VDD_R_CURR_MIN [61:59]  		The maximum values for read and write currents at the minimal power supply VDD
	INT32U vdd_w_curr_min : 3;	///VDD_W_CURR_MIN  [55:53]
	INT32U vdd_r_curr_max : 3;	///VDD_R_CURR_MAX  [58:56] 	The maximum values for read and write currents at the maximal power supply VDD
	INT32U vdd_w_curr_max : 3;	///VDD_W_CURR_MAX [52:50]  
	INT32U c_size_mult : 3;		///C_SIZE_MULT [49:47]  used for coding a factor MULT for computing the total device size
	INT32U erase_grp_size : 5;	///ERASE_GRP_SIZE [46:42]  	used to calculate the size of the erasable unit of the  Device
	INT32U erase_grp_mult : 5;	///ERASE_GRP_MULT [41:37] 	used for calculating the size of the  erasable unit of the Device 
	INT32U wp_grp_size : 5;		///WP_GRP_SIZE [36:32]		the minimum size of a write protected region
	INT32U wp_grp_enable : 1;	///WP_GRP_ENABLE[31]			indicates whether write protection of regions is  possible
	INT32U default_ecc : 2;		///DEFAULT_ECC[30:29]		defines the ECC code which is recommended for use
	INT32U r2w_factor : 3;		///R2W_FACTOR [28:26]	defines the typical block program time as a multiple of the read access time.
	INT32U write_bl_len : 4;	///WRITE_BL_LEN[25:22]	 	Max. write data block length 
	INT32U write_bl_partial : 1;		///WRITE_BL_PARTIAL[21]		Partial blocks for write allowed
	INT32U content_prot_app : 1;	///CONTENT_PROT_APP[16]	indicates whether the content protection application is s upported
	INT32U file_format_grp : 1;	///FILE_FORMAT_GRP [15]		indicates the selected group of file formats
	INT32U copy : 1;				///COPY[14]				 The COPY bit is a one time programmable bit.
	INT32U perm_write_protect : 1;	///PERM_WRITE_PROTECT [ 13]   
	INT32U tmp_write_protect : 1;		///TMP_WRITE_PROTECT [12]
	INT32U file_format : 2;			///FILE_FORMAT [11:10] 	indicates the file format on the Device
	INT32U ecc : 2;					///ECC[9:8]	defines the ECC code that was used for storing data on the Device
	INT32U crc : 7;					///CRC [7:1]	carries the check sum for the CSD contents
}PACK_ALIGN RAWCSD;

typedef struct tagCopyCfg {///不要在里面包含任何类
	INT32U CopySel : 1;				///选择的是智能拷贝还是原始拷贝  0-智能拷贝 1-原始拷贝
	PARTOPT PartOpts[8];		///USER等八大分区拷贝设置
	EXTCSD RawExtCSD;			//扩展CSD寄存器的原始值
	INT8U ModifyBitMap[512];	//修改Map表
	INT8U AuthKey[32];
	INT8U MifAuthKey[32];       ///不可见的MIF配置key
	INT32U IntelCopyFlag; ///只能拷贝的操作Flag  Bit0为1表明执行全擦除，Bit1为1表明执行Verify，其他为保留位默认为0
}PACK_ALIGN COPYCFG;

typedef struct tagAreaInfo {///不要在里面包含任何类
	INT32S AccessMode;
	INT32U UserAreaSize;			///User AreaM 为单位
	INT32U EnhUserAreaStartAddr;	///增强的User Area起始地址512K为单位
	INT32U EnhUserAreaSize;			///增强的User Area大小512K为单位
	INT32U Boot1AreaSize;			///Boot1 大小128K为单位
	INT32U Boot2AreaSize;			///Boot2 大小128K为单位
	INT32U RPMBAreaSize;			///RPMB 大小128K为单位
	INT32U GPP1AreaSize;			///GPP1 大小512K为单位
	INT32U GPP2AreaSize;			///GPP2 大小512K为单位
	INT32U GPP3AreaSize;			///GPP3 大小512K为单位
	INT32U GPP4AreaSize;			///GPP4 大小512K为单位
	INT32U WPGroupSize;				///Write Protect Group Size,大小512K为单位
}PACK_ALIGN AREAINFO;

typedef struct tagAnalysisCfg {///不要在里面包含任何类
	INT32U HugeBlkNum;			///分析的超级块占用多少个block
	ushort Virgin;				///空白值是多少
	INT8U AuthKey[32];
	INT32U CardSel;		///当前选中的卡分析位置
	INT32U CardTuningSel;	///当前选中用于做Tuning的卡的位置
}PACK_ALIGN ANALYSISCFG;

typedef struct tagMCardMakeCfg {///不要在里面包含任何类
	INT8U AuthKey[32];
}PACK_ALIGN MMCARDMAKECFG;

typedef struct tagVerifyInfo {
	uchar PartIdx; ///分区索引
	uchar IsEn;  ///是否使能
	uchar Type; ///校验类型 默认为0表示字节累加和
	ulong Chksum; ///实际校验和
}PACK_ALIGN tVerifyInfo;

typedef struct {
	INT8U scr_structure : 4;
	INT8U sd_spec : 4;
	INT8U data_stat_after_erase : 1;
	INT8U sd_security : 3;
	INT8U sd_bus_widths : 4;
	INT16U sd_spec3 : 1;
	INT16U ex_security : 4;
	INT16U sd_spec4 : 1;
	INT16U sd_specx : 4;
	INT16U reserved1 : 2;
	INT16U cmd_support : 4;
	INT32U reserved2;
}PACK_ALIGN tSDRawSCRReg;

typedef struct tagPSA {
	bool IsPSAEn; ///check whether the Product-State-Awareness
	INT8U ProductStateAwarenessEn;  ///ExtCSD[17]
	INT32U PreLoadDataSize;	///ExtCSD[25-22]
	INT8U PSAValue; ///ExtCSD[133], do not care about it 
}tPSA;

typedef struct tagMMCOption {
	uint OptSel;		///功能选择 EMMCOPT_MCOPY 等
	uint OptPartition;	//分区选择
	COPYCFG CopyCfg;
	AREAINFO  AreaInfo;
	uint RawCID[4];
	uint RawCSD[4];
	ANALYSISCFG AnalysisCfg;
	MMCARDMAKECFG MCardMakeCfg;
	uchar DrvPara[256];
	tVerifyInfo VerifyInfo[8];
	tSDRawSCRReg SDRawSCRReg;
	tVerifyInfo ExtCSDSumInfo;
	tVerifyInfo TotalSumInfo; ///总校验值信息
	uchar RawExtCSD[512]; ///从芯片读取回来的原始的512字节内容
	QString strMIFFile; ///选择的Mif文件
	tPSA PSAInfo;
	uchar ExtCSDMake[512]; ///母片制作设置的512ExtCSD，是对母片制作配置的补充，有些信息之前的EXTCSD结构体没有办法扩展
}PACK_ALIGN MMCOPTION;

struct ExtArea
{
	uint8_t ExtCSD[512];
	uint8_t csd[16];
};

struct SSDPartition
{
	uint8_t AreaName[6];
	uint16_t Crc16;
	uint32_t ChipBlKPos;
	uint32_t BLKNum;
	uint64_t NextSSD;
	uint8_t padding[8];
	uint8_t BlkData[0];
};

struct PartitionSizeModify
{
	uint32_t UserSize;//MB
	uint32_t EnhancedUserSize;//MB
	uint32_t GPP1Size;	//MB
	uint32_t GPP2Size;	//MB
	uint32_t GPP3Size;	//GPP2Size
	uint32_t GPP4Size;	//MB
	uint32_t BootSize;	//KB
	uint32_t RPMBSize;	//KB

	PartitionSizeModify()
	{
		UserSize = 0; EnhancedUserSize = 0; GPP1Size = 0; GPP2Size = 0;
		GPP3Size = 0; GPP4Size = 0; BootSize = 0; RPMBSize = 0;
	}
};

typedef struct mmc_raw_uicfg_extcsd {
	INT8U boot_size_mult;	///BOOT_SIZE_MULT [226] 
	INT8U hc_erase_grp_size;	///HC_ERASE_GRP_SIZE [224]  defines the erase -unit size for high -capacity memory.
	INT8U hc_wp_grp_size;	///HC_WP_GRP_SIZE [221]   the write protect group size for high -capacity memory.
	INT32U sec_count;	///SEC_COUNT [215:212]  calculate the device density LSB is byte 212
	INT8U partition_config; ///PARTITION_CONFIG [179]
	INT8U boot_config_prot;///BOOT_CONFIG_PROT [178]
	INT8U boot_bus_conditions;//BOOT_BUS_CONDITIONS  [177]
	INT8U erase_group_def;///ERASE_GROUP_DEF [175]
	INT8U boot_wp;///BOOT_WP  [173]
	INT8U user_wp;///USER_WP [171]
	INT8U fw_config;///FW_CONFIG [169]
	INT8U rpmb_size_mult;///RPMB_SIZE_MULT  [168]
	INT8U wr_rel_set;///WR_REL_SET [167]
	INT8U bkops_en;///BKOPS_EN [163]
	INT8U rst_n_function;///RST_n_FUNCTION  [162]
	INT8U partitions_attribute;///PARTITIONS_ATTRIBUTE  [156]
	INT8U partition_setting_completed;///PARTITION_SETTING_COMPLETED [155]
	INT8U gp_size_mult[12];///GP_SIZE_MULT [153:143]   [0][1][2]为GPP1，对应ExtCSD[143][144][145], 
												///    [3][4][5]为GPP2, 对应ExtCSD[146][147][148]
												///    [6][7][8]为GPP2, 对应ExtCSD[149][150][151]
												///    [9][10][11]为GPP2, 对应ExtCSD[152][153][154]
	INT8U enh_size_mult[3];///ENH_SIZE_MULT [142:140]     [0]为140,1为141,2为142
	INT8U enh_start_addr[4];///ENH_START_ADDR  [139:136]  [0]为136,1为137,2为138,3为139
	INT8U sec_bad_blk_mgmnt; ///SEC_BAD_BLK_MGMNT [134]
	INT8U secure_removal_type; ///SEC_REMOVAL_TYPE [16]
	INT8U ext_partitions_attribute[2];///EXT_PARTITIONS_ATTRIBUTE [53:52] [0]为52,1为53

	mmc_raw_uicfg_extcsd(){
		partition_config = 0; boot_config_prot = 0; boot_bus_conditions = 0; erase_group_def = 0; boot_wp = 0;
		user_wp = 0; fw_config = 0; wr_rel_set = 0; bkops_en = 0; rst_n_function = 0; partitions_attribute = 0;
		partition_setting_completed = 0; sec_bad_blk_mgmnt = 0; secure_removal_type = 0; hc_erase_grp_size = 0;
		hc_wp_grp_size = 0; boot_size_mult = 0; rpmb_size_mult = 0;
		memset(enh_size_mult, 0, sizeof(enh_size_mult));
		memset(enh_start_addr, 0, sizeof(enh_start_addr));
		memset(ext_partitions_attribute, 0, sizeof(ext_partitions_attribute));
		memset(gp_size_mult, 0, sizeof(gp_size_mult));
	}
}PACK_ALIGN  UI_CFG_EXTCSD;

typedef struct eMMCOptionModify
{
	UI_CFG_EXTCSD modify_extcsd;
	PartitionSizeModify modify_part;
}PACK_ALIGN eMMCOPTION_Modify;

//struct eMMCTableHeader
//{
//	uint16_t HeaderCRC;
//	uint8_t MagicType[6];
//	uint32_t PartitionTableSize;
//	uint32_t PartitionTableCRC;
//	uint8_t Reserved[48];
//};
#pragma pack(push, 1)  
struct eMMCTableHeader
{
	uint16_t HeaderCRC;//头的CRC
	uint32_t Magic; //0x4150524F
	uint16_t Version; //版本,2
	uint16_t HeaderLen; //长度512
	uint8_t MagicType[6];//后面类型是json
	uint32_t PartitionTableSize;//分区表内容长度
	uint32_t PartitionTableCRC;//分区表内容CRC
	uint64_t SSDStartOffset;//分区ACIMG的起始偏移
	uint64_t SSDEndOffset;//分区ACIMG的结束偏移
	uint64_t SSDUseTime; //保存时间，最近一次使用时间，用于排序
	uint8_t IsComplete;
	uint8_t ProjectName[163];//Emmc档名称，用于显示和区分
	uint8_t Reserved[300];
};
#pragma pack(pop)  

struct tHuaweiACFile {
	uint32_t EntryIdx;		//索引
	uint64_t FileBlockPos;	//在文件中的起始位置
	uint32_t dwBlkStart;	//起始block，对于字节或是sector模式都一样，以512字节为一个block。存放chip的开始地址ChipBlockPos
	uint32_t dwBlkNum;		//多少个block
	uint16_t CRC16;			//为1表示CRC16
	std::string PartName;	//分区名称
};

Q_DECLARE_METATYPE(EXTCSD)
Q_DECLARE_METATYPE(eMMCOPTION_Modify)