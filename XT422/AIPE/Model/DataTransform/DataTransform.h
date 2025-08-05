#pragma once

#include <QtCore>

// 和UI與系統界面有關的程式, 該用Qt的元件與做法保持Qt跨平台的特性
// 程式內部運行資料管理, 則使用std library與C Library, 以求移植其他開發工具時負擔減小

//#include <set> // for std::set

// The parameter storage of Ctor is QIODevice, it's the working memory buffer. 
// Assign storage as Memory Buffer: a. Chunks (Base unit for sites), b. Data Linked List (DB Chunks or data list of Special Bit)

// Data Source:
// 1. Text/Binary File
// 2. Database Huge Blob File
// 3. FTP/HTTP/FTPS/HTTPS file 
// 4. ESP output serial.dat, parsing T03 for Format Code, T04 for Hex File (motos3, intelhex, aschex)

// Functions: 
// 1. Detect File Format
// 2. Load Data File into Data Storage

// See also, 
// MergeData Class: It can merge data from Chunks and Data Linked List
// Project Editor: Which can read data from Data Linked List for Driver.DB or DataList for Special Bit
// Sparse Matrix: It's for Sequential Data or Special Bit Data. Every piece of data contail ChunkIndex, Address and Data List. 
//                It must be break for Chunk whil insert data.

// enum Format and its companion File Format Name List 
enum ENUMFORMAT {
    None, Binary, IntelHex, MicrochipINHX, TektronixHex, MotorolaS, SigneticsHex, ExtendedTekhex, HP64000Absolute, Spectrum, TISDSMAC,
    TIText, ASCIIHex, ASCIIOct, ASCIIBinary, Straight, FormatedBinary, HoltekOTPMTP, CypressIIC, ADIHex, PEM, PlainHex, EXARCFG, ATSHA204XML,
    ATECCXML, STNVM, TICSV, TITPSTXT, LatticeNVCM, EnpirionROM, RichtekProg, RichtekRRF, POF, POFTag17, ADP105xHEX, IRSalemATE, InfineonPSF,
    InfineonXSF, InfineonXCF, IRAcadiaMIC, XPBELF, OpusPMF, INIGUID, TIEPR, JEDEC, MemoryJEDEC, NXPiMXeFuse, NXPI2CText, SiliconText, TDKTXT, 
    TIOSCReg, LatticeFEA
};

// To check a format can do the Upload function. 
// Usage: CanUpload.count(ENUMFORMAT::SiliconText) == 0,  CanUpload.count(ENUMFORMAT::IntelHex) != 0
//const std::set<ENUMFORMAT> CanUpload{ 
//    ENUMFORMAT::Binary, ENUMFORMAT::IntelHex, ENUMFORMAT::MicrochipINHX, ENUMFORMAT::TektronixHex, ENUMFORMAT::MotorolaS, ENUMFORMAT::SigneticsHex, 
//    ENUMFORMAT::ExtendedTekhex, ENUMFORMAT::HP64000Absolute, ENUMFORMAT::Spectrum, ENUMFORMAT::TISDSMAC, ENUMFORMAT::TIText, ENUMFORMAT::ASCIIHex, 
//    ENUMFORMAT::ASCIIOct, ENUMFORMAT::ASCIIBinary, ENUMFORMAT::Straight, ENUMFORMAT::FormatedBinary
//};

#define C_BufSize 4096

// for all file formats, don't care the checksum
// direct to output by the KK format
struct RecordData {
    uint64_t address;  // Record address, It may not the physical address.
    int tag;           // Tag of the incoming record
    int cnt;           // Byte count
    uint8_t data[256]; // data buffer
};

class DataTransform : public QObject
{
    Q_OBJECT
public: 
    // explicit 
    bool RecordSumError;
    QString ErrorMessage;
    
    DataTransform(QObject* parent); // self make 0 size buffer
    DataTransform(QIODevice& ioDevice, QObject* parent); // outside io device

    ENUMFORMAT DetectFormat(const QString& filename); // 直接開啟檔案
    ENUMFORMAT DetectFormat(const QByteArray& fa); // 外部連接裝置以QFileDevice或QTextStream開啟檔案, 多種來源用這個. 特點程式內不Seek(0)
    ENUMFORMAT DataIOFomatCode(const int ESPT03); // return eFormat
    ENUMFORMAT NameToEnum(const char* FormatName);
    const char* EnumToName(ENUMFORMAT FormatEnum);

signals: 
    // emit progress
    // emit JEDEC secure option enabled/disabled
    // emit Result Code, 0 for Pass, other code with error message

private:
    QIODevice* _base; // storage pointer from ctor will be kept here
    qint64 _size;
    QString FFilename;
    QByteArray bytes; // buffer for gets() to store char * record
    RecordData line;

    unsigned char work[C_BufSize]; // for detecting file format, C_BufSize = 4096
    int step; // working index
    int len; // len=fSize<C_BufSize?fSize:C_BufSize; 
    int FuseQty; // JEDEC 'Q######'

    int isMotorolaSButSumError;
    int isMicrochipINHXButSumError;
    int isSigneticsHexButSumError;
    int isIntelHexButSumError;
    int isExtendedTekhexButSumError;
    int isTektronixHexButSumError;

    bool setIODevice(QIODevice& ioDevice);

    void removeFileNull(void);
    char* gets(void);
    void TrimSpace(void);
    void SkipThisLine(void);
    int RecordLength(void);

    bool isHP64000Absolute(void);
    bool isHoltekOTPMTP(void);
    bool isCypressIIC(void);
    bool isFormatedBinary(void);
    bool isASCIIBinary(void);
    bool isTIText(void);
    bool isEnpirionROM(void);
    bool isEXARCFG(void);
    bool isRichtekProg(void);
    bool isRichtekRRF(void);
    bool isPOFTag17(void);
    bool isASCIIHex(void);
    bool isASCIIOct(void);
    bool isSTNVM(void);
    bool isSpectrum(void);
    bool isStraight(void);
    bool isInfineonPSF(void);
    bool isLatticeNVCM(void);
    bool isATSHA204XML(void);
    bool isATECCXML(void);
//  bool isInfineonSBSL(void);
    bool isTICSV(void);
    bool isInfineonXSF(void);
    bool isInfineonXCF(void);
    bool isPMICHex(void);
    bool isNXPiMXeFuse(void);
    bool isNXPI2CText(void);
    bool isSiliconText(void);
    bool isTDKTXT(void);
    bool isTIOSCReg(void);
    bool isTITPSTXT(void);
    bool isJEDEC(void);
    bool SkipComment(void); // subroutine for isJEDEC
    bool SkipDec(char del, bool QF); // subroutine for isJEDEC
    bool SkipHex(void); // subroutine for isJEDEC
    bool SkipBinary(void); // subroutine for isJEDEC

    bool isLatticeFEA(void);
    bool isPEM(void);
    bool isPlainHex(void);

    bool isMotorolaSRecord(char* rec, int len);
    bool isMicrochipINHXRecord(char* rec, int len);
    bool isSigneticsHexRecord(char* rec, int len);
    bool isIntelHexRecord(char* rec, int len);
    bool isAnalogDeviceHexRecord(char* rec, int len);
    bool isExtendedTekhexRecord(char* rec, int len);
    bool isTektronixHexRecord(char* rec, int len);
    bool isTISDSMAC(char* rec, int len);
};
