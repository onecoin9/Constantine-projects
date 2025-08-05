#include "parser_global.h"
#include <qstring>
#include <QPair>

#define Enable_OverLapped_Strict (0x00)
// external string
char* C_None = "None";
char* C_Binary = "Binary";
char* C_IntelHex = "IntelHex";
char* C_MicrochipINHX = "MicrochipINHX";
char* C_TektronixHex = "TektronixHex";
char* C_MotorolaS = "MotorolaS";
char* C_SigneticsHex = "SigneticsHex";
char* C_ExtendedTekHex = "ExtendedTekHex";
char* C_HP64000Absolute = "HP64000Absolute";
char* C_Spectrum = "Spectrum";
char* C_TISDSMAC = "TISDSMAC";
char* C_TIText = "TIText";
char* C_ASCIIHex = "ASCIIHex";
char* C_ASCIIOct = "ASCIIOct";
char* C_ASCIIBinary = "ASCIIBinary";
char* C_Straight = "Straight";
char* C_FormatedBinary = "FormatedBinary";
char* C_HoltekOTPMTP = "HoltekOTPMTP";
char* C_CypressIIC = "CypressIIC";
char* C_ADIHex = "ADIHex";
char* C_PEM = "PEM";
char* C_PlainHex = "PlainHex";
char* C_EXARCFG = "EXARCFG";
char* C_ATSHA204XML = "ATSHA204XML";
char* C_ATECCXML = "ATECCXML";
char* C_STNVM = "STNVM";
char* C_TICSV = "TICSV";
char* C_TITPSTXT = "TITPSTXT";
char* C_LatticeNVCM = "LatticeNVCM";
char* C_EnpirionROM = "EnpirionROM";
char* C_RichtekProg = "RichtekProg";
char* C_RichtekRRF = "RichtekRRF";
char* C_POF = "POF";
char* C_POFTag17 = "POFTag17";
char* C_ADP105xHEX = "ADP105xHEX";
char* C_IRSalemATE = "IRSalemATE";
char* C_InfineonPSF = "InfineonPSF";
char* C_InfineonSBSL = "InfineonSBSL";
char* C_InfineonXSF = "InfineonXSF";
char* C_InfineonXCF = "InfineonXCF";
char* C_IRAcadiaMIC = "IRAcadiaMIC";
char* C_XPBELF = "XPBELF";
char* C_OpusPMF = "OpusPMF";
char* C_INIGUID = "INIGUID";
char* C_TIEPR = "TIEPR";
char* C_MemoryJEDEC = "MemoryJEDEC";
char* C_NXPiMXeFuse = "NXPiMXeFuse";
char* C_NXPI2CText = "NXPI2CText";
char* C_SiliconText = "SiliconText";
char* C_TDKTXT = "TDKTXT";
char* C_LatticeFEA = "LatticeFEA";

char* C_C2000OUT = "C2000 out";
char* C_ACTELSTP = "ActelSTP";
char* C_CHIL = "CHIL";
char* C_HEX = "HEX";
char* C_JBC = "JBC";
char* C_XSVF = "XSVF";
char* C_DAT = "DAT";
char* C_CVME = "CVME";
char* C_ACT = "ACT";
char* C_PSF = "PSF";
char* C_INTERSIHEX = "IntersiHex";
char* C_JED = "JED";
char* C_PMF = "PMF";
char* C_MIC = "MIC";
char* C_TIARMOUT = "TIARM Out";
char* C_MPS = "MPS";
char* C_MPSXLSX = "MPS XLSX";
char* C_MPSTYPEB = "MPSParserTypeB";
char* C_MPSTYPEC = "MPSParserTypeC";
char* C_MPSTYPED = "MPSParserTypeD";
char* C_MPSTYPEE = "MPSParserTypeE";
char* C_MPSHR12 = "MPSHR12Parser";
char* C_MPSHR12TXT = "MPS_HR12xx_txtParser";

QString C_ErrMsg_UserAbort   = QObject::tr("User Abort!");
QString C_ErrMsg_FileZero    = QObject::tr("File Size is Zero!");
QString C_ErrMsg_FormatError = QObject::tr("File Format Error!");
QString C_ErrMsg_RecordSum   = QObject::tr("Record Sum Error!");
QString C_ErrMsg_FileOpen    = QObject::tr("Fail to Open File!");
QString C_ErrMsg_FileRead    = QObject::tr("Fail to Read File!");
QString C_ErrMsg_WriteError  = QObject::tr("Buffer Write Error!");
QString C_ErrMsg_MemAllocErr = QObject::tr("Memory Alloc Error!");

unsigned char C_ArrowHead4nibble[6] = { 8, 0x1c, 0x2a, 0x49, 0x8, 0x0 };
unsigned char C_ArrowHead8nibble[6] = { 8, 0x1c, 0x3e, 0x6b, 0x8, 0x0 };
uint32_t ExttekTable[128] =  // array[$24..$7A]
{ 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00,
  00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00,
  00, 00, 00, 00, 36, 37, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00,
   0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 00, 00, 00, 00, 00, 00,
  00, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
  25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 00, 00, 00, 00, 39,
  00, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54,
  55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 00, 00, 00, 00, 00
};

char* C_ASCIIBinaryBITS = "PN10HL";
char* C_OCTDIGIT = "01234567";
char* C_DECDIGIT = "0123456789";
char* C_DECDIGITst = " \t0123456789"; // Space Tab 0~9
char* C_HEXDIGITsx = " X0123456789ABCDEF"; // Space X 0~9 A~F
char* C_HEXDIGIT = "0123456789ABCDEF";
char* C_HEXDIGITe = "ABCDEFabcdef";
char* C_HEXDIGIT0Aa = "0123456789ABCDEFabcdef";
char* C_CRLF = "\r\n";
char* C_TITPSTXT_SEPS = " \t:,"; // Separators/Delimiter
char* C_JEDECBINARY = "01 \t\n\r";
char* C_sColon = ":"; // for isLatticeFEA

void AbstractParser::InitControl(void)
{
    FuseQty = 0; // For JEDEC
    LineNum = 0; // for current parsing line number
    FilePos = 0;
    FileSize = 0;
    percentProgress = -1;
    ferror = 0; // io error happened
    feof = false;
    userabort = false;
}

AbstractParser::AbstractParser()
{
    // for TransferFile()
    InitControl();
    // ConfirmFormat()
    len = 0;
    step = 0;
    for (int i = 0; i < C_BufSize; i++)
        work[i] = 0;
}

// Slot: 接收外來用戶停止轉換的訊號, 設定內部控制變數
void AbstractParser::UserAbort(void)
{
    userabort = true;
}

// Check cnt number of char in part of outside string
bool AbstractParser::isTextHex(uint8_t* str, int cnt)
{
    for (int i = 0; i < cnt; i++)
        if (strchr(C_HEXDIGIT0Aa, str[i]) == NULL) // ['0'..'9', 'A'..'F', 'a'..'f']
            return false;
    return true;
}

// Input the ASCII (ch1) and return an integer value (int)
uint8_t AbstractParser::aschex(const uint8_t ch1)
{
    // make 'a'..'f' capital, but '0'..'9' into 0x10..0x19
    if (ch1 <= 0x39) // '0'..'9'
        return (uint8_t)(ch1 - 0x30);
    else // 0x41 'A'..'F' --> 0x61 'a'..'f', 61h-57h = 10
        return (uint8_t)((ch1 | 0x20) - 0x57);
}

// Input ASCII string "HH..." and  return an integer value HH;
uint8_t AbstractParser::asc2hex(uint8_t* str)
{
    uint8_t Result = aschex(*str++) << 4;
    return (Result | aschex(*str));
}

// Input ASCII string "HH..." and  return an integer value HH;
uint32_t AbstractParser::asc2Dec(uint8_t* str)
{
    uint32_t Result = aschex(*str++) * 10;
    return Result + aschex(*str); // 注意型別轉換, 是否有符號擴展帶來的錯誤
}

// Convert 4 hex ASCII string to unsigned int
uint32_t AbstractParser::asc2int(uint8_t* addr) // "0123..." == (int.hex)0123
{
    union { uint32_t w; uint8_t b[2]; } numw;
    numw.b[1] = asc2hex(addr);
    numw.b[0] = asc2hex(addr + 2);
    return numw.w;
}

// can deal with the 8 digits or 6 digits or error input
// str2int, str2long
uint32_t AbstractParser::str2uint(uint8_t* str, const int len)
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

uint64_t AbstractParser::str2long(uint8_t* str, const int len)
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

//uint32_t AbstractParser::Str2Hex(const unsigned char* str, unsigned char* buf, int size)
//{
//    return 0;
//}


// for record type data format, summing up CNT, ADDR,... area.
uint32_t AbstractParser::head_sum(uint8_t* str, int sumstytle)
{
    uint16_t i, j, t;
    switch (sumstytle)
    {
    case 0: for (i = j = 0; i < 4; i++)
        j -= asc2hex(str + i * 2);
        break; // 2's C
    case 1: for (i = j = 0; i < 6; i++)
        j += aschex(*(str + i));
        break; // nibble sum
    case 2: t = aschex(*str) + 2;
        for (i = j = 0; i < t; i++)
            j -= asc2hex(str + i * 2 + 1);
        break; // 1's C
    case 3: for (i = j = 0; i < 3; i++)
    {
        j ^= asc2hex(str + i * 2);
        // rotate left
        j <<= 1; 
        if (j & 0x100)
        {
            j |= 1;
            j &= 0xff;
        }
      // inline assemble cannot be used in othere OS and x64  
      //__asm mov ax, j
      //__asm rol al, 1
      //__asm mov j, ax
    }
          break; // XOR
    case 4: for (i = j = 0; i < 3; i++)
        j += aschex(*(str + i));
        break; // nibble sum
    }
    return j;
}

// must check with the function isTextHex
// must mask with value $ff
uint32_t AbstractParser::SumHexText(uint8_t* str, int len)
{
    uint32_t Result = 0;
    for (int i = 0; i < len; i++)
    {
        Result += asc2hex(str);
        str += 2;
    }
    return Result & 0xff; // Result:=Result mod $100
};

int AbstractParser::RecordLength(void)
{
    int i = step;
    while ((work[i] != C_CR) && (work[i] != C_LF) && (work[i] <= 0x7f) && (i < len))
        i++;
    return i - step;
}

void AbstractParser::removeFileNull(void)
{
    step = 0; // goto the start of a file
    if ((work[0] == 0xEF) && (work[1] == 0xBB) && (work[2] == 0xBF)) // remove UTF-8 header
        step = 3;
    // skip the leading character which in (NUL, SOH, STX, SOM, LF, CR)
    while ((((work[step] >= 0) && (work[step] <= 2)) || (work[step] == '\r') || (work[step] == '\n') || (work[step] == 0x12)) && (step < len))
        step++;
}

void AbstractParser::UpdateProgress(int curProg)
{
    int value;
    if (curProg != -1)
        emit Progress(percentProgress = curProg);
    if (FileSize > 0)
    {
        value = FilePos * 100 / FileSize;
        if (value != percentProgress)
            emit Progress(percentProgress = value);
    }
}

// 00, CR, LF, STX will be ignored
// FF FE: utf-16 BOM
// EF BB BF: utf-8 BOM
bool AbstractParser::SkipNullHead(QFile *src)
{
    int LFCnt = 0;
    char data;
   
    do
    {
        if (!src->getChar(&data))
            emit ErrorRpt(ferror = src->error(), src->errorString());
        else
            if (data == C_LF)
                 LFCnt++;
    } while (((data < C_Space) || (uint8_t(data) > 0x7F)) && (ferror==0) && !src->atEnd() && !userabort);
    LineNum += LFCnt;
    if (src->atEnd())
        feof = true;
    if (feof || ferror || userabort)
    {
        if (userabort)
            emit ErrorRpt(ferror = C_Err_UserAbort, C_ErrMsg_UserAbort);
        // ferror had already emited.    
        return false;
    }
    else
    {
        src->ungetChar(data);
        return true;
    }
}

// Ignored the char 0x0d, 0x0a, 0 that leading in file
// Jump to Record Type format leading character
bool AbstractParser::SkipNullHeadToTag(QFile* src, uint8_t tag, bool chktag)
{
    int LFCnt = 0;
    char data;
    do
    {
        if (!src->getChar(&data))
            emit ErrorRpt(ferror = src->error(), src->errorString());
        else
            if (data == C_LF)
                LFCnt++;
            else
                if ((data > C_CR) && (data < C_Space))
                    emit ErrorRpt(ferror = C_Err_FormatError, C_ErrMsg_FormatError);
                else
                    if (chktag && (data > C_Space) && (data != tag))
                        emit ErrorRpt(ferror = C_Err_FormatError, C_ErrMsg_FormatError);
    } while ((data != tag) && (ferror == 0) && !src->atEnd() && !userabort);
    LineNum += LFCnt;
    if (src->atEnd())
        feof = true;
    if (feof || ferror || userabort)
    {
        if (userabort)
            emit ErrorRpt(ferror = C_Err_UserAbort, C_ErrMsg_UserAbort);
        // ferror had already emited.    
        return false;
    }
    else
    {
        src->ungetChar(data); // special C function, you can unget a char
        return true;
    }
}

bool AbstractParser::myfgets(QFile* src)
{
    int LFCnt = 0;
    char data;
    do {
        if (!src->getChar(&data))
            emit ErrorRpt(ferror = src->error(), src->errorString());
        else
            if (data == C_LF)
                LFCnt++;
    } while ((data <= C_Space) && (ferror==0) && !src->atEnd());
    if (src->atEnd())
        feof = true;
    int i = 0;
    if ((ferror==0) && !feof)
    {
        work[i] = (uint8_t)data;
        for (i = 1; i < C_BufSize; i++)
        {
            if (src->atEnd())
                break; // feof is still false, because line data was just read
            else
                if (!src->getChar(&data))
                    emit ErrorRpt(ferror = src->error(), src->errorString());
                else
                {
                    if (data == C_LF)
                    {
                        LFCnt++;
                        break;
                    }
                    if (data == C_CR)
                        break;
                    if (data != C_NUL)
                        work[i] = (uint8_t)data;
                }
        }
    }
    LineNum += LFCnt;
    work[i] = C_NUL;
    if (i>0)
        FilePos = src->pos();
    return ((i > 0) && (ferror == 0));
}

int AbstractParser::mynRead(QFile* src, int size) 
{
    int LFCnt = 0;
    char data;
    if (src->atEnd())
        feof = true;
    int i = 0;
    if ((ferror == 0) && !feof)
    {
        for (i = 0; i < size && i < C_BufSize; i++)
        {
            if (src->atEnd())
                break; // feof is still false, because line data was just read
            else
                if (!src->read(&data, 1))
                    emit ErrorRpt(ferror = src->error(), src->errorString());
                else
                {
                    work[i] = (uint8_t)data;
                }
        }
    }
    if (i > 0)
        FilePos = src->pos();
    return i;
}

bool AbstractParser::myRead(QFile* src, int& readLen)
{
    int LFCnt = 0;
    char data;
    if (src->atEnd())
        feof = true;
    int i = 0;
    if ((ferror == 0) && !feof)
    {
        for (i = 0; i < C_BufSize; i++)
        {
            if (src->atEnd())
                break; // feof is still false, because line data was just read
            else
                if (!src->read(&data, 1))
                    emit ErrorRpt(ferror = src->error(), src->errorString());
                else
                {
                    work[i] = (uint8_t)data;
                    readLen = i + 1;
                }
        }
    }
    if (i > 0)
        FilePos = src->pos();
    return ((i > 0) && (ferror == 0));
}


// return actual length of string
int AbstractParser::removeBackNul(uint8_t *str)
{
    int len = strlen((char *)str) - 1;
    while (len >= 0) // remove tailed space, CR, LF, TAB
    {
        switch (*(str + len))
        {
        case ' ': case C_TAB: case C_CR: case C_LF: len--; continue;
        }
        break;
    }
    str[++len] = 0;
    return len;
}

// for record type data format, perform the data area transfer and download.
uint32_t AbstractParser::get_rec(uint8_t *str, int cnt, qint64 loadaddr, int sumstytle, QIODevice *dst)
{
    QByteArray buf;
    uint8_t k;
    uint16_t i, j, s;
    for (s = i = j = 0; i < cnt; i++)
    {
        buf.append(k = asc2hex(str + i * 2));
        s -= k; // 2's C
        if (sumstytle == 1)
            j += (k & 0x0f) + ((k >> 4) & 0x0f); // nibble sum
        else
            if (sumstytle == 3) // XOR
            {
                j ^= k;
                // rotate left
                j <<= 1;
                if (j & 0x100)
                {
                    j |= 1;
                    j &= 0xff;
                }
              // inline assemble cannot be used in othere OS and x64  
              //_asm mov ax, j
              //_asm rol al, 1
              //_asm mov j, ax
            }
    }
    if (dst)
    {
        dst->seek(loadaddr); // Heap檔不夠大 無法任意指定超出的高位址來擴張檔案大小
        qint64 written = dst->write(buf, buf.size());
        if (written != buf.size())
            emit ErrorRpt(ferror = C_Err_WriteError, C_ErrMsg_WriteError);
      //dst->flush();
      //QFile::flush();
    }
    if (sumstytle == 0 || sumstytle == 2)
        j = s;
    return j;
}


bool AbstractParser::CheckFlashTableInfo(unsigned long DataAddr, tFlashTableInfo FlashTableInfo)
{
    bool Rtn = false;
    QString strMsg;
    tFlashTableWriteInfo item;
    QPair<unsigned int, tFlashTableWriteInfo> kvPair;

    if (m_pWriteFlashTableMap == NULL) {
        goto __end;
    }

#if 1//Enable_OverLapped_Strict
    if (m_pWriteFlashTableMap->find(FlashTableInfo.Address) != m_pWriteFlashTableMap->end()) {
        strMsg = "There is overlapped in buffer, Failed to write 'FlashTableInfo' into buffer";
        m_pOutput->Error(strMsg.toLocal8Bit().data());
        goto __end;
    }

    if (m_pWriteFlashTableMap->size() >= 1) {
        QMap<unsigned int, tFlashTableWriteInfo>::iterator it = m_pWriteFlashTableMap->begin();
        for (; it != m_pWriteFlashTableMap->end(); it++)
        {
            unsigned int StartAddr = it.key();
            unsigned int EndAddr = StartAddr + it.value().Len - 1;
            if (FlashTableInfo.Address >= StartAddr && FlashTableInfo.Address <= EndAddr) {
                strMsg = "There is overlapped in buffer, Failed to write 'FlashTableInfo' into buffer";
                m_pOutput->Error(strMsg.toLocal8Bit().data());
                goto __end;
            }
        }
    }
#endif

    item.BuffAddr = DataAddr; //buffer地址
    item.Len = FlashTableInfo.Len;


    m_pWriteFlashTableMap->insert(FlashTableInfo.Address, item);

    Rtn = true;

__end:
    return Rtn;
}

int AbstractParser::BufferWrite(ADR adrStart, unsigned char* pBuf, ADR adrLen)
{

    if (adrStart > m_Dst->size()) {
        char* tmp = new char[adrStart - m_Dst->size()];
        memset(tmp, 0xFF, adrStart - m_Dst->size());
        m_Dst->seek(m_Dst->size());
        m_Dst->write(tmp, adrStart - m_Dst->size());
        delete[] tmp;
    }
    
    m_Dst->seek(adrStart);
    return m_Dst->write((const char*)pBuf, adrLen);
}