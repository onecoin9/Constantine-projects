

// ***                  Error Code define		    ***
/// ***------------------------------------------------------------------------***
// ***    File Name:      ErrorCode. h                                     ***
// ***    Author:                                                      	    ***
// ***    Created:        2012/1/1                                    ***
 //******************************************************************************/
#define	WCODE_ID 	0x1000
#define	ECODE_ID 	0x2000
#define	SCODE_ID 	0x4000
#define PRINT_ADDR	0x8000	///
#define PRINT_BCD	0x0800	///


//*****************************************************************************************//
//**********************   report error message without address    ************************//
//*****************************************************************************************//
//code address: 0x10~0x1FF

#define	E_CommandError					(0x10|ECODE_ID)
#define	E_CommandNotReleased			(0x11|ECODE_ID)
#define	E_CmdStringTooLong				(0x12|ECODE_ID)
#define	E_RS232upstreamLSRerror			(0x13|ECODE_ID)
#define	E_RS232downstreamLSRerror		(0x14|ECODE_ID)
#define	E_RS232upstreamStall			(0x15|ECODE_ID)
#define	E_RS232downstreamStall			(0x16|ECODE_ID)
#define	E_FmwFlashUnrecognized			(0x17|ECODE_ID)
#define	E_FmwFlashEraseFail				(0x18|ECODE_ID)
#define	E_FmwFlashProgramFail			(0x19|ECODE_ID)
#define	E_SerialNoHaveProg				(0x1A|ECODE_ID)
#define	E_SerialNoProgFail				(0x1B|ECODE_ID)
#define	E_SerialNoVfyFail				(0x1C|ECODE_ID)
#define	E_SkbEEPROMAccessFail			(0x1D|ECODE_ID)
#define	E_SocketBoardNotInstall			(0x1E|ECODE_ID)
#define	E_ConfigureFPGAfail				(0x1F|ECODE_ID)
#define	E_EditCompareFail				(0x20|ECODE_ID)
#define	E_EditSearchFail				(0x21|ECODE_ID)
#define	E_FmwFlashVerifyFail			(0x22|ECODE_ID)
#define	E_SKB_not_Match	               	(0x23|ECODE_ID)

///For Self Test & DRAM Test...///
#define	E_DmmHiReadTestFail				(0x28|ECODE_ID)
#define	E_DmmLoReadTestFail				(0x29|ECODE_ID)
#define	E_DmmWriteTestFail				(0x2A|ECODE_ID)
#define	E_DmmPFOxTestFail				(0x2B|ECODE_ID)


#define	E_VCCPCalibrationFail			(0x30|ECODE_ID)
#define	E_VPP1CalibrationFail			(0x31|ECODE_ID)
#define	E_VPP2CalibrationFail			(0x32|ECODE_ID)
#define	E_VHHCalibrationFail			(0x33|ECODE_ID)

#define	E_NoneUse						(0x34|ECODE_ID)
#define	E_VccpMarginHTestFail			(0x35|ECODE_ID)
#define	E_VccpMarginLTestFail			(0x36|ECODE_ID)
#define	E_VppMarginHTestFail			(0x37|ECODE_ID)
#define	E_VppMarginLTestFail			(0x38|ECODE_ID)
#define	E_VpxMarginHTestFail			(0x39|ECODE_ID)
#define	E_VpxMarginLTestFail			(0x3A|ECODE_ID)
///#define  E_VpsMarginHTestFail			0x3B
///#define  E_VpsMarginLTestFail			0x3C
#define	E_VhMarginHTestFail				(0x3D|ECODE_ID)
#define	E_VhMarginLTestFail				(0x3E|ECODE_ID)
#define	E_VzMarginHTestFail				(0x3F|ECODE_ID)
#define	E_VzMarginLTestFail				(0x40|ECODE_ID)
#define	E_IccpSenseFail					(0x41|ECODE_ID)
#define	E_IppSenseFail					(0x42|ECODE_ID)		
#define	E_IpeSenseFail					(0x43|ECODE_ID)
#define	E_IpsSenseFail					(0x44|ECODE_ID)
#define	E_PinDriverTestFail				(0x45|ECODE_ID)
#define	E_GNDpinLoTestFail				(0x46|ECODE_ID)		///Mapping Pool RAM Read/Write Test Fail/
#define	E_SRAMtestFail					(0x47|ECODE_ID)
#define	E_SDRAMtestFail					(0x48|ECODE_ID)
#define	E_SDRAMdetectFail				(0x49|ECODE_ID)
#define	E_GNDpinHiTestFail				(0x4A|ECODE_ID)
#define	E_EPDCalibrationFail	        (0x4B|ECODE_ID)		///EPD Calibration Fail ///////
#define	E_EPDADCInitFail	        	(0x4C|ECODE_ID)		///ADC of EPD Initial Fail /////
#define	E_TF_Configure_Fail           	(0x4D|ECODE_ID)     ///TF Configure Fail /////////
#define	E_Internal_Verify2_Fail       	(0x4E|ECODE_ID)     ///Internal Verify 2 Fail /////
#define	E_VRCalibrationFail				(0x4F|ECODE_ID)
#define E_CustomFail					(0x50|ECODE_ID)

/** For Device Operating ------------------------------------------------------
    No error address report
    Have pass/fail counting***/

#define	E_EraseFail_NA					(0x53|ECODE_ID)
#define	E_SyncCheckFail                	(0x5F|ECODE_ID)           /// Device Synchronous Checking Fail



///For File Up/Download...///
#define	E_SystemInfoUploadFail			(0x60|ECODE_ID)
#define	E_SkbInfoUpload					(0x61|ECODE_ID)
#define	E_DataUploadFail				(0x62|ECODE_ID)
#define	E_SpecialBitUploadFail			(0x63|ECODE_ID)
#define	E_DevBlockUploadFail			(0x64|ECODE_ID)
#define	E_DevSpecUploadFail				(0x65|ECODE_ID)
#define	E_SKBsSktAmountUpload			(0x66|ECODE_ID)
#define	E_DataBroadcastFail				(0x67|ECODE_ID)
#define	E_DriverDownloadFail			(0x68|ECODE_ID)
#define	E_DataDownloadFail				(0x69|ECODE_ID)
#define	E_DevSpecDownloadFail			(0x6A|ECODE_ID)
#define	E_SpecialBitDownloadFail		(0x6B|ECODE_ID)
#define	E_FmwDownloadFail				(0x6C|ECODE_ID)
#define	E_DevBlkActFgUploadFail			(0x6D|ECODE_ID)		/// Device block action flag upload fail//////
#define	E_DevBlkActFgDownloadFail		(0x6E|ECODE_ID)		/// Device block action flag download fail//////

/*For Device Operating ------------------------------------------------------
    Have error address report
    Have pass/fail counting**/
#define	E_ReadFail						(0x70|ECODE_ID)
#define	E_BlankFail						(0x71|ECODE_ID)
#define	E_IllegalFail					(0x72|ECODE_ID)
#define	E_EraseFail						(0x73|ECODE_ID)
#define	E_ProgramFail					(0x74|ECODE_ID)
#define	E_VerifyFail					(0x75|ECODE_ID)
#define	E_HiVccVerifyFail				(0x76|ECODE_ID)
#define	E_LoVccVerifyFail				(0x77|ECODE_ID)
#define	E_SecureFail					(0x78|ECODE_ID)
#define	E_ProtectFail					(0x79|ECODE_ID)
#define	E_UnprotectFail					(0x7A|ECODE_ID)
#define	E_AutoIDfail					(0x7B|ECODE_ID)
#define	E_UESfail						(0x7C|ECODE_ID)
#define	E_HiddenROMIllegalFail			(0x7D|ECODE_ID)
#define	E_HiddenROMEraseFail			(0x7E|ECODE_ID)
#define	E_HiddenROMProgramFail			(0x7F|ECODE_ID)
#define	E_HiddenROMVerifyFail			(0x80|ECODE_ID)
#define	E_HiddenROMBlankFail			(0x81|ECODE_ID)
#define	E_ParityCheckFail             	(0x82|ECODE_ID)
#define	E_Bad_Block_Over_Limit			(0x83|ECODE_ID)
#define	E_OTPBlankCheckFail				(0x84|ECODE_ID)
#define	E_ProtectionRegisterChkFail		(0x85|ECODE_ID)
#define	E_ProtectionRegisterVFYFail		(0x86|ECODE_ID)
#define	E_ProtectionRegisterPGFail		(0x87|ECODE_ID)

/** For Memory/MPU System -----------------------------------------------------
    No error address report
    Have pass/fail counting**/
#define	E_IncorrectManufacture			(0x88|ECODE_ID)
#define	E_IncorrectDevice				(0x89|ECODE_ID)
#define	E_DeviceWasSecured				(0x8A|ECODE_ID)
#define	E_DeviceLocked					(0x8B|ECODE_ID)
#define	E_NoDevInSocket              	(0x8C|ECODE_ID)
#define	E_BackwardDevInserted			(0x8D|ECODE_ID)
#define	E_NoDevSelect					(0x8E|ECODE_ID)
#define	E_Pin_Continue_Check_Fail		(0x8F|ECODE_ID)

#define	E_DriverInitFail				(0x90|ECODE_ID)
#define	E_CheckSumError					(0x91|ECODE_ID)		///Checksum Error //////
#define	E_HasNoSpecialBitBlock			(0x92|ECODE_ID)
#define	E_ProtectionRegisterFail		(0x93|ECODE_ID)
#define	E_LockFail						(0x94|ECODE_ID)
#define	E_UnknowFormat					(0x95|ECODE_ID)
#define	E_NoData						(0x96|ECODE_ID)		/// for NAND Flash
#define	E_Map_Table_Invalid				(0x97|ECODE_ID)
#define	E_Size_not_enough				(0x98|ECODE_ID)
#define	E_Over_max_block_limit			(0x99|ECODE_ID)
#define	E_No_Map_Table					(0x9A|ECODE_ID)
#define	E_Have_new_bad_block			(0x9B|ECODE_ID)
#define	E_Bad_Block_was_over_Limit		(0x9C|ECODE_ID)
#define	E_Program_Fail_in_ECC			(0x9D|ECODE_ID)
#define	E_Removed_Check_Fail			(0x9E|ECODE_ID)		/// for Gang System remove check
#define	E_Leakage_Current_Fail_1		(0x9F|ECODE_ID)


///For PLDPOF) System...///
#define	E_FileFormatError				(0xA0|ECODE_ID)

//for Gang system
#define E_M_NoDevice					(0xA1|ECODE_ID)
#define E_M_IDFail						(0xA2|ECODE_ID)
#define E_M_InitialFail					(0xA3|ECODE_ID)
#define E_M_ReadFail			    	(0xA4|ECODE_ID)
#define E_M_PCCFail			    		(0xA5|ECODE_ID)
#define	E_Adp_Mismatch					(0xA6|ECODE_ID)

//for NAND
#define E_BadFlag_ERR					(0xA7|ECODE_ID)
#define E_PaswdFail						(0xA8|ECODE_ID)
#define E_PaswdRDFail					(0xA9|ECODE_ID)
#define E_PaswdWRFail					(0xAB|ECODE_ID)
#define E_PaswdVFFail					(0xAC|ECODE_ID)
#define E_PaswdBKFail					(0xAD|ECODE_ID)

#define E_Adp_OEMErr					(0xAE|ECODE_ID)
#define E_Adp_OverLimit					(0xAF|ECODE_ID)
#define E_Adp_AccessErr					(0xF0|ECODE_ID)
#define E_Adp_OverNotify            	(0xF1|ECODE_ID)
#define E_Adp_HardwareCompatible    	(0xF3|ECODE_ID)


#define E_SN_FETCH                  	(0xF2|ECODE_ID)
#define E_CMD_NOEXEC                	(0xFFF|ECODE_ID)
#if 0
///For Miscellaneous...///
#define	E_CountStackOverflow           	(0xF0|ECODE_ID)           ///  For debug purpose, to monitor CountStack
#define	E_LoopStackOverflow            	(0xF1|ECODE_ID)           ///  For debug purpose, to monitor CountStack

///For Matrix Use..///
#define	E_AlgoInfoAreaFail				(0xF2|ECODE_ID)			///  for Matrix use
#define	E_NormalVerifyFail				(0xF3|ECODE_ID)			///  for Matrix use
#define	E_OutofOrderVerifyFail			(0xF4|ECODE_ID)			///  for Matrix use
#define	E_SimpleSecureFail				(0xF5|ECODE_ID)			/// for Matrix use
#define	E_IDDQFail						(0xF6|ECODE_ID)			///  for Matrix use


#define	E_StopInsertChipToSocket       	(0xF7|ECODE_ID)           

#define	E_IO_Table_P_Define_Error		(0xF8|ECODE_ID)           
#define	E_Test_Condition_Not_Define		(0xF9|ECODE_ID)           
#define	E_Field_Not_Define				(0xFA|ECODE_ID)           
#define	E_Preload_Fail					(0xFB|ECODE_ID)           
#define	E_Test_Vector_Over_Limit		(0xFC|ECODE_ID)
#define	E_JEDEC_Pattern_Over_Limit		(0xFD|ECODE_ID)
#define	E_Test_Function_not_Supported	(0xFE|ECODE_ID)
#endif




//For chipdata/DB table parse error///////////////////////////////////////
#define	E_Algo_Table_Error				(0x100|ECODE_ID)           
#define	E_Block_Table_Error				(0x101|ECODE_ID)           
#define	E_PinMap_Table_Error			(0x102|ECODE_ID)           
#define	E_Spc_Table_Error				(0x103|ECODE_ID)  
#define E_DBPARSER_ERR				    (0x10b|ECODE_ID)
#define	E_ChipData_Error				(0x104|ECODE_ID)           
#define E_SocketNum_Error				(0x105|ECODE_ID)


//FPGA error
#define	E_FPGA_Intial_Fail				(0x106|ECODE_ID)           
#define E_Logic_Error					(0x107|ECODE_ID)
#define E_DeviceInitFail		    	(0x108|ECODE_ID)
#define	E_DeviceInitialFail           	E_DeviceInitFail
#define	E_ConfigSetFail					(0x109|ECODE_ID)
#define E_DeviceProtected           	(0x10a|ECODE_ID)

#define E_FAKELOAD_Error				(0x10c|ECODE_ID)
#define E_PowerOn_Fail					(0x10d|ECODE_ID)
#define E_PowerOff_Fail					(0x10e|ECODE_ID)
#define E_DrvSNInit_Fail				(0x10f|ECODE_ID)
#define E_DrvSNFree_Fail				(0x110|ECODE_ID)
#define	E_SkbPinMap_SetFail				(0x111|ECODE_ID)
#define E_PowerCtl_Fail					(0x112|ECODE_ID)
#define E_MRegist_Fail					(0x113|ECODE_ID)

//add by mike on 2014/10/24 for selftesting
#define E_VCCPFail_TPL0202				(0x114|ECODE_ID)
#define E_VCCPFail						(0x115|ECODE_ID)
#define E_TTLFail_TPL0202				(0x116|ECODE_ID)
#define E_TTLFail						(0x117|ECODE_ID)
#define E_VPP1Fail_TPL0202				(0x118|ECODE_ID)
#define	E_VPP1Fail						(0x119|ECODE_ID)
#define E_VPP2Fail_TPL0202				(0x11A|ECODE_ID)
#define E_VPP2Fail						(0x11B|ECODE_ID)
#define	E_VIHFail						(0x11C|ECODE_ID)
#define E_ResisterFail					(0x11D|ECODE_ID)
#define E_GNDFail						(0x11E|ECODE_ID)
#define E_DDRFail						(0x11F|ECODE_ID)

#define	E_3v3Fail						(0x120|ECODE_ID)
#define E_5v0Fail						(0x121|ECODE_ID)
#define E_5v0_Backup_Fail				(0x122|ECODE_ID)
#define E_7v0Fail						(0x123|ECODE_ID)

///OverLap Checking
#define E_ChipOverlap                   (0x124|ECODE_ID)  ///Socket had a chip remained

/////////////////////////////////////////////////////////////////////
#define E_UIDGet                        (0x130|ECODE_ID)


#define E_OpenTestFail                      (0x131|ECODE_ID)        //0x2131  　 开路测试失败
#define E_ShortTestFail                     (0x132|ECODE_ID)        //0x2132　　 短路测试失败
#define E_OSFail                            (0x133|ECODE_ID)        //0x2133    开短路测试失败

//*****************************************************************************************//
//**********************   report error message with address    ***************************//
//*****************************************************************************************//

#define	E_ReadFail_A						(E_ReadFail|ECODE_ID|PRINT_ADDR)
#define	E_BlankFail_A						(E_BlankFail|ECODE_ID|PRINT_ADDR)
#define	E_IllegalFail_A						(E_IllegalFail|ECODE_ID|PRINT_ADDR)
#define	E_EraseFail_A						(E_EraseFail|ECODE_ID|PRINT_ADDR)
#define	E_ProgramFail_A						(E_ProgramFail|ECODE_ID|PRINT_ADDR)
#define	E_VerifyFail_A						(E_VerifyFail|ECODE_ID|PRINT_ADDR)
#define	E_HiVccVerifyFail_A					(E_HiVccVerifyFail|ECODE_ID|PRINT_ADDR)
#define	E_LoVccVerifyFail_A					(E_LoVccVerifyFail|ECODE_ID|PRINT_ADDR)
#define	E_HiddenROMIllegalFail_A			(E_HiddenROMIllegalFail|ECODE_ID|PRINT_ADDR)
#define	E_HiddenROMEraseFail_A				(E_HiddenROMEraseFail|ECODE_ID|PRINT_ADDR)
#define	E_HiddenROMProgramFail_A			(E_HiddenROMProgramFail|ECODE_ID|PRINT_ADDR)
#define	E_HiddenROMVerifyFail_A				(E_HiddenROMVerifyFail|ECODE_ID|PRINT_ADDR)
#define	E_HiddenROMBlankFail_A				(E_HiddenROMBlankFail|ECODE_ID|PRINT_ADDR)
#define	E_OTPBlankCheckFail_A				(E_OTPBlankCheckFail|ECODE_ID|PRINT_ADDR)
#define	E_ProtectionRegisterChkFail_A		(E_ProtectionRegisterChkFail|ECODE_ID|PRINT_ADDR)
#define	E_ProtectionRegisterVFYFail_A		(E_ProtectionRegisterVFYFail|ECODE_ID|PRINT_ADDR)
#define	E_ProtectionRegisterPGFail_A		(E_ProtectionRegisterPGFail|ECODE_ID|PRINT_ADDR)
#define	E_Pin_Continue_Check_Fail_A_BCD	    (E_Pin_Continue_Check_Fail|ECODE_ID|PRINT_BCD|PRINT_ADDR)


#define	E_VCCPFail_A					(E_VCCPFail|ECODE_ID|PRINT_ADDR)
#define	E_TTLFail_A						(E_TTLFail|ECODE_ID|PRINT_ADDR)
#define	E_VPP1Fail_A					(E_VPP1Fail|ECODE_ID|PRINT_ADDR)
#define	E_VPP2Fail_A					(E_VPP2Fail|ECODE_ID|PRINT_ADDR)
#define	E_VIHFail_A						(E_VIHFail|ECODE_ID|PRINT_ADDR)
#define	E_ResisterFail_A				(E_ResisterFail|ECODE_ID|PRINT_ADDR)
#define	E_GNDFail_A	    				(E_GNDFail|ECODE_ID|PRINT_ADDR)

#define E_LockFail_A					(E_LockFail|ECODE_ID|PRINT_ADDR)
#define E_PaswdFail_A					(E_PaswdFail|ECODE_ID|PRINT_ADDR)
#define E_PaswdWRFail_A					(E_PaswdWRFail|ECODE_ID|PRINT_ADDR)
#define E_PaswdRDFail_A					(E_PaswdRDFail|ECODE_ID|PRINT_ADDR)
#define E_PaswdVFFail_A					(E_PaswdVFFail|ECODE_ID|PRINT_ADDR)
#define E_PaswdBKFail_A					(E_PaswdBKFail|ECODE_ID|PRINT_ADDR)
