#pragma once

#include <QtCore/qglobal.h>
#include <qobject>
#include <qstring>
#include <qfile>

#ifndef BUILD_STATIC
# if defined(ABSTRACTPARSER_LIB)
#  define ABSTRACTPARSER_EXPORT Q_DECL_EXPORT
# else
#  define ABSTRACTPARSER_EXPORT Q_DECL_IMPORT
# endif
#else
# define ABSTRACTPARSER_EXPORT
#endif

// 有Qt Plugin的Project/Solution在設定上的注意項目
// 1. 每一個\ParserPlugins\的子代project, .cpp/.h都可做為樣板, 複製src為新的Format修改類別名稱 調整程式內容
// 再為這新Format創新的Qt Library Project, 加Class命名後就加這兩 cpp/h檔(但空白) 把修改好的樣板 覆蓋過來
// 2. 改Project中的 Output Directory改為 $(SolutionDir)$(Platform)\$(Configuration)\parser\
// 3. C++ | Preprocessor 添加 ABSTRACTPARSER_LIB (另外注意 PARSER<F<FormatName>_LIB 已在創建過程添加)
// 4. Qt Meta-Object Compiler | Macro Definition 也要添加 ABSTRACTPARSER_LIB
// 不然 moc後的自動產生程式 會Compile通過 但在Link過程會LNK2019
// 5. Debugging的command位置貼上 $(SolutionDir)$(Platform)\$(Configuration)\QtDataFile.exe
// 這樣在Solution選擇x64 或 x86 修正好Config後 才能在不改Project的情況下 順利Compile及LINK
// 6. Directories中: Library, Include不必加$(QTDIR)\include, $(QTDIR)\lib
// Linker的Additional Lib 會自動添加 $(QTDIR)\lib
// 7. 不moc的問題會產生LNK2001, 這是因為new Class當下並沒標示Q_OBJECT, 而把.h歸類到C++ Compiler
// 點選有問題的.h 修改屬性把型態改成QT Meta-object compiler (moc), .cpp維持C++ Compiler
// 8. project的x86/x64雙設定, 到Solution的properties, Configuration Properties做出切換, 看哪一個需要添加
// 點選Project, 滑鼠右擊選Qt Project Settings, 看看 Qt Version: Qt5.15.2_x64 或是 Qt5.15.2 有沒有選對
// 9. 切換Solution 或畫面上的 x64/x86 看看 項目2~8 有沒有出現問題 設定出現缺漏

#define C_BufSize 4096 // size of buffer for detecting File format 

// for all file formats, do not care the checksum
// direct to output by the send function
struct LineData {
    uint64_t address;  // Record address, It may not the physical address.
    int tag;           // Tag of the incoming record
    int cnt;           // Byte count
    uint8_t data[256]; // data buffer
};

class ABSTRACTPARSER_EXPORT AbstractParser: public QObject
{
    Q_OBJECT
public:
    explicit AbstractParser();
    virtual ~AbstractParser() {}
    virtual char *getVersion() = 0;
    virtual char* getFormatName() = 0;
    virtual bool ConfirmFormat(QString &filename) = 0;
    virtual int TransferFile(QString& srcfn, QFile* dst) = 0;
    
    // shared among derived parser classes, for ConfirmFormat()
    uint8_t work[C_BufSize]; 
    int step; // working index
    int len; // len=fSize<C_BufSize?fSize:C_BufSize;
    int LineNum;
    int FuseQty; // JEDEC 'Q######'
    
    qint64 FilePos; // src
    qint64 FileSize; // src
    int percentProgress; // src
    bool TruncErr; // addressing outside the iodevice range
    bool feof;
    int ferror;
    bool userabort;

    // subrourines for ConfirmFormat() and TransferFile()
    bool isTextHex(uint8_t* str, int cnt);
    uint8_t aschex(const uint8_t ch1);
    uint8_t asc2hex(uint8_t* str);
    uint32_t asc2Dec(uint8_t* str);
    uint32_t asc2int(uint8_t* addr);
    uint32_t str2uint(uint8_t* str, const int len);
    uint64_t str2long(uint8_t* str, const int len);
    uint32_t head_sum(uint8_t* str, int sumstytle);
    uint32_t SumHexText(uint8_t* str, int len);
    int RecordLength(void);
    void removeFileNull(void);

    // subroutine for TransforFile();
    void UpdateProgress(void);
    bool SkipNullHead(QFile *src);
    bool SkipNullHeadToTag(QFile* src, uint8_t tag, bool chktag);
    bool myfgets(QFile* src);
    int removeBackNul(uint8_t *str);
    uint32_t get_rec(uint8_t* str, int cnt, qint64 loadaddr, int sumstytle, QFile *dst);

signals:
    void Progress(int value);
    void ErrorRpt(int code, QString msg);

public slots:
    void UserAbort(void);
};

//封装成插件需要在原本封装dll的基础上添加以下语句
QT_BEGIN_NAMESPACE
#define AbstractParser_IID "org.acroview.plugin.AbstractParser"
Q_DECLARE_INTERFACE(AbstractParser, AbstractParser_IID)
QT_END_NAMESPACE

// const character/value
#define C_NUL 0
#define C_STX 2
#define C_ETX 3
#define C_HT 9
#define C_TAB 9
#define C_CR 0xd
#define C_LF 0xa
#define C_Space 0x20
#define C_NumberSign 0x23 // # number sign 井号
#define C_DollarSign 0x24 // $ dollar sign 美元符
#define C_PercentSign 0x25 // % percent sign 百分号
#define C_Ampersand 0x26 // & ampersand 与和符
#define C_Apostrophe 0x27 // ‘ apostrophe 单引号，省略符号
#define C_Asterisk 0x2A // * asterisk 星号
#define C_PlusSign 0x2B // + plus sign
#define C_Comma 0x2C // , comma 逗号 
#define C_Hyphen 0x2D // - hyphen 连字符
#define C_MinusSign 0x2D // - minus sign
#define C_Period 0x2E // . period, full stop or dot 句号 点
#define C_Slash 0x2F // / slash, forward slash 斜线
#define C_Colon 0x3A // : colon 冒号
#define C_Semicolon 0x3B // ; semicolon 分号
#define C_ATSign 0x40 // @ at sign or commercial at 爱特或小老鼠
#define C_BackSlash 0x5C // \ backslash 反斜线
#define C_Caret 0x5E // ^ caret 脱字符
#define C_GraveAccent 0x60 // ` Grave Accent
#define C_AcuteAccent 0xB4 // it's with Europea keyboard

extern char* C_None; // "None";
extern char* C_Binary; // "Binary";
extern char* C_IntelHex; // "IntelHex";
extern char* C_MicrochipINHX; // "MicrochipINHX";
extern char* C_TektronixHex; // "TektronixHex";
extern char* C_MotorolaS; // "MotorolaS";
extern char* C_SigneticsHex; // "SigneticsHex";
extern char* C_ExtendedTekhex; // "ExtendedTekhex";
extern char* C_HP64000Absolute; // "HP64000Absolute";
extern char* C_Spectrum; // "Spectrum";
extern char* C_TISDSMAC; // "TISDSMAC";
extern char* C_TIText; // "TIText";
extern char* C_ASCIIHex; // "ASCIIHex";
extern char* C_ASCIIOct; // "ASCIIOct";
extern char* C_ASCIIBinary; // "ASCIIBinary";
extern char* C_Straight; // "Straight";
extern char* C_FormatedBinary; // "FormatedBinary";
extern char* C_HoltekOTPMTP; // "HoltekOTPMTP";
extern char* C_CypressIIC; // "CypressIIC";
extern char* C_ADIHex; // "ADIHex";
extern char* C_PEM; // "PEM";
extern char* C_PlainHex; // "PlainHex";
extern char* C_EXARCFG; // "EXARCFG";
extern char* C_ATSHA204XML; // "ATSHA204XML";
extern char* C_ATECCXML; // "ATECCXML";
extern char* C_STNVM; // "STNVM";
extern char* C_TICSV; // "TICSV";
extern char* C_TITPSTXT; // "TITPSTXT";
extern char* C_LatticeNVCM; // "LatticeNVCM";
extern char* C_EnpirionROM; // "EnpirionROM";
extern char* C_RichtekProg; // "RichtekProg";
extern char* C_RichtekRRF; // "RichtekRRF";
extern char* C_POF; // "POF";
extern char* C_POFTag17; // "POFTag17";
extern char* C_ADP105xHEX; // "ADP105xHEX";
extern char* C_IRSalemATE; // "IRSalemATE";
extern char* C_InfineonPSF; // "InfineonPSF";
extern char* C_InfineonSBSL; // "InfineonSBSL";
extern char* C_InfineonXSF; // "InfineonXSF";
extern char* C_InfineonXCF; // "InfineonXCF";
extern char* C_IRAcadiaMIC; // "IRAcadiaMIC";
extern char* C_XPBELF; // "XPBELF";
extern char* C_OpusPMF; // "OpusPMF";
extern char* C_INIGUID; // "INIGUID";
extern char* C_TIEPR; // "TIEPR";
extern char* C_MemoryJEDEC; // "MemoryJEDEC";
extern char* C_NXPiMXeFuse; // "NXPiMXeFuse";
extern char* C_NXPI2CText; // "NXPI2CText";
extern char* C_SiliconText; // "SiliconText";
extern char* C_TDKTXT; // "TDKTXT";
extern char* C_LatticeFEA; // "LatticeFEA";

extern unsigned char C_ArrowHead4nibble[6]; //  [6] = { 8, 0x1c, 0x2a, 0x49, 0x8, 0x0 };
extern unsigned char C_ArrowHead8nibble[6]; //  [6] = { 8, 0x1c, 0x3e, 0x6b, 0x8, 0x0 };
extern uint32_t ExttekTable[128]; // [128]

extern char* C_ASCIIBinaryBITS; // "PN10HL";
extern char* C_OCTDIGIT; // "01234567";
extern char* C_DECDIGIT; // "0123456789";
extern char* C_DECDIGITst; // " \t0123456789"; // Space Tab 0~9
extern char* C_HEXDIGITsx; // " X0123456789ABCDEF"; // Space X 0~9 A~F
extern char* C_HEXDIGIT; // "0123456789ABCDEF";
extern char* C_HEXDIGITe; // "ABCDEFabcdef";
extern char* C_HEXDIGIT0Aa; // "0123456789ABCDEFabcdef";
extern char* C_CRLF; // "\r\n";
extern char* C_TITPSTXT_SEPS; // " \t:,"; // Separators/Delimiter
extern char* C_JEDECBINARY; // "01 \t\n\r";
extern char* C_sColon; // ":"; // for isLatticeFEA

// Spare C_Err_xxxx 1~14 for QFileDevice::FileError
#define C_Err_FormatError 0x52
#define C_Err_UserAbort   0x53
#define C_Err_FileZero    0x54
#define C_Err_RecordSum   0x55 
// QString(C_ErrMsg_RecordSumError+" Line %1: ").arg(LineNum, 4, 10) + QString((char *)work) + QString(" (%1h)").arg(ss, 2, 16);
#define C_Err_FileOpen    QFileDevice::FileError::OpenError
#define C_Err_WriteError  QFileDevice::FileError::WriteError

extern QString C_ErrMsg_UserAbort;   // tr("User Abort!");
extern QString C_ErrMsg_FileZero;    // tr("file size is zero!");
extern QString C_ErrMsg_FormatError; // tr("File Format Error!");
extern QString C_ErrMsg_RecordSum;   // tr("Record Sum Error!");
extern QString C_ErrMsg_FileOpen;    // tr("Fail to Open File!");
extern QString C_ErrMsg_WriteError;  // tr("Buffer Write Error!");

