#pragma once

#include "..\parser_global.h"


#define MAIN_NOTE				0x00
#define MAIN_ACTION				0x01
#define MAIN_BSRPATTERN			0x02
#define MAIN_SAMPLEMASK			0x03
#define MAIN_RLOCK				0x04
#define MAIN_M1BUFF				0x05
#define MAIN_M7BUFF				0x06
#define MAIN_IDCODEVALUE		0x07
#define MAIN_IDMASK				0x08
#define MAIN_COREPLAIN			0x09
#define MAIN_FROMPLAIN			0x0A
#define MAIN_NVMPLAIN			0x0B
#define MAIN_SECURITY			0x0C
#define MAIN_MAX				0x2F

#define SUB_SEC0			0x00
#define SUB_SEC1			0x01
#define SUB_SEC2			0x02
#define SUB_SEC3			0x03
#define SUB_SEC4			0x04
#define SUB_SEC5			0x05
#define SUB_SEC6			0x06
#define SUB_SEC7			0x07

typedef struct tParseItemData {
	unsigned int nBufferWriteStart;//记录起地址，按字节对齐
	unsigned int nItemCount;//记录bit数或条目数（比如多少条NOTEs ,多少条ACTIONs）
	unsigned int nRev[3];
	unsigned char  nRev2[3];
	unsigned char  nActive; //记录是否提取成功标记。1—成功，0-不成功
}ParseItemData;

typedef struct tHeadData {
	unsigned int nActionCnt;//Action字符串数量
	unsigned int nNoteCnt;//Note字符串数量
	unsigned int nChecksumCnt;//checksum字符串数量
	unsigned int nDataStreamCompressLen;//DataStream加密数据长度
	unsigned int nDataStreamUnCompressLen;//DataStream解密数据长度, 数组长度
	unsigned int nRlockUnCompressLen;//Rlock数组长度
	unsigned int nRlockBufferLen;//Rlock数据写Buffer长度
	unsigned int nBsrPatternUnCompressLen;//BsrPattern数数组长度
	unsigned int nBsrPatternBufferLen;//BsrPattern写Buffer长度
	unsigned int nSampleMaskUnCompressLen;//SampleMask数组长度
	unsigned int nSampleMaskBufferLen;//SampleMask写Buffer长度
	unsigned int nM1UnCompressLen;//M1Buffer数组长度
	unsigned int nM1BufferLen;//M1Buffer写Buffer长度
	unsigned int nM7UnCompressLen;//M1Buffer数组长度
	unsigned int nM7BufferLen;//M1Buffer写Buffer长度
	unsigned int nIDCodeUnCompressLen;//IDCode数组长度
	unsigned int nIDCodeBufferLen;//IDCode写Buffer长度
	unsigned int nIDMaskUnCompressLen;//IDMask数组长度
	unsigned int nIDMaskBufferLen;//IDMask写Buffer长度
	unsigned int nActUrowAlgoUnCompressLen;//ActUrowAlgo数组长度
	unsigned int nActUrowAlgoBufferLen;//ActUrowAlgo写Buffer长度
	unsigned int nActUrowDesignUnCompressLen;//ActUrowDesign写Buffer长度
	unsigned int nActUrowDesignBufferLen;//ActUrowDesign写Buffer长度
	unsigned int nActUrowProgUnCompressLen;//ActUrowProg写Buffer长度
	unsigned int nActUrowProgBufferLen;//ActUrowProg写Buffer长度
	unsigned int nActUrowProgramSwUnCompressLen;//ActUrowProgramSw写Buffer长度
	unsigned int nActUrowProgramSwBufferLen;//ActUrowProgramSw写Buffer长度
	unsigned int nUrowMaskUnCompressLen;//UrowMask写Buffer长度
	unsigned int nUrowMaskBufferLen;//UrowMask写Buffer长度
	unsigned int nStpFileBufferLen;//StpFile写Buffer长度
	unsigned int nMDeviceBufferLen;//MDevice写Buffer长度
	void ReInit() {
		nNoteCnt = 0x00;
		nActionCnt = 0x00;
		nDataStreamCompressLen = 0;
		nDataStreamUnCompressLen = 0;
		nRlockUnCompressLen = 0;
		nRlockBufferLen = 0;
		nBsrPatternUnCompressLen = 0;
		nBsrPatternBufferLen = 0;
		nSampleMaskUnCompressLen = 0;
		nSampleMaskBufferLen = 0;
		nM1UnCompressLen = 0;
		nM1BufferLen = 0;
		nM7UnCompressLen = 0;
		nM7BufferLen = 0;
		nIDCodeUnCompressLen = 0;
		nIDCodeBufferLen = 0;
		nIDMaskUnCompressLen = 0;
		nIDMaskBufferLen = 0;
		nActUrowAlgoUnCompressLen = 0;
		nActUrowAlgoBufferLen = 0;
		nActUrowDesignUnCompressLen = 0;
		nActUrowDesignBufferLen = 0;
		nActUrowProgUnCompressLen = 0;
		nActUrowProgBufferLen = 0;
		nActUrowProgramSwUnCompressLen = 0;
		nActUrowProgramSwBufferLen = 0;
		nUrowMaskUnCompressLen = 0;
		nUrowMaskBufferLen = 0;
		nStpFileBufferLen = 0;
		nMDeviceBufferLen = 0;
	}
	tHeadData() {
		ReInit();
	};
}HeadData;



# if defined(ACTELSTPPASER_LIB)
#  define ACTELSTPPASER_EXPORT Q_DECL_EXPORT
# else
#  define ACTELSTPPASER_EXPORT Q_DECL_IMPORT
# endif


class ACTELSTPPASER_EXPORT ActelSTPPaser : public AbstractParser
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID AbstractParser_IID)
    Q_INTERFACES(AbstractParser)
public:
    explicit ActelSTPPaser();
    char* getVersion();
    char* getFormatName();
    bool ConfirmFormat(QString& filename);
    int TransferFile(QString& srcfn, QIODevice* dst);
private:
	int ParseStationment(QString strFormat, QString& lastStationment, QString strOneRow, ADR writeAddr, bool bDoingParse, int* nArrayLen, int* writeBufferLen);
};
