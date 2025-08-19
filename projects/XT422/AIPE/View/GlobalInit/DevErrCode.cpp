#include "DevErrCode.h"

static tCodeMsg gErrCodeMsg[]=
{
	//report error message without address
	{DEVERR_INACTIVE,                   "Inactive",      NULL},
	{E_CommandError,					"Command Error",						    NULL},
	{E_ReadFail,						"Read Fail",							    NULL},	
	{E_BlankFail,						"Blank Fail",							    NULL},
	{E_IllegalFail,						"Illegal Fail",							    NULL},
	{E_EraseFail,						"Erase Fail",							    NULL},
	{E_ProgramFail,						"Program Fail",							    NULL},
	{E_VerifyFail,						"Verify Fail",							    NULL},
	{E_HiVccVerifyFail,					"Hi-VF Fail",						   	 	NULL},
	{E_LoVccVerifyFail,					"Lo-VF Fail",						    	NULL},
	{E_SecureFail,						"Secure Fail",				    		    NULL},
	{E_ProtectFail,						"Protect Fail",							    NULL},
	{E_UnprotectFail,					"Unprotect Fail",						    NULL},
	{E_AutoIDfail,						"CheckID Fail",						        NULL},
	{E_Bad_Block_Over_Limit,			"Bad blocks was over limit",			    NULL},
	{E_CheckSumError,					"Checksum compare error",				    NULL},
	{E_BackwardDevInserted,				"Device in error position",				    NULL},
	{E_FPGA_Intial_Fail,				"HW Init Fail",		    					NULL},
	{E_DriverInitFail,					"DrvInit Fail",					    		NULL},	
	{E_DeviceInitFail,					"DevInit Fail",						    	NULL},

	//{E_NoDevInSocket,"No device in socket!",NULL},
	{E_OSFail,					"OS Fail",							    NULL},
	{E_NoDevInSocket,					"No Device",							    NULL},
	{E_Pin_Continue_Check_Fail,			"PCC Fail Pin",		    					NULL},		
	{E_SpecialBitUploadFail,			"Config UP Fail",				    		NULL},
	{E_ProtectionRegisterFail,			"PRT REG Fail",				    			NULL},
	{E_Algo_Table_Error,				"Algo ERR",							    	NULL},
	{E_Block_Table_Error,				"Block ERR",							    NULL},
	{E_PinMap_Table_Error,				"Pinmap ERR",						        NULL},
	{E_Spc_Table_Error,					"Parameter ERR",						    NULL},
	{E_DBPARSER_ERR,					"DatabaseParse ERR",		    			NULL},
	{E_ChipData_Error,					"ChipData ERR",						    	NULL},
	{E_SocketNum_Error,					"AdpConfig ERR",					    	NULL},			
	{E_BadFlag_ERR,						"BadFlag ERR",					    		NULL},

	{E_HiddenROMIllegalFail,			"HidROM IG Fail",							NULL},
	{E_HiddenROMEraseFail,				"HidROM ER Fail",					    	NULL},
	{E_HiddenROMProgramFail,			"HidROM PG Fail",				    		NULL},
	{E_HiddenROMVerifyFail,				"HidROM VF Fail",							NULL},
	{E_HiddenROMBlankFail,				"HidROM BK Fail",					    	NULL},
	{E_Logic_Error,						"Logic ERR",							    NULL},
	{E_ConfigSetFail,				    "Config ERR",								NULL},
	{E_DeviceProtected,                 "Device Protected",	                		NULL},
	{E_DeviceWasSecured,			    "Device Secured",			    			NULL},
	{E_FAKELOAD_Error,				    "FakeloadParse ERR", 	        			NULL},
	{E_FileFormatError,					"FileFormatERR", 	                		NULL},
	{E_PowerOn_Fail,					"PowerOn ERR",				    			NULL},
	{E_PowerOff_Fail, 					"PowerOff ERR",				    			NULL},
	{E_DrvSNInit_Fail,					"SNInit ERR", 								NULL},
	{E_DrvSNFree_Fail,					"SNFree ERR",								NULL},
	{E_SkbPinMap_SetFail,				"AdpPinMapSet ERR",							NULL},
	{E_MRegist_Fail,					"Regist master ERR", 						NULL},


	//for copy master
	{E_M_NoDevice,				        "No Master",				                NULL},
	{E_M_IDFail,				        "M-CheckID Fail",				            NULL},
	{E_M_InitialFail,				    "M-InitialFail",				            NULL},	
	{E_M_ReadFail,				        "M-ReadFail",				                NULL},
	{E_M_PCCFail,				        "M-PCCFail",				                	NULL},	
	{E_Adp_Mismatch,					"Adp Mismatch",						NULL},	
	{E_Adp_OEMErr,						"Adp productor match failed",			NULL},	
	{E_Adp_OverLimit,					"Adp count over limited",				NULL},	
	{E_Adp_AccessErr,					"Adp Access ERR",						NULL},	

	//for selfting system
	{E_VRCalibrationFail,				"VRCalibrationFail",						NULL},
	{E_PinDriverTestFail,				"PinDriverTest Fail",				    	NULL},		
	{E_VCCPCalibrationFail,				"VCCPCalibrationFail",				    	NULL},
	{E_VPP1CalibrationFail,				"VPP1CalibrationFail",				    	NULL},
	{E_VPP2CalibrationFail,				"VPP2CalibrationFail",				    	NULL},
	{E_VHHCalibrationFail,				"VHHCalibrationFail",				    	NULL},

	{E_VCCPFail_TPL0202,				"VCCPFail_TPL0202",				    		NULL}, //VCCP TPL0202级检测错误信息
	{E_VCCPFail,						"VCCPFail",				    				NULL}, //VCCP Pin driver级检测错误信息

	{E_TTLFail_TPL0202,					"VHHFail_TPL0202",				    		NULL}, //TTL(VHH) TPL0202级检测错误信息
	{E_TTLFail,							"TTLFail",				    				NULL}, //TTL Pin driver级检测错误信息

	{E_VPP1Fail_TPL0202,				"VPP1Fail_TPL0202",				    		NULL}, //VPP1 TPL0202级检测错误信息
	{E_VPP1Fail,						"VPP1Fail",				    			    NULL}, //VPP1 Pin driver级检测错误信息

	{E_VPP2Fail_TPL0202,				"VPP2Fail_TPL0202",				    		NULL}, //VPP2 TPL0202级检测错误信息
	{E_VPP2Fail,						"VPP2Fail",				    			    NULL}, //VPP2 Pin driver级检测错误信息

	{E_VIHFail,							"VIHFail",				    				NULL}, //VIH Pin driver级检测错误信息
	{E_ResisterFail,					"ResisterFail",				    			NULL}, //47k电阻错误信息
	{E_GNDFail,							"GNDFail",				    				NULL}, //GND检测错误信息
	{E_DDRFail,							"DDRFail",				    				NULL}, //DDR检测错误信息

	{E_3v3Fail,							"3v3 ERR",				    				NULL}, //3.3v 检测错误信息
	{E_5v0Fail,							"5v0 ERR",				    				NULL}, //5.0v 检测错误信息
	{E_5v0_Backup_Fail,					"Backup 5v0 ERR",				    		NULL}, //backup 5.0v 检测错误信息
	{E_7v0Fail,							"7v0 ERR",				    				NULL}, //7.0v 检测错误信息
	{E_LockFail,						"LockFail",									NULL},
	{E_PaswdFail,						"PaswdFail",								NULL},
	{E_PaswdWRFail,						"PaswdWRFail",								NULL},
	{E_PaswdVFFail,						"PaswdVFFail",								NULL},
	{E_PaswdBKFail,						"PaswdBKFail",								NULL},	
	{E_PaswdRDFail,						"PaswdRDFail",								NULL},		
	{E_ChipOverLap,						"Chip Overlap",								NULL},	
};

QString GetErrMsg(ushort ErrCode)
{
	QString ErrMsg;
	int Size=sizeof(gErrCodeMsg)/sizeof(tCodeMsg);
	ushort ErrCodeReal=ErrCode&0x3FFF;///去掉地址打印位
	for(int i=0;i<Size;++i){
		if(gErrCodeMsg[i].code==ErrCodeReal){
			ErrMsg = gErrCodeMsg[i].msg;
		}
	}
	if(ErrMsg.isEmpty() ){
		ErrMsg = "Undefined Error Message";
	}
	return ErrMsg;
}