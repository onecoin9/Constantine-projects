#include "DataTransform.h"
#include <QIODevice>
#include <QString>
#include <QFile>
#include <cstring>

// const string
const char* C_None = "None";
const char* C_Binary = "Binary";
const char* C_IntelHex = "IntelHex";
const char* C_MicrochipINHX = "MicrochipINHX";
const char* C_TektronixHex = "TektronixHex";
const char* C_MotorolaS = "MotorolaS";
const char* C_SigneticsHex = "SigneticsHex";
const char* C_ExtendedTekhex = "ExtendedTekhex";
const char* C_HP64000Absolute = "HP64000Absolute";
const char* C_Spectrum = "Spectrum";
const char* C_TISDSMAC = "TISDSMAC";
const char* C_TIText = "TIText";
const char* C_ASCIIHex = "ASCIIHex";
const char* C_ASCIIOct = "ASCIIOct";
const char* C_ASCIIBinary = "ASCIIBinary";
const char* C_Straight = "Straight";
const char* C_FormatedBinary = "FormatedBinary";
const char* C_HoltekOTPMTP = "HoltekOTPMTP";
const char* C_CypressIIC = "CypressIIC";
const char* C_ADIHex = "ADIHex";
const char* C_PEM = "PEM";
const char* C_PlainHex = "PlainHex";
const char* C_EXARCFG = "EXARCFG";
const char* C_ATSHA204XML = "ATSHA204XML";
const char* C_ATECCXML = "ATECCXML";
const char* C_STNVM = "STNVM";
const char* C_TICSV = "TICSV";
const char* C_TITPSTXT = "TITPSTXT";
const char* C_LatticeNVCM = "LatticeNVCM";
const char* C_EnpirionROM = "EnpirionROM";
const char* C_RichtekProg = "RichtekProg";
const char* C_RichtekRRF = "RichtekRRF";
const char* C_POF = "POF";
const char* C_POFTag17 = "POFTag17";
const char* C_ADP105xHEX = "ADP105xHEX";
const char* C_IRSalemATE = "IRSalemATE";
const char* C_InfineonPSF = "InfineonPSF";
//const char* C_InfineonSBSL = "InfineonSBSL";
const char* C_InfineonXSF = "InfineonXSF";
const char* C_InfineonXCF = "InfineonXCF";
const char* C_IRAcadiaMIC = "IRAcadiaMIC";
const char* C_XPBELF = "XPBELF";
const char* C_OpusPMF = "OpusPMF";
const char* C_INIGUID = "INIGUID";
const char* C_TIEPR = "TIEPR";
const char* C_MemoryJEDEC = "MemoryJEDEC";
const char* C_NXPiMXeFuse = "NXPiMXeFuse";
const char* C_NXPI2CText = "NXPI2CText";
const char* C_SiliconText = "SiliconText";
const char* C_TDKTXT = "TDKTXT";
const char* C_LatticeFEA = "LatticeFEA";

const unsigned char C_ArrowHead4nibble[6] = { 8, 0x1c, 0x2a, 0x49, 0x8, 0x0 };
const unsigned char C_ArrowHead8nibble[6] = { 8, 0x1c, 0x3e, 0x6b, 0x8, 0x0 };
const uint32_t ExttekTable[128] =  // array[$24..$7A]
{ 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00,
  00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00,
  00, 00, 00, 00, 36, 37, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00,  
   0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 00, 00, 00, 00, 00, 00, 
  00, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 
  25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 00, 00, 00, 00, 39, 
  00, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 
  55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 00, 00, 00, 00, 00
};

const char Base64EncodeChars[64] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
    'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
    'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
    'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
    'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
    'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
    'w', 'x', 'y', 'z', '0', '1', '2', '3',
    '4', '5', '6', '7', '8', '9', '+', '/' 
};

const int Base64DecodeChars[256] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63,
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1,
    -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1,
    -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 
};

const char* C_ASCIIBinaryBITS = "PN10HL";
const char* C_OCTDIGIT = "01234567";
const char* C_DECDIGIT = "0123456789";
const char* C_DECDIGITst = " \t0123456789"; // Space Tab 0~9
const char* C_HEXDIGITsx = " X0123456789ABCDEF"; // Space X 0~9 A~F
const char* C_HEXDIGIT = "0123456789ABCDEF";
const char* C_HEXDIGITe = "ABCDEFabcdef";
const char* C_HEXDIGIT0Aa = "0123456789ABCDEFabcdef";
const char* C_CRLF = "\r\n";
const char* C_TITPSTXT_SEPS = " \t:,"; // Separators/Delimiter
const char* C_JEDECBINARY = "01 \t\n\r";
const char* C_sColon = ":"; // for isLatticeFEA

// const character/value
#define C_STX 2
#define C_ETX 3
#define C_HT 9
#define C_Tab 9
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

// Check cnt number of char in part of outside string
static bool isTextHex(char* str, int cnt)
{
    for (int i = 0; i < cnt; i++)
        if (strchr(C_HEXDIGIT0Aa, str[i]) == NULL) // ['0'..'9', 'A'..'F', 'a'..'f']
            return false;
    return true;
}

// Input the ASCII (ch1) and return an integer value (int)
static uint8_t aschex(const unsigned char ch1)
{
    // make 'a'..'f' capital, but '0'..'9' into 0x10..0x19
    if (ch1 <= 0x39) // '0'..'9'
        return (uint8_t)(ch1 - 0x30);
    else // 0x41 'A'..'F' --> 0x61 'a'..'f', 61h-57h = 10
        return (uint8_t)((ch1 | 0x20) - 0x57);
}

// Input ASCII string "HH..." and  return an integer value HH;
static uint8_t asc2hex(char* str)
{
    uint8_t Result = aschex(*str++) << 4;
    return (Result | aschex(*str));
}

// Input ASCII string "HH..." and  return an integer value HH;
static uint32_t asc2Dec(char* str)
{
    uint32_t Result = aschex(*str++) * 10;
    return Result + aschex(*str); // 注意型別轉換, 是否有符號擴展帶來的錯誤
}

// Convert 4 hex ASCII string to unsigned int
static uint32_t asc2int(char* addr) // "0123..." == (int.hex)0123
{
    union { uint32_t w; uint8_t b[2]; } numw;
    numw.b[1] = asc2hex(addr);
    numw.b[0] = asc2hex(addr + 2);
    return numw.w;
}

// can deal with the 8 digits or 6 digits or error input
// str2int, str2long
static uint32_t str2uint(char* str, const int len)
{
    uint32_t Result = 0;
    for (int i = 0; i < len; i++)
        if (strchr(C_HEXDIGIT0Aa, *str) != NULL) // 0..9, A..F, a..f
        {
            Result <<= 4;
            Result |= (uint32_t)aschex(*str++);
        }
        else
            break; // zero end string, You cannot count on the len
    return Result;
}

static uint64_t str2long(char* str, const int len)
{
    uint64_t Result = 0;
    for (int i = 0; i < len; i++)
        if (strchr(C_HEXDIGIT0Aa, *str) != NULL) // 0..9, A..F, a..f
        {
            Result <<= 4;
            Result |= (uint32_t)aschex(*str++);
        }
        else
            break; // zero end string, You cannot count on the len
    return Result;
}

// must check with the function isTextHex
// must mask with value $ff
static uint32_t SumHexText(char* str, int len)
{
    uint32_t Result = 0;
    for (int i = 0; i < len; i++)
    {
        Result += asc2hex(str);
        str += 2;
    }
    return Result & 0xff; // Result:=Result mod $100
};

// For the Signetic Absolute Object Format
static uint32_t XorRotateSum(char* str, int len)
{
    uint32_t Result = 0;
    for (int i = 0; i < len; i++)
    {
        Result ^= (uint32_t)asc2hex(str);
        // ROTATE left one bit after xor
        Result <<= 1;
        if (Result & 0x100)
            Result |= 1;
        str += 2;
    }
    return Result & 0xFF; // Result:=Result mod $100
}

// Character   Value (Decimal)
// '0'..'9'    0..9
// 'A'..'Z'    10..35
// '$'         36
// '%'         37
// '_'         39    (underscroe)
// 'a'..'z'    40..65
static uint32_t SumExtTek(char* str, int len)
{
    // '%lltcc.... sum all but exclude the cc
    uint32_t Result = 0;
    for (int i = 0; i < len; i++) // *str in ['0'..'9', 'A'..'Z', '$', '%', '_', 'a'..'z']
        if ((*str == '_') || (*str == '%') || (*str == '$') ||
            ((*str >= '0') && (*str <= '9')) ||
            ((*str >= 'A') && (*str <= 'Z')) ||
            ((*str >= 'a') && (*str <= 'z')))
        {
            if ((i != 3) && (i != 4)) // skip the Block Checksum
                Result += ExttekTable[*str];
            str++;
        }
        else
            return 0xFFFFFFFF; // If the character set is more than the table.
    return Result & 0xff; // Result:=Result mod $100
}

// For the Tekhex
// Note, the len is the number of digits
static uint32_t NibbleSum(char* str, int len)
{
    uint32_t Result = 0;
    for (int i = 0; i < len; i++)
        Result += (uint32_t)aschex(*str++);
    return Result & 0xff; // Result:=Result mod $100
}

// https://mayaposch.wordpress.com/2011/11/01/how-to-really-truly-use-qthreads-the-full-explanation/ 中介紹
// 使用QThread衍生子類, 實現自己的run來執行是錯誤的做法, 該在QObject衍生自己的類與程式, 用moveToThread推送給QThread的Instance來運算
// 永遠不應該在QObject 類別的建構函式中指派堆物件（使用new）

// ctor() parent需要指到工作的目標, 例如, Main Heap, DB虛擬空間, Sparse Matrix (Special Bit中的DRAM空間)
// Main Heap對應Project工作空間, 可以完成實際燒錄取資料
// DB虛擬空間用來對應建立Project過程, 選完資料後, 在不影響Main Heap的情況下, 可以擷取資料, 給下一站計算Checksum
// Sparse Matrix則是選完IC後,選完資料後, 過濾資料中的數值, 當作客戶進入Special Bit Editor前工作空間的預設值. 但是Special還分SRAM/DRAM兩組工作空間
DataTransform::DataTransform(QIODevice* ioDevice, QObject* parent) : QObject(parent)
{
    SrcFilename = ""; // it's for error message filename
    Src = NULL;
    SrcFormat = ENUMFORMAT::None;
    SrcLine = 0; // it's for error message line number
    RecordSumError = false; // for MotorolaS, MicrochipINHX, SigneticsHex, IntelHex, ExtendedTekhex, TektronixHex
    _base = ioDevice; // ioDevice: IO Target is DDR Memory Buffer, SSD of each Modules. USB or Ethernet are interface
    if (ioDevice != NULL)
        _size = _base->size();
    else
        _size = 0;
    _bias = 0;
}

void DataTransform::AssignBias(qint64 offset)
{
    _bias = offset;
}

// 來源檔案名稱, 開檔後呼叫DetectFormat(const QByteArray& fa)
// 因為想讓來源多樣, QFile開啟後能呼叫, 如果用SQL相關元件開啟Blob file後也要能呼叫, 他們的最大公約數是QByteArray嗎?
ENUMFORMAT DataTransform::DetectFormat(const QString& filename)
{
    QFile fptr(filename);

    if (fptr.open(QFile::ReadOnly | QFile::ExistingOnly))
    {   
        qint64 rSize = fptr.size();
        if (rSize > C_BufSize)
            rSize = C_BufSize;
        QByteArray ba = fptr.read(rSize); // 如果是Sparse Matrix
        ENUMFORMAT f = DetectFormat(ba); 
        fptr.close();
        return f;
    }
    else
        return ENUMFORMAT::None; // Fail to detect file format. have to extend this format 
}

// 最好資料來源是多樣的, Delphi中傳來的是開啟好的File Stream, FTP/HTTP Stream, 安全機制中轉的Memory Stream(不存中間檔), Database中的Blob Stream, 
// 共同的類別就是Stream, Qt的共同基底物件QByteArray? QIODevice?
// ESP要做過前置處理, 把T03/T04等結構取值, 移動檔案指標到T04之後, 其他來源, File Offset應是由0開始.  
// 諮詢橙子: QIODevice/QByteArray/QDataStream/QTextStream 都可達成, 後續要由應用標的來決定, 相關stream的緩存則使用QBuffer
ENUMFORMAT DataTransform::DetectFormat(const QByteArray& fa)
{
    ENUMFORMAT Result = ENUMFORMAT::Binary;

    isMotorolaSButSumError = 0;
    isMicrochipINHXButSumError = 0;
    isSigneticsHexButSumError = 0;
    isIntelHexButSumError = 0;
    isExtendedTekhexButSumError = 0;
    isTektronixHexButSumError = 0;

    len = fa.size();
    if (len > C_BufSize)
        len = C_BufSize;
    memcpy(work, fa.constData(), len);
    if (isHP64000Absolute()) return ENUMFORMAT::HP64000Absolute;
    if (isFormatedBinary()) return ENUMFORMAT::FormatedBinary;
    if (isHoltekOTPMTP()) return ENUMFORMAT::HoltekOTPMTP;
    if (isInfineonPSF()) return ENUMFORMAT::InfineonPSF;
    if (isATSHA204XML()) return ENUMFORMAT::ATSHA204XML;
    if (isATECCXML()) return ENUMFORMAT::ATECCXML;
    if (isTICSV()) return ENUMFORMAT::TICSV;
    if (isLatticeNVCM()) return ENUMFORMAT::LatticeNVCM;
    if (isSTNVM()) return ENUMFORMAT::STNVM;
    if (isEnpirionROM()) return ENUMFORMAT::EnpirionROM;
    if (isRichtekProg()) return ENUMFORMAT::RichtekProg;
    if (isRichtekRRF()) return ENUMFORMAT::RichtekRRF;
    if (isPOFTag17()) return ENUMFORMAT::POFTag17;
    if (isInfineonXSF()) return ENUMFORMAT::InfineonXSF;
    if (isInfineonXCF()) return ENUMFORMAT::InfineonXCF;
    if (isNXPiMXeFuse()) return ENUMFORMAT::NXPiMXeFuse;
    if (isNXPI2CText()) return ENUMFORMAT::NXPI2CText;
    if (isSiliconText()) return ENUMFORMAT::SiliconText;
    if (isTITPSTXT()) return ENUMFORMAT::TITPSTXT;
    if (isTDKTXT()) return ENUMFORMAT::TDKTXT;
    if (isTIOSCReg()) return ENUMFORMAT::TIOSCReg;
    if (isJEDEC()) return ENUMFORMAT::JEDEC;
    if (isPEM()) return ENUMFORMAT::PEM;
    if (isPlainHex()) return ENUMFORMAT::PlainHex;
    if (isLatticeFEA()) return ENUMFORMAT::LatticeFEA;

    // 2, Text type file format, Don't care the JEDEC/POF file in this version.
    // case of ':', 'S', '%'
    // Make a hash table for specific character, and record the first three char
    removeFileNull(); // assign step 0
    TrimSpace();
    // result=binary means the format is still unknown
    while ((step < len) && (Result == ENUMFORMAT::Binary))  // ÁÙ¨S§ä¨ìFormat
    switch (work[step]) 
    {
        case 'S': // MotorolaS
            if (isMotorolaSRecord((char*)&work[step], RecordLength())) // False is pass
                Result = ENUMFORMAT::MotorolaS;
            else
                SkipThisLine();
            break;
        case ':':  // IntelHex | SigneticsHex | MicrochipINHX
            if (isMicrochipINHXRecord((char*)&work[step], RecordLength())) // False is pass
                Result = ENUMFORMAT::MicrochipINHX;
            else // 1
                if (isSigneticsHexRecord((char*)&work[step], RecordLength())) // false is pass
                    Result = ENUMFORMAT::SigneticsHex;
                else
                    if (isAnalogDeviceHexRecord((char*)&work[step], RecordLength()))
                        Result = ENUMFORMAT::ADIHex;
                    else // Analog Device Hex
                        if (isIntelHexRecord((char*)&work[step], RecordLength())) // false is pass
                            Result = ENUMFORMAT::IntelHex;
                        else 
                            SkipThisLine();
            break;
            
        case '%': // Extended Tektronix
            if (isExtendedTekhexRecord((char *)&work[step], RecordLength())) // False is pass
                Result = ENUMFORMAT::ExtendedTekhex;
            else 
                SkipThisLine();
            break;
        case '/': // TektronixHex 
            if (isTektronixHexRecord((char*)&work[step], RecordLength())) // False is pass
                Result = ENUMFORMAT::TektronixHex;
            else 
                SkipThisLine();
        case '$': // it may only applicable in the first line
            // $Ahhhh,   for ASCIIHex
            // $Shhhh,   optional checksum field
            // $Aoooooo, for ASCIIOct
            // $oooooo,  optional checksum field
            if (isASCIIHex())
                Result = ENUMFORMAT::ASCIIHex;
            else
                if (isASCIIOct())
                    Result = ENUMFORMAT::ASCIIOct;
                else 
                    SkipThisLine();
            break;
        case 'K':  // 0, (K), 9, (inside a record 7, 8, B, F, *) TISDSMAC
            if (isTISDSMAC((char*)&work[step], RecordLength()))
                Result = ENUMFORMAT::TISDSMAC;
            else
                SkipThisLine();
            break;
        case 'w':  // Cypress IIC
            if (isCypressIIC())
                Result = ENUMFORMAT::CypressIIC;
            else
                SkipThisLine();
            break;
        case '@': // TI Text
            if (isTIText())
                Result = ENUMFORMAT::TIText;
            else 
                SkipThisLine();
            break;
            // EXAR CFG
        case '\'':
            if (isEXARCFG())
                Result = ENUMFORMAT::EXARCFG;
            else
                SkipThisLine();
            break;
        default:
         // check strong typed file format first
          // 0, (K), 9, (inside a record 7, 8, B, F, *) TISDSMAC
            if (isTISDSMAC((char*)&work[step], RecordLength()))
                Result = ENUMFORMAT::TISDSMAC;
            else
                // BPPPPNNNNF B11110000F BHHHHLLLLF, ASCIIBinary
                if ((work[step] == 'B') && isASCIIBinary())
                    Result = ENUMFORMAT::ASCIIBinary;
                else
                    // 3001 FFFF 642B 016D 1028 1076 8086 B284
                    if (isStraight())
                        Result = ENUMFORMAT::Straight;
                    else
                        // dddd 11110000 (4 digit decimal + 8 digit binary) Spectrum
                        // 0923 1010 (4 digit decimal + 4 digit binary) Spectrum
                        if (isSpectrum())
                            Result = ENUMFORMAT::Spectrum;
                        else
                            if (isPMICHex()) return ENUMFORMAT::IntelHex; // 移到所有判斷後, 這Format有Comment有MAIN_OTP:, FS_OTP: 額外的字
                            else
                                step = len; // Result:=C_Binary
                                  // if U make sure that the format is binary you have to break this loop
    }
    return Result;
}

// 這個程式用來做Project存檔, File Format Name, 這樣改動enum數值順序, 也不會索引錯的內容
const char* DataTransform::EnumToName(ENUMFORMAT FormatEnum)
{
    switch (FormatEnum)
    {
        case ENUMFORMAT::None: return C_None;
        case ENUMFORMAT::Binary: return C_Binary;
        case ENUMFORMAT::IntelHex: return C_IntelHex;
        case ENUMFORMAT::MicrochipINHX: return C_MicrochipINHX;
        case ENUMFORMAT::TektronixHex: return C_TektronixHex;
        case ENUMFORMAT::MotorolaS: return C_MotorolaS;
        case ENUMFORMAT::SigneticsHex: return C_SigneticsHex;
        case ENUMFORMAT::ExtendedTekhex: return C_ExtendedTekhex;
        case ENUMFORMAT::HP64000Absolute: return C_HP64000Absolute;
        case ENUMFORMAT::Spectrum: return C_Spectrum;
        case ENUMFORMAT::TISDSMAC: return C_TISDSMAC;
        case ENUMFORMAT::TIText: return C_TIText;
        case ENUMFORMAT::ASCIIHex: return C_ASCIIHex;
        case ENUMFORMAT::ASCIIOct: return C_ASCIIOct;
        case ENUMFORMAT::ASCIIBinary: return C_ASCIIBinary;
        case ENUMFORMAT::Straight: return C_Straight;
        case ENUMFORMAT::FormatedBinary: return C_FormatedBinary;
        case ENUMFORMAT::HoltekOTPMTP: return C_HoltekOTPMTP;
        case ENUMFORMAT::CypressIIC: return C_CypressIIC;
        case ENUMFORMAT::ADIHex: return C_ADIHex;
        case ENUMFORMAT::PEM: return C_PEM;
        case ENUMFORMAT::PlainHex: return C_PlainHex;
        case ENUMFORMAT::EXARCFG: return C_EXARCFG;
        case ENUMFORMAT::ATSHA204XML: return C_ATSHA204XML;
        case ENUMFORMAT::ATECCXML: return C_ATECCXML;
        case ENUMFORMAT::STNVM: return C_STNVM;
        case ENUMFORMAT::TICSV: return C_TICSV;
        case ENUMFORMAT::TITPSTXT: return C_TITPSTXT;
        case ENUMFORMAT::LatticeNVCM: return C_LatticeNVCM;
        case ENUMFORMAT::EnpirionROM: return C_EnpirionROM;
        case ENUMFORMAT::RichtekProg: return C_RichtekProg;
        case ENUMFORMAT::RichtekRRF: return C_RichtekRRF;
        case ENUMFORMAT::POF: return C_POF;
        case ENUMFORMAT::POFTag17: return C_POFTag17;
        case ENUMFORMAT::ADP105xHEX: return C_ADP105xHEX;
        case ENUMFORMAT::IRSalemATE: return C_IRSalemATE;
        case ENUMFORMAT::InfineonPSF: return C_InfineonPSF;
        case ENUMFORMAT::InfineonXSF: return C_InfineonXSF;
        case ENUMFORMAT::InfineonXCF: return C_InfineonXCF;
        case ENUMFORMAT::IRAcadiaMIC: return C_IRAcadiaMIC;
        case ENUMFORMAT::XPBELF: return C_XPBELF;
        case ENUMFORMAT::OpusPMF: return C_OpusPMF;
        case ENUMFORMAT::INIGUID: return C_INIGUID;
        case ENUMFORMAT::TIEPR: return C_TIEPR;
        case ENUMFORMAT::MemoryJEDEC: return C_MemoryJEDEC;
        case ENUMFORMAT::NXPiMXeFuse: return C_NXPiMXeFuse;
        case ENUMFORMAT::SiliconText: return C_SiliconText;
        case ENUMFORMAT::TDKTXT: return C_TDKTXT;
        case ENUMFORMAT::LatticeFEA: return C_LatticeFEA;
        default: return C_None;
    }
}

ENUMFORMAT DataTransform::NameToEnum(const char* FormatName)
{
    if (strcmp(FormatName, C_None) == 0) return ENUMFORMAT::None;
    if (strcmp(FormatName, C_Binary) == 0) return ENUMFORMAT::Binary;
    if (strcmp(FormatName, C_IntelHex) == 0) return ENUMFORMAT::IntelHex;
    if (strcmp(FormatName, C_MicrochipINHX) == 0) return ENUMFORMAT::MicrochipINHX;
    if (strcmp(FormatName, C_TektronixHex) == 0) return ENUMFORMAT::TektronixHex;
    if (strcmp(FormatName, C_MotorolaS) == 0) return ENUMFORMAT::MotorolaS;
    if (strcmp(FormatName, C_SigneticsHex) == 0) return ENUMFORMAT::SigneticsHex;
    if (strcmp(FormatName, C_ExtendedTekhex) == 0) return ENUMFORMAT::ExtendedTekhex;
    if (strcmp(FormatName, C_HP64000Absolute) == 0) return ENUMFORMAT::HP64000Absolute;
    if (strcmp(FormatName, C_Spectrum) == 0) return ENUMFORMAT::Spectrum;
    if (strcmp(FormatName, C_TISDSMAC) == 0) return ENUMFORMAT::TISDSMAC;
    if (strcmp(FormatName, C_TIText) == 0) return ENUMFORMAT::TIText;
    if (strcmp(FormatName, C_ASCIIHex) == 0) return ENUMFORMAT::ASCIIHex;
    if (strcmp(FormatName, C_ASCIIOct) == 0) return ENUMFORMAT::ASCIIOct;
    if (strcmp(FormatName, C_ASCIIBinary) == 0) return ENUMFORMAT::ASCIIBinary;
    if (strcmp(FormatName, C_Straight) == 0) return ENUMFORMAT::Straight;
    if (strcmp(FormatName, C_FormatedBinary) == 0) return ENUMFORMAT::FormatedBinary;
    if (strcmp(FormatName, C_HoltekOTPMTP) == 0) return ENUMFORMAT::HoltekOTPMTP;
    if (strcmp(FormatName, C_CypressIIC) == 0) return ENUMFORMAT::CypressIIC;
    if (strcmp(FormatName, C_ADIHex) == 0) return ENUMFORMAT::ADIHex;
    if (strcmp(FormatName, C_PEM) == 0) return ENUMFORMAT::PEM;
    if (strcmp(FormatName, C_PlainHex) == 0) return ENUMFORMAT::PlainHex;
    if (strcmp(FormatName, C_EXARCFG) == 0) return ENUMFORMAT::EXARCFG;
    if (strcmp(FormatName, C_ATSHA204XML) == 0) return ENUMFORMAT::ATSHA204XML;
    if (strcmp(FormatName, C_ATECCXML) == 0) return ENUMFORMAT::ATECCXML;
    if (strcmp(FormatName, C_STNVM) == 0) return ENUMFORMAT::STNVM;
    if (strcmp(FormatName, C_TICSV) == 0) return ENUMFORMAT::TICSV;
    if (strcmp(FormatName, C_TITPSTXT) == 0) return ENUMFORMAT::TITPSTXT;
    if (strcmp(FormatName, C_LatticeNVCM) == 0) return ENUMFORMAT::LatticeNVCM;
    if (strcmp(FormatName, C_EnpirionROM) == 0) return ENUMFORMAT::EnpirionROM;
    if (strcmp(FormatName, C_RichtekProg) == 0) return ENUMFORMAT::RichtekProg;
    if (strcmp(FormatName, C_RichtekRRF) == 0) return ENUMFORMAT::RichtekRRF;
    if (strcmp(FormatName, C_POF) == 0) return ENUMFORMAT::POF;
    if (strcmp(FormatName, C_POFTag17) == 0) return ENUMFORMAT::POFTag17;
    if (strcmp(FormatName, C_ADP105xHEX) == 0) return ENUMFORMAT::ADP105xHEX;
    if (strcmp(FormatName, C_IRSalemATE) == 0) return ENUMFORMAT::IRSalemATE;
    if (strcmp(FormatName, C_InfineonPSF) == 0) return ENUMFORMAT::InfineonPSF;
//  if (strcmp(FormatName, C_InfineonSBSL) == 0) return ENUMFORMAT::InfineonSBSL;
    if (strcmp(FormatName, C_InfineonXSF) == 0) return ENUMFORMAT::InfineonXSF;
    if (strcmp(FormatName, C_IRAcadiaMIC) == 0) return ENUMFORMAT::IRAcadiaMIC;
    if (strcmp(FormatName, C_XPBELF) == 0) return ENUMFORMAT::XPBELF;
    if (strcmp(FormatName, C_OpusPMF) == 0) return ENUMFORMAT::OpusPMF;
    if (strcmp(FormatName, C_INIGUID) == 0) return ENUMFORMAT::INIGUID;
    if (strcmp(FormatName, C_TIEPR) == 0) return ENUMFORMAT::TIEPR;
    if (strcmp(FormatName, C_MemoryJEDEC) == 0) return ENUMFORMAT::MemoryJEDEC;
    if (strcmp(FormatName, C_NXPiMXeFuse) == 0) return ENUMFORMAT::NXPiMXeFuse;
    if (strcmp(FormatName, C_SiliconText) == 0) return ENUMFORMAT::SiliconText;
    if (strcmp(FormatName, C_TDKTXT) == 0) return ENUMFORMAT::TDKTXT;
    if (strcmp(FormatName, C_LatticeFEA) == 0) return ENUMFORMAT::LatticeFEA;
    return ENUMFORMAT::None;
}

// 這是給序號燒錄ESP模式的程式使用, 執行ESP會產生serial.dat中間檔, 
// 這文檔內的T03 Translation format number, 屬於Data I/O的Format Code, 移動檔案指標到T04之後, 就可以呼叫轉換程式
ENUMFORMAT DataTransform::DataIOFomatCode(const int ESPT03)
{
	switch (ESPT03) // It's data i/o format code
 	{ 
    // 不同年代時空背景下 不同開發環境, 產生了多種的Format, Data I/O有文件說明差異, 像現代utf-8的BOM一樣, 30多年前也有奇怪的文字檔的leading charactars.
    // 這轉換程式把他們間差異合併考慮, 讓一個Format轉換程序能處理這些差異來Download
    // Upload就由其中選一種形式來處理
    case 1: // ASCII-BNPF
    case 5: // ASCII-BNPF
    case 2: // ASCII-BHLF
    case 6: // ASCII-BHLF 
    case 3: // ASCII-B10F 
    case 7: // ASCII-B10F
    case 8: // 5-level BNPF
    case 9: // 5-level BNPF
        return ENUMFORMAT::ASCIIBinary;
    case 4: // 7, Texas Instruments SDSMAC (320)
        return ENUMFORMAT::TISDSMAC;
    case 12: // Spectrum
    case 13: // Spectrum
        return ENUMFORMAT::Spectrum;
    case 30: // ASCII-Octal Space
    case 35: // ASCII-Octal Space 
    case 31: // ASCII-Octal Percent
    case 36: // ASCII-Octal Percent
    case 32: // ASCII-Octal Apostrophe 
    case 37: // ASCII-Octal SMS
        return ENUMFORMAT::ASCIIOct;
    case 50: // ASCII-Hex Space
    case 55: // ASCII-Hex Space 
    case 51: // ASCII - Hex Percent
    case 56: // ASCII-Hex Percent
    case 52: // ASCII-Hex Apostrophe
    case 57: // ASCII-Hex SMS
    case 53: // ASCII-Hex Comma
    case 58: // ASCII-Hex Comma
        return ENUMFORMAT::ASCIIHex;
    case 82: // Motorola Exorciser
    case 87: // Motorola Exormax
    case 95: // Motorola 32 bit (S3 record)
        return ENUMFORMAT::MotorolaS;
    case 83: // Intel Intellec 8/MDS
    case 88: // Intel MCS-86 Hex Object
    case 99: // Intel Hex-32
        return ENUMFORMAT::IntelHex; // 這個對應的轉換程式就要能一次處理三個時代的Intel Hex
    case 86: // Tektronix Hexadecimal
        return ENUMFORMAT::TektronixHex;
    case 90: // Texas Instruments SDSMAC
    case 94: // Tektronix Hexadecimal Extended 
        return ENUMFORMAT::TISDSMAC;
    default: 
        return ENUMFORMAT::None; // ESP傳來新的Format ID, 跳轉自動判別或是報錯
    }
}

// Skip white space charcter NUL(0), SOH(1), STX(2), ETX(3), CR, LF, SOM
// at the beginning of a file
// don't skip the $8 for the Formated Binary
void DataTransform::removeFileNull(void)
{
    step = 0; // goto the start of a file
    if ((work[0] == 0xEF) && (work[1] == 0xBB) && (work[2] == 0xBF)) // remove UTF-8 header
        step = 3;
    // skip the leading character which in (NUL, SOH, STX, SOM, LF, CR)
    while ((((work[step]>=0) && (work[step] <= 2)) || (work[step] == '\r') || (work[step] == '\n') || (work[step] == 0x12)) && (step < len))
        step++;
}

// A text file format can be initial by removeFileNull for removing file prefix
// It's a new string copy of the Work buffer
char * DataTransform::gets(void)
{
    bytes.clear(); // It's a new string copy of the Work buffer with QByteArray
    while ((work[step] == '\r') || (work[step] == '\n'))  step++;
    while ((work[step] != '\r') && (work[step] != '\n') && (step < len))
        bytes.append(work[step++]);
    return bytes.data(); // make a string copy with QByteArray, so we can use string function such as strtok()
}

// Skip white space (Tab, LF, CR, Space)
void DataTransform::TrimSpace(void)
{
    while ((work[step] <= C_Space) && (step < len))
        step++;
}

// trim the rear white space
void DataTransform::SkipThisLine(void)
{
    while ((work[step] > ' ') && (work[step] < 0x7f) && (step < len))
        step++;
    TrimSpace();
}

// always start from the step
int DataTransform::RecordLength(void)
{
    int i = step;
    while ((work[i] != C_CR) && (work[i] != C_LF) && (work[i] <= 0x7f) && (i < len))
        i++;
    return i - step;
}

// HP 64000 Absolute
bool DataTransform::isHP64000Absolute(void)
{
    if ((work[1] == 4) && // HP 64000 Absolute header
        (work[0x12] == 0) && (work[0x13] == 7) && // processor information record
        (len > 0x25)) // check the size (include the first data record)
    { // records linkage test till the end of sample
        bool Result = true; // Suppose it's right format first
        int i = 0x1c; // the index of the first block
        do
            // Visit all Length structure, Check two byte, first byte is always 0,
            // the second byte is always point to next buffer.
            // Once the linkage is broken or reach the end of 4K sample buffer
            // then break the test loop
            if (work[i] != 0)
                Result = false;
            else
                i += (int)work[i + 1] + 3; // step to the next record
        while (Result && (i < len));
        return Result;
    }
    else
        return false;
}

// Holtek OTP, 2013 1202 Extent Holtek OTP to Holtek OTP/MTP
// Header(C0+2D+00+data list)
// (88+LL+LL+Data List)(88+LL+LL+Data List)...
// OTP Program(65+LL+LL+Data List+Checksum)
// OTP Option(65+LL+LL+Data List+Checksum),
// MTP Voice Data(65+LL+LL+00 00 30 00+Data List+Checksum) (for HT86R, HT86ARx, HT86BRx)
// MTP Data EEPROM(65+LL+LL+00 00 00 80+Data List+Checksum) Âà 9x00 0x08000000
// MTP SPI Flash Data(65+LL+LL+00 00 00 F0+Data List+Checksum) Âà 9x00 0x0F000000
// End(88+LL+LL+Data List)
bool DataTransform::isHoltekOTPMTP(void)
{
    bool Result = false;
    if (work[0] == 0xC0) // Header
    {
       int idx = 3 + (int)work[1] + (((int)work[2]) << 8);
       if ((idx < C_BufSize) && ((work[idx] == 0x65) || (work[idx] == 0x88)))
            Result = true;
    }
    return Result;
}

// CypressIIC
// Header('w 00 A0 03 ')
// Data(122 byte ASCII Hex)
// Unknow(hh hh hh)
// End(0D 0A 0D 0A)
bool DataTransform::isCypressIIC(void)
{
    if ((work[0] != 'w') || (work[1] != ' ')) // Header
        return false;
    else
        for (int i = 2; i <= 378; i++) // if not (work[i] in [#$D, #$A, ' ', '0'..'9', 'A'..'F']) then
            if ((work[i] != C_CR) && (work[i] != C_LF) && (work[i] != C_Space) && ((work[i] < 0x30) || (work[i] > 0x39)) && 
                ((work[i] < 0x41) || (work[i] > 0x46)))
                return false;
    return true;
}

// Formated Binary (Data I/O)
// ArrowHead4nibble : array[0..5] of Byte = (8, $1c, $2a, $49, 8, 0);
// ArrowHead8nibble : array[0..5] of Byte = (8, $1c, $3e, $8b, 8, 0);
bool DataTransform::isFormatedBinary(void)
{
    int i, idx;
    // note, check the procedure removeFileNull, I need the $8 for this format
    bool Result = false;
    removeFileNull();
    idx = step;
    // check the Arrow Head, two type: 4 nibble and 8 nibble
    if ((step < len) &&
        (work[idx] == C_ArrowHead4nibble[0]) &&
        (work[idx + 1] == C_ArrowHead4nibble[1]) &&
        (work[idx + 4] == C_ArrowHead4nibble[4]) &&
        (work[idx + 5] == C_ArrowHead4nibble[5]))
    {
        int nibble = 0;
        if ((work[idx + 2] == C_ArrowHead4nibble[2]) && (work[idx + 3] == C_ArrowHead4nibble[3]))
            nibble = 4;
        else
            if ((work[idx + 2] == C_ArrowHead8nibble[2]) && (work[idx + 3] == C_ArrowHead8nibble[3]))
                nibble = 8;
        // check the nibble byte count
        if (nibble > 0)
        {
            idx += 6; // the size of ArrowHead4nibble and ArrowHead8nibble
            for (i = 0; i < nibble; i++) // byte count area, all data is nibble data
                if (((work[idx + i]) & 0xf0) != 0) // high nibble must be 0
                    return Result;
            // check the rubout character
            idx += nibble;
            if (work[idx] == 0xff)
                Result = true; // don't care the checksum
        }
    }
    return Result;
}

// ASCII Binary
// The Following function only check one record. if it match then return True
// Note, don't modify the variable step in these functions.

// BxxxxxxxxF, where x in "PN10HL"
// BPPPPNNNNF B11110000F BHHHHLLLLF, ASCIIBinary
bool DataTransform::isASCIIBinary(void)
{
    int idx = step;
    if (work[idx] == 0x42)  // 'B'
    {
        for (int i = 0; i < 8; i++)
            if (strchr(C_ASCIIBinaryBITS, work[idx + i]) == NULL) // C_ASCIIBinaryBITS = "PN10HL"
                return false;
        if (work[idx + 9] == 0x4F) // 'F'
            return true;
    }
    return false;
}

// @aaaa hh hh hh hh hh hh hh
bool DataTransform::isTIText(void)
{
    int cnt;
    bool Result = true;
    if (work[step] == C_ATSign) // First is '@'
    {  
        step++;
        cnt = 0;
        // addres is preceded by a '@', must contain 2 to 8 hex
        while ((strchr(C_HEXDIGIT0Aa, work[step]) != NULL) && (step < len - 2))
        {
            cnt++;
            step++;
        }
        if (cnt > 0)
        do
            if ((work[step]==C_Space) || (work[step] == C_LF) || (work[step] == C_CR) || (work[step] == C_ATSign) || 
                (strchr(C_HEXDIGIT0Aa, work[step]) != NULL))
            {
                cnt++;
                step++;
            }
            else
            {
                if (work[step] != 'q') // ending character 'q'
                    Result = false; // Other character, format error
                break;
            }
        while (step < (len - 2));
        if (Result && (cnt < 5)) // '@a h?
            Result = false;
    }
    else
        Result = false;
    return Result;
}

// // Comment area
// @aaaa hhhh
// ª`·NWord Addressing
// C_EnpirionROM
bool DataTransform::isEnpirionROM(void)
{
    bool Result = false;
    removeFileNull();
    if (step < len)
    {
        int idx;
        if (work[step] != C_ATSign)
        { // Skip comment area
            idx = step + 1;
            while (((strchr(C_CRLF, work[idx]) == NULL) || (work[idx + 1] != C_ATSign)) && (idx < len - 9))
                idx++;
            if ((strchr(C_CRLF, work[idx]) != NULL) && (work[idx + 1] == C_ATSign))
                step = idx + 1;
        }
        // This address is preceded by a '@', must contain 4 hex digits address, blank and 4 hex digits data
        if (work[step] == C_ATSign) // Check '@aaaa hhhh'
        {
            Result = true;
            for (idx=1; idx<=4; idx++)
                if (strchr(C_HEXDIGIT0Aa, work[step + idx]) == NULL)
                    Result = false;
            if (work[step + 5] != C_Space)
                Result = false;
            if (Result && (strchr(C_HEXDIGIT0Aa, work[5]) == NULL))
                for (idx=6; idx<=9; idx++)
                    if (strchr(C_HEXDIGIT0Aa, work[step + idx]) == NULL)
                        Result = false;
        }
    }
    return Result;
}

bool DataTransform::isEXARCFG(void)
{
    bool Result = false;
    if (work[step] == C_Apostrophe) // '
    {
        if (work[step + 1] == 'C')
        {
            if (strstr((char*)&work[step + 1], "Customer'") != NULL) // locate "Customer'"
                Result = true;
        }
        else
            if (work[step + 1] == 'c') // 'ch1' :
            {
                if (strstr((char*)&work[step + 1], "ch1'") != NULL) // apostrophe
                    Result = true;
            }
            else
                if (work[step + 1] == 'r') // 'registers' :
                {
                    if (strstr((char*)&work[step + 1], "registers'") != NULL) // apostrophe
                        Result = true;
                }
                else
                    if (work[step + 1] == 'o') // 'otp' :
                    {
                        if (strstr((char*)&work[step + 1], "otp'") != NULL)
                            Result = true;
                    }
    }
    return Result;
}

bool DataTransform::isRichtekProg(void)
{
    bool Result = false;
    removeFileNull();
    if ((step < len) && (work[step] == '[') && (work[step + 1] == 'H'))
        if (strstr((char *)&work[step + 1], "Header]") != NULL)
            if (strstr((char *)&work[step + 10], "Description:") != NULL)
                if (strstr((char *) &work[step + 22], "Richtek Memory Program File") != NULL)
                    Result = true;
    return Result;
}

bool DataTransform::isRichtekRRF(void)
{
   return (work[0] == 0x52) && (work[1] == 0x52) && (work[2] == 0x46) && (work[3] == 0) && // "RRF\0" 
          (work[4] >= 0x20) && (work[4] <= 0x24) && (work[5] == 00); // 20h, 21h, 22h (' ', '!', '"')
}

bool DataTransform::isPOFTag17(void)
{
    union { uint32_t w; uint8_t b[2]; } numw;
    union { uint64_t l; uint8_t b[4]; } numl;
    bool rev; // Little Endian
    bool Result = false;

    if ((work[0] == 0x50) && (work[1] == 0x4F) && (work[2] == 0x46) && (work[3] == 0)) // "POF"
    {
        // ref PageHeap
        rev = (work[4] == 0) && (work[5] == 0); // little endian
        step = 12; // POF_HEADER: Always 12B, ref function fgetHeader: Boolean;
        do  // scan body for the different tags display
        {
            if (rev)
            {
                numw.b[0] = work[step++];
                numw.b[1] = work[step++];
            }
            else
            {
                numw.b[1] = work[step++];
                numw.b[0] = work[step++];
            }
            Result = numw.w == 17; // Logical_Address_and_Data_32 tag = 17
            if (rev)
            {
                numl.b[0] = work[step++];
                numl.b[1] = work[step++];
                numl.b[2] = work[step++];
                numl.b[3] = work[step++];
            }
            else
            {
                numl.b[3] = work[step++];
                numl.b[2] = work[step++];
                numl.b[1] = work[step++];
                numl.b[0] = work[step++];
            }
            step += numl.l; // number of bytes in rest of packet
        } while ((step < (C_BufSize-6)) && !Result);
    }
    else
        Result = false;
    return Result;
}

// ASCII Hex
// start code is nonprintable STX or SOM
// end code is nonprintable EOM or ETX
// if a new start code follows within 16 characters of an end code, input will
// continue uninterrupted.

// '$A0000,                     for address
// 'FF 00 11 55 AA              for data block
// '$S0000                      for checksum
// Data in these formats is organized in sequential bytes separated
// by the execute character(space, percent, apostrophe, or comma).
// Characters immediately preceding the execute character are
// interpreted as data.
// ASCII-Hex and Octal formats can express 8-bit data, by 3 octal,
// or 2 Hex characters. Line feeds, carriage returns and other
// characters may be included in the data stream as long as a data
// byte directly precedes each execute character.
bool DataTransform::isASCIIHex(void)
{
    bool Result = false;
    int idx;
    if (work[step] == C_DollarSign)
        if ((work[step + 1] == 'A') || (work[step + 1] == 'S'))
        {
            // so far, the ASCII Hex is just the same as the ASCII Oct
            idx = step + 2;
            // This address is preceded by a '$' and a 'A', must contain 2 to 8 hex
            // or 3 to 11 octal characters, and must be followed by a comma, except
            // for the ASCII-HEX (Comma) format, which uses a period.
            while ((strchr(C_HEXDIGIT, work[idx]) != NULL) && (idx < len) && (idx < step + 10))
                idx++;
            if ((work[idx]==C_Comma) || (work[idx]==C_Period)) // Comma or period
            { // check the size of data. 2 digit in Hex for each data
                idx++;
                // Get execute character
                do
                {
                    while ((work[idx]!=C_Space) && (work[idx]!=C_PercentSign) && (work[idx]!=C_Apostrophe) && (work[idx]!=C_GraveAccent) && (work[idx]!=C_Comma))
                        idx++;
                } while (work[idx-1] <= C_Space);
                // Check the 2 characters just precedes each execute character.
                // They must be two hex digits.
                // But check the one more character. if ASCII Oct '$A0000. 000,'
                // will be identified as ASCII hex.
                if ((strchr(C_HEXDIGIT, work[idx - 1]) != NULL) &&
                    (strchr(C_HEXDIGIT, work[idx - 2]) != NULL) &&
                    (strchr(C_OCTDIGIT, work[idx - 3]) == NULL)) // Make sure it's not ASCII OCT
                    Result = true;
            }
        }
    return Result;
}

// ASCII Oct
// '$A000000,                   for address
// '377 000 000 255 001         for data block
// '$S0000                      for checksum
bool DataTransform::isASCIIOct(void)
{
    bool Result = false;
    int idx;
    if (work[step] == C_DollarSign)
        if ((work[step + 1] == 'A') || (work[step + 1] == 'S'))
        {
            // so far, the ASCII Oct is just the same as the ASCII Hex
            idx = step + 2;
            // This address is preceded by a '$' and a 'A', must contain 2 to 8 hex
            // or 3 to 11 octal characters, and must be followed by a comma, except
            // for the ASCII-HEX (Comma) format, which uses a period.
            while ((strchr(C_OCTDIGIT, work[idx]) != NULL) && (idx < len) && (idx < step + 13))
                idx++;
            // Comma or period
            if ((work[idx] == C_Comma) || (work[idx] == C_Period)) // Comma or period
            { // check the size of data. 3 digit in Oct for each data
                idx++;
                // Get execute character
                do
                {
                    while ((work[idx] != C_Space) && (work[idx] != C_PercentSign) && (work[idx] != C_Apostrophe) && (work[idx] != C_GraveAccent) && (work[idx] != C_Comma))
                        idx++;
                } while (work[idx - 1] <= C_Space);
                // Check the 3 characters just precedes execute character
                // They must be 3 Octal digits.
                if ((strchr(C_OCTDIGIT, work[idx - 1]) != NULL) &&
                    (strchr(C_OCTDIGIT, work[idx - 2]) != NULL) &&
                    (strchr(C_OCTDIGIT, work[idx - 3]) != NULL))
                    Result = true;
            }
        }
    return Result;
}

// ST NVM, Example file :
// #0 AADDAA0031FFFFFFFFFFFFFFFFFFFFFF
// #1 657440675928A0E00640006030AC8697
// #2 920204CC04C6156B3CE24CCF0C1A20D8
// #3 1FCE5504B056BA1D0000000000347604
// #4 25F56415040000000000000000000000
// #5 AADDAA0031FFFFFFFFFFFFFFFFFFFFFF
// #6 A007A818008015C02500807DC04C1839
// #7 C0F2904340DCF207C81F38C000210124
// #8 0400A005C012FA8888A468AC869EA80E
// #9 00000D0F000066A60A2D1F6D06483210
// #A 10F80E380224780090E1E7E6E79A3042
// #B 0E4FE4A9D1A228A0EDCF0008128A8981
// #C B98981A9899181899909920200000000
// #D 0000000000000000000000000000F779
// '#p hhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhh' addr=p*16
// CRC16 (preset value = 0x1D0F)
bool DataTransform::isSTNVM(void)
{
    bool Result = false;
    step = 0;
    if ((step < len) && (work[step] == C_NumberSign)) // #pppppppp hhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhh
    {
        int cnt = 0;
        int idx = step + 1;
        // This address paragraph contains 1 to 3 hex
        while ((strchr(C_HEXDIGIT0Aa, work[idx]) != NULL) && (idx < len) && (idx < step + 9))
        {
            idx++;
            cnt++;
        }
        if (cnt > 0) // address
        {
            cnt = 0;
            do
                if ((work[idx] == C_Space) || (work[idx] == C_HT)) // Space and HT (Tab Key)
                {
                    idx++;
                    cnt++;
                }
                else
                    break;
            while (idx < len - 2);
            if (cnt > 0)
            {
                cnt = 0;
                while ((strchr(C_HEXDIGIT0Aa, work[idx]) != NULL) && (idx < len))
                {
                    idx++;
                    cnt++;
                }
                if ((idx < len - 1) && (cnt > 0))
                    Result = true;
            }
        }
    }
    return Result;
}

// In this formats, bytes are recorded in ASCII codes with binary digits
// represented by 1's and 0's. During output, each byte is preceded by a
// decimal address. During input, all addresses are ignored.
// Bytes are sandwiched betweed the space and carriage return characters and
// are normally separated by line feeds. The start code is nonprintable STX,
// ctrl B (of hex 02), and the end code is a nonprintable ETX, ctrl C
// (or hex 03).
// <STX>0000 11111111   Address cide is 4 decimal digits
//      0001 11111110   4 or 8 data bits appears between the space and the CR
//      0002 11111101
//      0003 11111100
//      0004 11111011
//      0005 11111010
//      0006 11111001
//      0007 11111000
//      0008 11110111
//      0009 11110110
//      0010 11110101
//      0011 11110100<ETX>
bool DataTransform::isSpectrum(void)
{
    int i;
    for (i = 0; i <= 3; i++)
        if (strchr(C_DECDIGIT, work[step + i]) == NULL)
            return false;
    if (work[step + 4] == C_Space)
        for (i = 0; i <= 3; i++) // 4 or 8 bit binary digit
            if ((work[step + 5 + i] != '0') && (work[step + 5 + i] != '1'))
                return false;
    return true;
}

// Use ASCII to record each byte
// 0000 0000 0100 0D10 1818 FFFF FFFF FFFF
// 3001 FFFF 642B 016D 1028 1076 8086 B284
// 20DD 2222 1600 2F90 2380 0012 1E20 0012
bool DataTransform::isStraight(void)
{
    int i = 0;
    bool hex = false, space = false, crlf = false, other = false;
    while (i < len)
    {
        if (strchr(C_HEXDIGIT0Aa, work[i]) != NULL)
            hex = true;
        else
            if ((work[i] == C_CR) || (work[i] == C_LF))
                crlf = true;
            else
                if ((work[i] == C_Space) || (work[i] == C_HT))
                    space = true;
                else
                    other = true;
        i++;
    }
    return (hex && space && crlf && !other);
}

// //%SYSTEM
// //CPU0|U1 (C0  - 8860B)|0xC0|8860B|0
// //%RAIL
// //%RAIL_END
// //%CONFIG//8860B - 0xC0
// 
// 4000	00100000	//ReadWriteByte
// 4001	1010101111001101	//ReadWriteWord
bool DataTransform::isInfineonPSF(void)
{
    return (work[0] == C_Slash) && (work[1] == C_Slash) && (work[2] == C_PercentSign);
}

// #DF 1-13-2009
// #DC c728
// #SR 2008.12.5544
// #DN
// #PT
// #HF 01
// 06
// 02 00 00 00 7e aa 99 7e 51 00 01 05
// 02 00 00 08 92 00 20 62 03 67 72 01
// . . .
// 02 03 36 58 00 00 00 00 00 00 00 00
// 02 03 36 60 00 00 22 c7 28 01 06 00
// 04
// #FC b21f
// C_LatticeNVCM:    g_LatticeNVCM(@mScanRec);
bool DataTransform::isLatticeNVCM(void)
{
    return (work[0] == '#') &&
        (((work[1] == 'D') && ((work[2] == 'F') || (work[2] == 'C') || (work[2] == 'N'))) ||
        ((work[1] == 'I') && (work[2] == 'D')) ||
        ((work[1] == 'P') && (work[2] == 'T')) ||
        ((work[1] == 'S') && (work[2] == 'R')) ||
        ((work[1] == 'O') && (work[2] == 'T')) ||
        ((work[1] == 'V') && (work[2] == 'C')) ||
        ((work[1] == 'H') && (work[2] == 'F')) ||
        ((work[1] == 'C') && (work[2] == 'M')));
}

// <SHA204_Format>
// <!--You must verify that this is your custom part number-->
// <PartNumber>ATSHA204-TSU-01-T</PartNumber>
// 2015 1209 ¼W¥[¿ëÃÑ²v, °{±¼¥i¯àªº <?xml version="1.0" encoding="utf-8"?>
bool DataTransform::isATSHA204XML(void)
{
    bool Result = false;
    for (int i= 0; i<=128; i++)
        if ((work[i] == '<') && (work[i + 1] == 'S') && (work[i + 2] == 'H') && (work[i + 3] == 'A') &&
            (work[i + 4] == '2') && (work[i + 5] == '0') && (work[i + 6] == '4') && (work[i + 7] == '_') && (work[i + 8] == 'F'))
        {
            Result = true;
            break;
        }
    return Result;
}

// <?xml version="1.0" encoding="utf-8"?>
// <ATECC508A>
//    <XMLVersion>01 00 04</XMLVersion>
//    <PartNumber>ATECC508A-MAHXX-T</PartNumber>
bool DataTransform::isATECCXML(void)
{
    bool Result = false;
    for (int i=0; i<=128; i++)
        if ((work[i] == '<') && (work[i + 1] == 'A') && (work[i + 2] == 'T') && (work[i + 3] == 'E') && (work[i + 4] == 'C') && (work[i + 5] == 'C'))
        {
            Result = true;
            break;
        }
    return Result;
}

//bool DataTransform::isInfineonSBSL(void)
//{
//    ext, fn_kc, fn_ip: string;
//begin
//    ext : = ExtractFileExt(FFileName);
//    if CompareText(ext, '.properties') = 0 then
//    begin
//        ext : = ChangeFileExt(FFileName, '');
//        fn_kc: = ext + '_kc.ldf';
//        fn_ip: = ext + '_ip.ldf';
//        Result: = FileExistsEx(fn_kc) and FileExistsEx(fn_ip);
//    end
//    else
//        Result: = False;
//}

bool DataTransform::isTICSV(void)
{
    bool Result = false;
    removeFileNull();
    char *str = strupr(gets()); // UpperCase(Trim(
    if ((str != NULL) && (step < len) && (strlen(str) > 6))
        Result = ((strstr(str, "COMMENT,") != NULL) && (strstr(str, ",FORMAT=CSV;") != NULL)) ||
            (strstr(str, "WRITEBYTE,")!=NULL) ||
            (strstr(str, "WRITEWORD,") != NULL) ||
            (strstr(str, "BLOCKWRITE,") != NULL) ||
            (strstr(str, "READBYTE,") != NULL) ||
            (strstr(str, "READWORD,") != NULL) ||
            (strstr(str, "BLOCKREAD,") != NULL) ||
            (strstr(str, "PAUSE,") != NULL) ||
            (strstr(str, "SENDBYTE,") != NULL) ||
            (strstr(str, "BLOCKPROCESSCALL,") != NULL) ||
            (strstr(str, "RESET") != NULL);
    return Result;
}

// //*********************************
// // Add comments here.
// // Do not edit any line below CRC32
// //*********************************
// CRC32 : 0x01319B5E
// XDP System File Version : 1.0
// XCF File Version : 1.0
bool DataTransform::isInfineonXSF(void)
{
    int cnt = 0;
    char *str;
    removeFileNull();
    do 
    {
        str = strupr(gets()); // UpperCase(Trim(
        if ((str != NULL) && (strlen(str) > 2) && (str[1] != '/') && (str[2] != '/')) // skip comment area
            if ((strstr(str, "CRC32")==str) || (strstr(str, "XDP SYSTEM FILE VERSION")==str))
                cnt++;
    } while ((step < len) && (cnt < 2));
    return cnt == 2;
}

bool DataTransform::isInfineonXCF(void)
{
    int cnt = 0;
    char *str;
    removeFileNull();
    do
    {
        str = strupr(gets());
        if ((str != NULL) && (strlen(str) > 2) && (str[1] != '/') && (str[2] != '/'))
            if ((strstr(str, "CRC32") == str) || (strstr(str, "XCF FILE VERSION") == str))
                cnt++;
    } while ((step < len) && (cnt < 2));
    return cnt == 2;
}


////Device Type: FS8510A0
////Date: 7/1/2020
////Customer: NXP
//
//MAIN_OTP
//:100014002007AC0D8C078806881C95937F00380058
//:0A0024000430180A1C1C000104013E
//:00000001FF
//
//FS_OTP
//:0E000A0088FF00FFFFFFFFFF3F3F7F00240441
//:00000001FF
bool DataTransform::isPMICHex(void)
{
    int en = 0;
    char *str;
    removeFileNull();
    do
    {
        str = gets();
        if ((str != NULL) && (strlen(str) > 2))
        {
            if ((str[0] == '/') && (str[1] == '/')) // comment
                en |= 1;
            else
                if ((str[0] == ':') && isIntelHexRecord(str, strlen(str))) // intel hex record
                    en |= 2;
                else
                    if (strstr(str, "_OTP") != NULL) // MAIN_OTP or FS_OTP
                        en |= 4;
        }
    } while ((step < len) && (en != 7));
    return (en == 7);
}

bool DataTransform::isNXPiMXeFuse(void)
{
    int cnt = 0;
    removeFileNull();
    char *str = strupr(gets()); // fuse_bank,fuse_word,fuse_addr,ocotp_addr,value,,,
    if ((str != NULL) && (strlen(str) > 46))
        if ((strstr(str, "FUSE_ADDR") != NULL) && (strstr(str, "VALUE") != NULL))
            for (int i=0; i<strlen(str); i++)
                if (str[i] == ',')
                    cnt++;
    return cnt > 2;
}

bool DataTransform::isNXPI2CText(void)
{
    char* str;
    int en = 0;
    removeFileNull();
    // Register Output for PF0100 programmer
    // Generated from Spreadsheet Revision: P1.8
    // WRITE_I2C:7F:01 // Access FSL EXT Page 1
    do
    {
        str = strupr(gets());
        if (str != NULL)
        {
            if (strstr(str, "REGISTER OUTPUT") != NULL)
                en |= 1;
            if ((strstr(str, "GENERATED") != NULL) && (strstr(str, "SPREADSHEET") != NULL))
                en |= 2;
            if (strstr(str, "WRITE_I2C:") != NULL)
                en |= 4;
        }
    } while ((step < len) && (en!=7));
    return en == 7;
}

bool DataTransform::isSiliconText(void)
{
    char* str;
    int en = 0;
    removeFileNull();
    do
    {
        str = strupr(gets());
        if (str != NULL)
            if ((strstr(str, "# SI") == str) && (strstr(str, "REGISTERS SCRIPT") != NULL))
                en |= 1;
            else
                if (strstr(str, "# START CONFIGURATION PREAMBLE") == str)
                    en |= 2;
                else
                    if (strstr(str, "# START CONFIGURATION REGISTERS") == str)
                        en |= 4;
                    else
                        if (strstr(str, "# START CONFIGURATION POSTAMBLE") == str)
                            en |= 8;
    } while ((step < len) && (en!=0xf));
    return (en == 0xf);
}

bool DataTransform::isTDKTXT(void)
{
    int en = 0;
    char* str;
    removeFileNull();
    do
    {
        str = strupr(gets());
        if ((str != NULL) && (strlen(str) > 10))
        {
            if (strstr(str, "#DEVICE FAMILY") == str)
                en |= 1;
            else
                if (strstr(str, "#RAIL NAME") == str)
                    en |= 2;
                else
                    if (strstr(str, "#I2C ADDRESS") == str)
                        en |= 4;
        }
    } while ((step < len) && (en!=7));
    return (en == 7);
}

//R0 0x0010
//R9 0x0901
//R16 0x1000
//R30	0x1E00
//R31	0x1F00
//R49	0x3110
//R50	0x3200
//R56	0x3800
//R72	0x4802
bool DataTransform::isTIOSCReg(void)
{
    removeFileNull();
    char* str;
    char* ptr;
    int i, j, k;
    int cnt = 0;
    do
    {
        str = strupr(gets());
        if ((str != NULL) && (strlen(str) > 0))
            if (str[0] == 'R')
            {
                ptr = strchr(str, ' ');
                if (ptr == NULL)
                    ptr = strchr(str, C_Tab);
                if (ptr != NULL)
                    i = ptr - str;
                else
                    i = -1;
                ptr = strstr(str, "0X");
                if (ptr != NULL)
                    j = ptr - str;
                else
                    j = -1;
                if ((i > 0) && (j > i))
                {
                    for (k = 1; k <= i; k++)
                        if (strchr(C_DECDIGITst, str[k]) == NULL) // not (str[k] in [#9, ' ', '0'..'9'])
                        {
                            cnt = -1;
                            break;
                        }
                    for (k = j; k < strlen(str); k++)
                        if (strchr(C_HEXDIGITsx, str[k]) == NULL) // not (str[k] in [' ', '0'..'9', 'a'..'f', 'x'])
                        {
                            cnt = -2;
                            break;
                        }
                    if (cnt >= 0)
                        cnt++;
                }
            }
            else
                break;
    } while ((cnt >=0) && (cnt <= 5));
    return (cnt > 5);
}

bool DataTransform::isTITPSTXT(void)
{
    bool first = true;
    bool tps = true;
    char* str;
    char* token;
    int en = 0;
    removeFileNull();
    do
    {
        str = strupr(gets());
        if ((str != NULL) && (strlen(str) > 3))
        { // be careful there is no ' ' but #9
            if (first && (strstr(str, "REGADDR")!=NULL) && (strstr(str, "VALUE") != NULL)) // "regdump.txt"
                tps = false;
            if (tps) //"SAC105437.000.txt"
            {
                //"name": "CHIPID",
                //"addr" : 0x00,
                //"value" : 0x15,
                if ((strchr(str, ':') != NULL) && (strchr(str, ',') != NULL))
                {
                    token = strtok(str, C_TITPSTXT_SEPS); // break string with ' ', '\t', ':', ',' 
                    if ((token != NULL) && (strlen(token) > 0))
                        if (strstr(token, "\"NAME\"") == token)
                            en |= 1;
                        else
                            if ((strstr(token, "\"ADDR\"") == token) || (strstr(token, "\"ADDRESS\"") == token))
                                en |= 2;
                            else
                                if (strstr(token, "\"VALUE\"") == token)
                                    en |= 4;
                  //token = strtok(NULL, C_TITPSTXT_SEPS); // continue break string
                }
            }
            else
                if (!first) // "regdump.txt"
                {
                    //RegAddr		Value
                    //00			00
                    //01			00
                    int i, j;
                    token = strtok(str, C_TITPSTXT_SEPS); // break string with white space
                    if ((token != NULL) && (strlen(token) > 0))
                    {
                        j = 0;
                        en |= 1;
                        for (i = 0; i < strlen(token); i++)
                            if (strchr(C_HEXDIGIT, token[i]) == NULL)
                                j++;
                        if (j == 0) // Pure Digit
                            en |= 2;
                    }
                    token = strtok(NULL, C_TITPSTXT_SEPS); // continue break string
                    if ((token != NULL) && (strlen(token) > 0))
                    {
                        j = 0;
                        for (i = 0; i < strlen(token); i++)
                            if (strchr(C_HEXDIGIT, token[i]) == NULL)
                                j++;
                        if (j == 0) // Pure Digit
                            en |= 4;
                    }
                }
                else
                    first = false;
        }
    } while ((step < len) && (en != 7));
    return (en == 7);
}

bool DataTransform::SkipComment(void)
{
    do
        step++;
    while ((step < len) || (work[step] != '*'));
    bool Result = (step == len) || ((step < len) && (work[step] == '*'));
    step++;
    return Result;
}

bool DataTransform::SkipDec(char del, bool QF)
{
    bool Result = false;
    if (QF) FuseQty = 0;
    do
    {
        step++;
        if (strchr(C_DECDIGIT, work[step]) == NULL) // not (work[step] in ['0'..'9'])
            break; // break while loop
        else
            if (QF)
                FuseQty = FuseQty * 10 + aschex(work[step]);
    } while ((step < len) || (work[step] != del));

    if (step == len)
        Result = true;
    else
        if (step > len)
            Result = false;
        else
            if (work[step] == del) // '*', ' '
                Result = true;
            else
                if (del == C_Space)
                    Result = (work[step]==C_CR) || (work[step] == C_LF) || (work[step] == C_Tab) || (work[step] == C_Space);
    return Result;
}

bool DataTransform::SkipHex(void)
{
    do step++;
    while ((step < len) && (strchr(C_HEXDIGIT, work[step])!=NULL) && (work[step] != '*'));
    return ((step == len) || ((step < len) && (work[step] == '*')));
}

bool DataTransform::SkipBinary(void)
{
    do step++;
    while ((step < len) && (strchr(C_JEDECBINARY, work[step])!=NULL) && (work[step] != '*'));
    return (step >= len) || (work[step] == '*');
}

bool DataTransform::isJEDEC(void)
{
    int Lcnt = 0;
    bool Result = false;
    //FuseQty:=0; FuseQty is side effect if (Dev.pdid=PLDRICH) and format=JEDEC
    step = 0;
    if (work[step] == C_STX) // <STX> {<field>} <ETX>;
        if (SkipComment() && (step < len))
        {
            step++;
            Result = true;
            while (Result && (step < len) && (Lcnt <= 5))
            {
                switch (work[step])
                {
                case 'N':
                    if (!SkipComment())
                        Result = false;
                    break;
                case 'Q':
                    step++;
                    if ((step >= len) || ((work[step] != 'P') && (work[step] != 'F')) || !SkipDec('*', (work[step] == 'F')))
                        Result = false;
                    break;
                case 'G': case 'F': case 'U': case 'E':
                    if (!SkipBinary())
                        Result = false;
                    break;
                case 'L':
                    if (!SkipDec(' ', false) || !SkipBinary())
                        Result = false;
                    else
                        Lcnt++; //if (Lcnt > 5) break; Good Enough
                    break;
                case 'C':
                    if (!SkipHex())
                        Result = false;
                    break;
                case C_ETX:
                    return Result; // Transmission Checksum ?
                case C_CR: case C_LF:
                    break;
                default:
                    Result = false;
                }
                step++;
            }
        }
    return Result;
}

bool DataTransform::isLatticeFEA(void)
{
    int l = 0;
    int d = 0;
    int k = 0;
    int cnt = 0;
    char* name;
    char* value;
    removeFileNull();
    do
    {
        char* str = strupr(gets());
        if (str != NULL)
        {
            l++;
            name = strtok(str, C_sColon);
            if (name != NULL)
            {
                value = strtok(NULL, C_sColon);
                if ((strstr(name, "CREATED BY") != NULL) && (strstr(value, "LATTICE") != NULL))
                    k = l; // CREATED BY:	Lattice Semiconductor
                if (value != NULL)
                {
                    if ((strstr(name, "DESIGN NAME") != NULL) || // DESIGN NAME:	XO3LFP_test_file_impl1.ncd
                        (strstr(name, "DEVICE NAME") != NULL) || // DEVICE NAME:	LCMXO3D-4300HC
                        (strstr(name, "CREATION DATE") != NULL)) // CREATION DATE:	Wed May 12 16:39:30
                        cnt++;
                    if (strstr(name, "DATA") != NULL) // DATA:
                    {
                        cnt++;
                        d++;
                    }
                }
            }
        }
    } while ((step < len) && (d <= 0));
    return ((d > 0) && (k > 0) && (cnt >= 3));
}

// PEM is a certificate file. (Base64 encoded file.)
//-----BEGIN RSA PRIVATE KEY-----
//MIIJKgIBAAKCAgEAo+N/ASGLdaFs3mnpW6fqUOXVYbQcx9t/i077C9VOcND4uUfQ
//JdblXW2HBJ+paa66+QIH1xY9e9b1AuAotpg9zWj1REnXvkfcOP4/AI1AVs5sXgmK
//3ebdh0VyO0RWQgk34XxMESx7LvqZuAOyCPDeOS+CNJ7mBb64fvje5oVEyeBGLjbB
// . . .
//FYYrOcg0jnGHV2uJ6W+5ZLrkD6sxmhEo/d3peZ9/VR2VAqPaN3NhmJyvbEYd6rCV
//tiH8rg50JeHPykiaM09AO+/+7qgNQCcB+Yx3Qo8NIFuce6SS3Q/3RQSJbJSNlA==
//-----END RSA PRIVATE KEY-----
bool DataTransform::isPEM(void)
{
    int cnt = 0;
    int i, j;
    removeFileNull();
    char *str = gets(); // first line
    if ((str != NULL) && (strlen(str) > 16) && (strstr(str, "---") == str) && (strstr(str, "BEGIN") != NULL) && (strstr(str, "KEY") != NULL))
        do
        {
            str = gets();
            j = strlen(str);
            if (j > 0)
                if (strstr(str, "---") == str)
                    break;
                else
                    for (int i = 0; i < j; i++)
                        if ((Base64DecodeChars[str[i]] < 0) && (str[i] != '='))
                        {
                            cnt = 0;
                            break;
                        }
                        else
                            cnt++; // pass one line

        } while ((step < len) && (cnt < 3));
    return (cnt >= 1);
}

bool DataTransform::isPlainHex(void)
{
    int cnt = 0;
    int i, j, hex;
    char* str;
    bool h;

    removeFileNull();
    do
    {
        h = true;
        str = strupr(gets());
        if (str != NULL)
        {
            j = strlen(str);
            if (j > 0)
            {
                hex = 0;
                for (i = 0; i < j; i++)
                    if (strchr(C_HEXDIGIT, str[i]) != NULL)
                        hex++;
                    else
                        if ((str[i] != ' ') && (str[i] != '\t'))
                            h = false;
                        else
                            if (hex == 2)
                            {
                                cnt++;
                                hex = 0;
                            }
                            else
                                if (hex != 0)
                                    h = false;
                if (hex == 2) // not space or tab key, each line two hex digits
                    cnt++;
            }
        }
    } while ((step < len) && (cnt < 8) && h);
    return (cnt >= 8);
}

// The following function check and transfer the file record into the data chunk
// The reuslt is in the RecordData::line.
// But in the TformDownload.DetectFileFormat. You'll find some identify function
// not all of them can translate the record into address-tag-cnt-data.
// Some of them are the word type format.
// Some of them are stream type. The address is not included in each record.
// Need more code to keep the address.

// 'Shhhhhh...'
//  Start Character: 1 hex digit; 0, 1, 2, 3, 7, 8, 9
//  Byte Count: 2 hex digit; 00..FF
//  Address: 4 digit for tag 0, 1, 8, 9
//           6 digit for tag 2
//           8 digit for tag 3, 7
// 2-characcter checksum, the one's complement of the preceding
// bytes in the record, including byte count, address and data byte
// 2015 0508 Word Addressing
bool DataTransform::isMotorolaSRecord(char* rec, int len)
{
    bool Result = false;
    if (*rec++ == 'S')
        if (isTextHex(rec, 3)) // check tag, byte count
        {
            line.tag = aschex(*rec++);
            line.cnt = asc2hex(rec); // count the checksum from Byte Count
            if (isTextHex(rec, (line.cnt + 1) * 2)) // pure hexadecimal check
                if (SumHexText(rec, line.cnt + 1) == 0xff) // 1's C // false is pass
                {
                    rec += 2;
                    // The format is correct now.
                    switch (line.tag) // get address
                    {
                    case 2:
                        line.address = str2long(rec, 6);
                        rec += 6;
                        break;
                    case 3: case 7:
                        line.address = str2long(rec, 8);
                        rec += 8;
                        break;
                    default: // tag: 0, 1, 8, 9 or unknown
                        line.address = str2long(rec, 4);
                        rec += 4;
                        break;
                    }
                }
            // get data
            for (int i = 0; i <= line.cnt - 4; i++)
            {
                line.data[i] = asc2hex(rec);
                rec += 2;
            }
            Result = true;
        }
        else
            if (++isMotorolaSButSumError > 2)
                Result = true;
    return Result;
}

// ':hhhhhhhh...'
// Byte Count:  2 hex digit, 0..ff
// Address:     4 hex digit
// Record Type: 2 hex digit, 0: data record,
//                           1: end of file record
// Word data:   Byte Count*4 hex digit
// Checksum:    2 hex digit, 2's C sum from Byte Count

// Adjust the word address and byte count after the record is correct
bool DataTransform::isMicrochipINHXRecord(char* rec, int len)
{
    bool Result = false;
    if (*rec++ == ':')
        if (isTextHex(rec, 8)) // check area of byte count, address, record type
        {
            line.cnt = asc2hex(rec); // count the checksum from Byte Count
            if ((line.cnt != 0) &&
                (len >= (line.cnt * 4 + 11)) && // check the length of this word type format
                isTextHex(rec, line.cnt * 4 + 10))
                if (SumHexText(rec, line.cnt * 2 + 5) == 0) // 2's C // false is pass
                {
                    rec += 2;
                    line.address = str2long(rec, 4) * 2;
                    rec += 4;
                    line.tag = asc2hex(rec);
                    rec += 2;
                    line.cnt *= 2;
                    for (int i = 0; i < line.cnt; i++)
                    {
                        line.data[i] = asc2hex(rec);
                        rec += 2;
                    }
                    Result = true;
                }
                else
                    if (++isMicrochipINHXButSumError > 2) 
                        Result = true; // ??
        }
    return Result;
} // isMicrochipINHXRecord, note. the address and cnt is adjusted to byte form

// ':hhhhhh....'
// Address: 4 hex digit, 0..ffff
// Byte Count: 2 hex digit
// Address Checksum: 2 hex digit sum, each byte xor with previous byte and rotate left
// data list: 2 * Byte Count hex digit
// Data Checksum: each byte xor with previous byte and rotate left
//
// Note, No tag, terminated by Byte Count=0
bool DataTransform::isSigneticsHexRecord(char* rec, int len)
{
    bool Result = false;
    if (*rec++ == ':')
        if (isTextHex(rec, 6)) // check area of byte count, address, record type
        {
            uint32_t check = XorRotateSum(rec, 3);
            line.address = str2long(rec, 4);
            rec += 4;
            line.cnt = asc2hex(rec);
            rec += 2; // count the checksum from Byte Count
            if ((line.cnt == 0) && (len < 8))
                Result = true;
            else // cannot check but pass
                if (isTextHex(rec, line.cnt * 2 + 4)) // check from the Address Check to end
                    if (asc2hex(rec) == check)
                    {
                        rec += 2; // step to the data area
                        check = XorRotateSum(rec, line.cnt);
                        for (int i = 0; i < line.cnt; i++) // get data
                        {
                            line.data[i] = asc2hex(rec);
                            rec += 2;
                        }
                        if (asc2hex(rec) == check)
                            Result = true;
                        else
                            if (++isSigneticsHexButSumError > 2)
                                Result = true;
                    }
                    else
                        if (++isSigneticsHexButSumError > 2)
                            Result = true;
        }
    return Result;
}

// ':hhhhhhhh...'
// Byte Count:  2 hex digit, 0..ff
// Address:     4 hex digit
// Record Type: 2 hex digit, 0: data record,
//                           1: end of file record
// Word data:   Byte Count*2 hex digit
// Checksum:    2 hex digit, 2's C sum from Byte Count
bool DataTransform::isIntelHexRecord(char* rec, int len)
{
    bool Result = false;
    if (*rec++ == ':')
        if (isTextHex(rec, 8)) // check area of byte count, address, record type
        {
            line.cnt = asc2hex(rec); // count the checksum from Byte Count
            // 移除(cnt<>0) 一開始有可能是End Of Record
            if ((len >= (line.cnt * 2 + 11)) && isTextHex(rec, line.cnt * 2 + 10)) // check the length of this word type format
                if (SumHexText(rec, line.cnt + 5) == 0) // 2's C false is pass
                {
                    rec += 2;
                    line.address = str2uint(rec, 4); rec += 4;
                    line.tag = asc2hex(rec); rec += 2;
                    for (int i = 0; i < line.cnt; i++)
                    {
                        line.data[i] = asc2hex(rec);
                        rec += 2;
                    }
                    Result = true;
                }
                else
                    if (++isIntelHexButSumError > 2) // Intel Hex But checksum error
                        Result = true; // ???
        }
    return Result;
}

// ':hhhhhhhh...'
// Byte Count:  2 hex digit, 0..ff
// Address:     4 hex digit
// Record Type: 2 hex digit, 0: data record,
//                           1: end of file record
// Word data:   Byte Count*2 hex digit
// Checksum:    2 hex digit, 1's C sum from Byte Count
bool DataTransform::isAnalogDeviceHexRecord(char* rec, int len)
{
    bool Result = false;
    if (*rec++ == ':')
        if (isTextHex(rec, 8)) // check area of byte count, address, record type
        {
            line.cnt = asc2hex(rec); // count the checksum from Byte Count
            if ((line.cnt != 0) &&
                (len >= (line.cnt * 2 + 11)) && // check the length of this word type format
                isTextHex(rec, line.cnt * 2 + 10))
                if ((SumHexText(rec, line.cnt + 5) & 0xFF) == 1) // 2's C +1, Analog Device RD, what are u doing?
                {
                    rec += 2;
                    line.address = str2long(rec, 4); rec += 4;
                    line.tag = asc2hex(rec); rec += 2;
                    for (int i = 0; i < line.cnt; i++)
                    {
                        line.data[i] = asc2hex(rec);
                        rec += 2;
                    }
                    Result = true;
                }
        }
    return Result;
}

// '%hhhh......' note. you need a special transfer table
// Header field
//   Block length: 2 hex digit
//   Block type: 1 hex digit,
//               3: Symbol block, ignore the message portion
//               5: Memory space block, ignore the message portion
//               6: Data block
//               8: Termination block
//   Checksum: 2 hex digit sum of all the characters in the block
//             does not include the leading %, the checksum digits or LF/CR
bool DataTransform::isExtendedTekhexRecord(char* rec, int len)
{
    bool Result = false;
    int i;
    // check the ASCII code of Block Length, Block Type, Checksum.
    if (*rec++ == '%')
        if (isTextHex(rec, 6))
        {
            char* str = rec;
            // get Block Length
            uint32_t BlockLength = asc2hex(rec);
            rec += 2; // real record length
            // get Block Type
            line.tag = aschex(*rec); rec++;
            // get Check Sum
            uint32_t  check = asc2hex(rec); rec += 2;
            if ((line.tag == 3) || (line.tag == 5) || (line.tag == 6) || (line.tag == 8)) // the valid block type [3, 5, 6, 8]
                if (SumExtTek(str, BlockLength) == check) // false is pass
                {
                    Result = true;
                    if ((line.tag == 6) || (line.tag == 8))
                    {

                        check = aschex(*rec); rec++;
                        line.address = 0;
                        for (i = 1; i <= check; i++)
                        {
                            line.address += aschex(*rec);
                            rec++;
                        }
                    }
                    switch (line.tag)
                    {
                    case 6:
                        // data block
                        line.cnt = (BlockLength - check - 6) / 2;
                        for (i = 0; i < line.cnt; i++)
                        {
                            line.data[i] = asc2hex(rec);
                            rec += 2;
                        }
                        break;
                    case 8: // Termination Block
                        // The logical address where program execution is to begin.
                        line.cnt = 0; // for the mose file format usage
                        break;
                    }
                }
                else
                    if (++isExtendedTekhexButSumError > 2)
                        Result = true;
        }
    return Result;
}

// '/hhhhh...'
// Address: 4 hex digit
// Byte Count: 2 byte
// 1st sum: the six hex digit of the Address and Byte Count. (nibble sum)
// data: 2n, n=[1..30] digit. A maximum of 30 data bytes is allowed.
// 2nd sum: sum of the 2n Hex digits of the data field
//
// Note, No tag, terminated by Byte Count=0
//       '/010006070202020202020C
//       '/10000001'
bool DataTransform::isTektronixHexRecord(char* rec, int len)
{
    bool Result = false;
    if (*rec++ == '/')
        if (isTextHex(rec, 8)) // check area of byte count, address, 1st sum
        {
            uint32_t check = NibbleSum(rec, 6); // nibble sum
            line.address = str2long(rec, 4); rec += 4;
            line.cnt = asc2hex(rec); rec += 2; // the maximum is 30 byte
            if (line.cnt > 0)
                if (asc2hex(rec) == check)
                {
                    rec += 2; // move to data field
                    if (isTextHex(rec, line.cnt * 2 + 2))
                    {
                        check = NibbleSum(rec, line.cnt * 2);
                        for (int i = 0; i < line.cnt; i++)
                        {
                            line.data[i] = asc2hex(rec);
                            rec += 2;
                        }
                        if (asc2hex(rec) == check)
                            Result = true;
                        else
                            if (++isTektronixHexButSumError > 2)
                                Result = true;
                    }
                }
                else
                    if (++isTektronixHexButSumError > 2)
                        Result = true;
        }
    return Result;
}

// TISDSMAC
// Too much leading character (tag). Word type format.
// (Not every record start from a address field (9). Each record can start
// from the data field (tag B, *).
// Data file in the SDSMAC format consist of a Start-of-File record, data records,
// and an End -of-File record.

// Each record is composed of a series of small fields, each initiated by a tag
// character. The programmer recognizes and acknowledges following tag characters:

// 0 or K - followed by a file header. (Start-of-File; 12-character file header)
//          the first four hex digit are the byte count of the 16-bit data bytes;
//          The remaining file header characters are the name of the file and may
//          be any ASCII characters(in hex notation).
// 7      - followed by a checksum which the programmer acknowledges.
// 8      - followed by a checksum which the programmer ignores.
//          The record ends with a checksum field initiated by the tag character
//          7 or 8, a 4-digit checksum, and the tag F. The checksum is the two's
//          complement of the 8-bit ASCII values of the characters, begin with
//          the first tag character and ending with the checksum tag character
//          (7 or 8).
// 9      - followed by a load address.
//          If any data field appear before the first address field int the file,
//          the first of those data fields is assigned to address 0.
// B      - followed by 4 data characters
// *      - followed by 2 data characters
//          Data records follow the same format as the Start-of-File record
//          but do not contain a file header.
// F      - denotes the end of a data record
// :      - The End-of-File consists of a colon (:) only. The output translator
//          send a control S ($13 DC3 XOFF) after the colon.
// This function just check the legality of the file. It cannot translate the
// record into data.
bool DataTransform::isTISDSMAC(char* rec, int len)
{
    uint32_t sum;
    int j;
    uint8_t* str;
    uint8_t* tmp;

    bool Result = false;
    int i = 0;
    str = (uint8_t*)rec; // keep the start point of the string
    do
    {
        line.tag = *rec++;
        i++;
        switch (line.tag)  // sort by the frequency
        {
        case '9': case 'B':
            if ((i > len - 4) || !isTextHex(rec, 4))
                return Result;
            rec += 4;
            i += 4;
            break;
        case '0': case 'K':
            if ((i > len - 12) || !isTextHex(rec, 4))
                return Result;
            rec += 12;
            i += 12;
            break;
        case '*':
            if ((i > len - 2) || !isTextHex(rec, 2))
                return Result;
            rec += 2;
            i += 2;
            break;
        case '7': case '8':
            if ((i > len - 4) || !isTextHex(rec, 4))
                return Result;
            if (line.tag == '7') // check the sum
            {
                sum = 0;
                tmp = str;
                for (j = 1; j <= i; j++) // include the '7'
                    sum -= *tmp++;
                if ((sum & 0xffff) == str2uint(rec, 4))
                {
                    rec += 4;
                    i += 4;
                    if (*rec == 'F')
                        Result = true;
                }
            }
            else // '8': ignore the checksum
            {
                rec += 4;
                i += 4;
                if (*rec == 'F')
                    Result = true;
            }
            break;
        default:
            return Result;
            break;
        }
    } while (!Result && (i < len));
    return Result;
}

int DataTransform::ConvertFile(QString fn, ENUMFORMAT format, qint64 filepos)
{
    SrcFilename = fn;
    QFile src(fn);
    SrcSize = src.size();
    if (src.open(QIODevice::ReadOnly | QFile::ExistingOnly))
        if (SrcSize > 0)
        {
            if (filepos > 0)
                src.seek(filepos);
            readSoFar = filepos;
            return ConvertFile(&src, SrcSize-filepos, format);
        }
        else
        {
            emit error(tr("(-2) Error! File Size Zero"));
            emit finished();
            return -2;
        }
            
    else
    {
        emit error(tr("(-1) Error! Fail to open file"));
        emit finished();
        return -1;
    }
}

// File Stream already opened (Database blob, ftp file)
// QFile* Src = fp, FSize
// 傳送到 _base _size
int DataTransform::ConvertFile(QFile* fp, qint64 fsize, ENUMFORMAT format)
{
    int Result;
    readSoFar = fp->pos();
    if (format == ENUMFORMAT::None) // 沒指定Format 判斷後先偵測
    {
        // 準備detect file format
        qint64 rSize = SrcSize = fsize;
        if (rSize > C_BufSize)
            rSize = C_BufSize;
        QByteArray ba = fp->read(rSize);
        format = DetectFormat(ba);
        if (format == ENUMFORMAT::None) // 當依然是無法辨識的Format, 也不能當Binary直接傳送, 就是Fail了
        {
            emit error(tr("(-3) Error! Fail to detect file format"));
            return -3;
        }
        fp->seek(readSoFar); // move file position back
    }
    
    Src = fp;
    switch (format)
    {
    case ENUMFORMAT::Binary: Result = trBinary();
    case ENUMFORMAT::IntelHex: Result = trIntelHex();
    case ENUMFORMAT::MicrochipINHX: Result = trMicrochipINHX();
    case ENUMFORMAT::TektronixHex: Result = trTektronixHex();
    case ENUMFORMAT::MotorolaS: Result = trMotorolaS();
    case ENUMFORMAT::SigneticsHex: Result = trSigneticsHex();
    case ENUMFORMAT::ExtendedTekhex: Result = trExtendedTekhex();
    case ENUMFORMAT::HP64000Absolute: Result = trHP64000Absolute();
    case ENUMFORMAT::Spectrum: Result = trSpectrum();
    case ENUMFORMAT::TISDSMAC: Result = trTISDSMAC();
    case ENUMFORMAT::TIText: Result = trTIText();
    case ENUMFORMAT::ASCIIHex: Result = trASCIIHex();
    case ENUMFORMAT::ASCIIOct: Result = trASCIIOct();
    case ENUMFORMAT::ASCIIBinary: Result = trASCIIBinary();
    case ENUMFORMAT::Straight: Result = trStraight();
    case ENUMFORMAT::FormatedBinary: Result = trFormatedBinary();
    case ENUMFORMAT::HoltekOTPMTP: Result = trHoltekOTPMTP();
    case ENUMFORMAT::CypressIIC: Result = trCypressIIC();
    case ENUMFORMAT::ADIHex: Result = trADIHex();
    case ENUMFORMAT::PEM: Result = trPEM();
    case ENUMFORMAT::PlainHex: Result = trPlainHex();
    case ENUMFORMAT::EXARCFG: Result = trEXARCFG();
    case ENUMFORMAT::ATSHA204XML: Result = trATSHA204XML();
    case ENUMFORMAT::ATECCXML: Result = trATECCXML();
    case ENUMFORMAT::STNVM: Result = trSTNVM();
    case ENUMFORMAT::TICSV: Result = trTICSV();
    case ENUMFORMAT::TITPSTXT: Result = trTITPSTXT();
    case ENUMFORMAT::LatticeNVCM: Result = trLatticeNVCM();
    case ENUMFORMAT::EnpirionROM: Result = trEnpirionROM();
    case ENUMFORMAT::RichtekProg: Result = trRichtekProg();
    case ENUMFORMAT::RichtekRRF: Result = trRichtekRRF();
    case ENUMFORMAT::POF: Result = trPOF();
    case ENUMFORMAT::POFTag17: Result = trPOFTag17();
    case ENUMFORMAT::ADP105xHEX: Result = trADP105xHEX();
    case ENUMFORMAT::IRSalemATE: Result = trIRSalemATE();
    case ENUMFORMAT::InfineonPSF: Result = trInfineonPSF();
    case ENUMFORMAT::InfineonXSF: Result = trInfineonXSF();
    case ENUMFORMAT::InfineonXCF: Result = trInfineonXCF();
    case ENUMFORMAT::IRAcadiaMIC: Result = trIRAcadiaMIC();
    case ENUMFORMAT::XPBELF: Result = trXPBELF();
    case ENUMFORMAT::OpusPMF: Result = trOpusPMF();
    case ENUMFORMAT::INIGUID: Result = trINIGUID();
    case ENUMFORMAT::TIEPR: Result = trTIEPR();
    case ENUMFORMAT::MemoryJEDEC: Result = trMemoryJEDEC();
    case ENUMFORMAT::NXPiMXeFuse: Result = trNXPiMXeFuse();
    case ENUMFORMAT::SiliconText: Result = trSiliconText();
    case ENUMFORMAT::TDKTXT: Result = trTDKTXT();
    case ENUMFORMAT::LatticeFEA: Result = trLatticeFEA();
    default:
        emit error(tr("(-4) Unkown Error!"));
        Result = -4;
    }
    emit finished();
    return Result;
}

// QThread* thread = new QThread;
// Worker* worker = new Worker(); ???
// worker->moveToThread(thread);
// connect(worker, SIGNAL(error(QString)), this, SLOT(errorString(QString)));
// connect(thread, SIGNAL(started()), worker, SLOT(process()));
// connect(worker, SIGNAL(finished()), thread, SLOT(quit()));
// connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()));
// connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
// thread->start();
void DataTransform::process(void)
{
    // allocate rsources using new here (new QObject here. Don't new QObject in ctor)
    // start processing data
    int Result;
    if (!SrcFilename.isNull() && !SrcFilename.isEmpty())
    {
        if (QFile::exists(SrcFilename))
            Result = ConvertFile(SrcFilename, SrcFormat, SrcFilePos);
        else
            Result = -1; // File not found
    }
    else
        if (Src != NULL)
            Result = ConvertFile(Src, SrcSize, SrcFormat);
        else
            Result = -2; 

    // ConvertFile(QString fn, ENUMFORMAT formt = ENUMFORMAT::None, qint64 FilePos = 0);
    // ConvertFile(QFile * fp, ENUMFORMAT formt = ENUMFORMAT::None); // File Stream already opened (Database blob, ftp file)

    Src = NULL;
    SrcFilename = "";
}

int DataTransform::trBinary() 
{ 
    qint64 bytesRead = 64;
    int p;
    int percentProgress = -1;

    p = readSoFar * 100 / SrcSize;
    if (percentProgress != p)
        emit updateProgress(percentProgress = p);
    emit error(QString("Error"));
    // break;
    do
    {
        readSoFar++;
    } while (readSoFar < SrcSize); // or file accessing error

    return 0; 
};

int DataTransform::trIntelHex() { return 0; };
int DataTransform::trMicrochipINHX() { return 0; };
int DataTransform::trTektronixHex() { return 0; };
int DataTransform::trMotorolaS() { return 0; };
int DataTransform::trSigneticsHex() { return 0; };
int DataTransform::trExtendedTekhex() { return 0; };
int DataTransform::trHP64000Absolute() { return 0; };
int DataTransform::trSpectrum() { return 0; };
int DataTransform::trTISDSMAC() { return 0; };
int DataTransform::trTIText() { return 0; };
int DataTransform::trASCIIHex() { return 0; };
int DataTransform::trASCIIOct() { return 0; };
int DataTransform::trASCIIBinary() { return 0; };
int DataTransform::trStraight() { return 0; };
int DataTransform::trFormatedBinary() { return 0; };
int DataTransform::trHoltekOTPMTP() { return 0; };
int DataTransform::trCypressIIC() { return 0; };
int DataTransform::trADIHex() { return 0; };
int DataTransform::trPEM() { return 0; };
int DataTransform::trPlainHex() { return 0; };
int DataTransform::trEXARCFG() { return 0; };
int DataTransform::trATSHA204XML() { return 0; };
int DataTransform::trATECCXML() { return 0; };
int DataTransform::trSTNVM() { return 0; };
int DataTransform::trTICSV() { return 0; };
int DataTransform::trTITPSTXT() { return 0; };
int DataTransform::trLatticeNVCM() { return 0; };
int DataTransform::trEnpirionROM() { return 0; };
int DataTransform::trRichtekProg() { return 0; };
int DataTransform::trRichtekRRF() { return 0; };
int DataTransform::trPOF() { return 0; };
int DataTransform::trPOFTag17() { return 0; };
int DataTransform::trADP105xHEX() { return 0; };
int DataTransform::trIRSalemATE() { return 0; };
int DataTransform::trInfineonPSF() { return 0; };
int DataTransform::trInfineonXSF() { return 0; };
int DataTransform::trInfineonXCF() { return 0; };
int DataTransform::trIRAcadiaMIC() { return 0; };
int DataTransform::trXPBELF() { return 0; };
int DataTransform::trOpusPMF() { return 0; };
int DataTransform::trINIGUID() { return 0; };
int DataTransform::trTIEPR() { return 0; };
int DataTransform::trMemoryJEDEC() { return 0; };
int DataTransform::trNXPiMXeFuse() { return 0; };
int DataTransform::trSiliconText() { return 0; };
int DataTransform::trTDKTXT() { return 0; };
int DataTransform::trLatticeFEA() { return 0; };
