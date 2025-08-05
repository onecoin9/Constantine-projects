#include "TektronixHex.h"
#include <QFile>

// Intel Intellec 8/MDS Format/Intel MSC-86 Hexadecimal OBJ
TektronixHex::TektronixHex()
{
    isTektronixHexButSumError = 0;
    line.address = 0;
    line.tag = 0;
    line.cnt = 0;
    for (int i = 0; i < C_LineDataSize; i++)
        line.data[i] = 0;
}

char* TektronixHex::getVersion() 
{ 
    return "1.0.0.0"; 
}

char* TektronixHex::getFormatName() 
{ 
    return C_TektronixHex;
}

bool TektronixHex::ConfirmFormat(QString& filename)
{ 
    qint64 rSize;
    QFile fptr(filename);

    if (fptr.open(QFile::ReadOnly | QFile::ExistingOnly))
    {
        rSize = fptr.size();
        if (rSize > C_BufSize)
            rSize = C_BufSize;
        QByteArray ba = fptr.read(rSize); // 如果是Sparse Matrix
        len = ba.size();
        if (len > C_BufSize)
            len = C_BufSize;
        memcpy(work, ba.constData(), len);
        removeFileNull(); // assign step 0
        bool ret = isTektronixHexRecord(work + step, RecordLength());

        fptr.close();
        return ret;
    }
    else
        return false; 
}

// For the Tekhex
// Note, the len is the number of digits
uint32_t TektronixHex::NibbleSum(uint8_t* str, int len)
{
    uint32_t Result = 0;
    for (int i = 0; i < len; i++)
        Result += (uint32_t)aschex(*str++);
    return Result & 0xff; // return Result % 0x100;
}

// '/hhhhh...'
// Address: 4 hex digit
// Byte Count: 2 byte
// 1st sum: the six hex digit of the Address and Byte Count. (nibble sum)
// data: 2n, n=[1..30] digit. A maximum of 30 data bytes is allowed.
// 2nd sum: sum of the 2n HexParser digits of the data field
//
// Note, No tag, terminated by Byte Count=0
//       '/010006070202020202020C
//       '/10000001'
bool TektronixHex::isTektronixHexRecord(uint8_t* rec, int len)
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

/// <summary>
/// Tektronix Hexadecimal Format
/// </summary>
/// <param name="srcfn"> soure data filename </param>
/// <param name="dst"> destination QFile pointer </param>
/// <returns>  </returns>
int TektronixHex::TransferFile(QString& srcfn, QIODevice* dst)
{
    m_Dst = dst;
    qint64 aaaa = 0, lastaddr = 0;
    int t, cnt, lastcnt = -1;
    uint32_t ss;
    bool HaveEndRecord = false, chktag = false, WordAddressing = false;

    InitControl();
    QFile src(srcfn);
    FileSize = src.size();
    if (FileSize <= 0)
        emit ErrorRpt(ferror = C_Err_FileZero, C_ErrMsg_FileZero);
    else
        if (src.open(QIODevice::ReadOnly))
        {
            chktag = false;
            do {

                if (SkipNullHeadToTag(&src, '/', chktag) && myfgets(&src))
                {
                    chktag = true;
                    t = removeBackNul(work); // trim tailed white space and control key, return length
                    if (isTextHex(work + 1, t - 1))
                    {
                        ss = head_sum(work + 1, 1); // checksum from BB to TT
                        t = asc2hex(work + 7);
                        if (t != (ss & 0xff))
                            emit ErrorRpt(ferror = C_Err_RecordSum, C_ErrMsg_RecordSum);
                        else
                            {
                                cnt = asc2hex(work + 5);
                                if (cnt == 0) 
                                    HaveEndRecord = true;
                                else
                                {
                                    aaaa = asc2int(work + 1);
                                    if (WordAddressing)
                                        aaaa *= 2;
                                    else
                                    {
                                        if ((lastcnt > 0) && (aaaa == lastaddr + lastcnt / 2))
                                        {
                                            WordAddressing = true;
                                            src.seek(0); // do the whole file again with WordAddressing.  
                                            continue;
                                        }
                                        lastcnt = cnt;
                                        lastaddr = aaaa;
                                    }
                                    ss = get_rec(work + 9, cnt, aaaa, 1, dst);
                                    if ((ss & 0xff) != asc2hex(work + cnt * 2 + 9))
                                        emit ErrorRpt(ferror = C_Err_RecordSum, C_ErrMsg_RecordSum);
                                }
                            }
                    }
                    else
                        if (!HaveEndRecord)
                            emit ErrorRpt(ferror = C_Err_FormatError, C_ErrMsg_FormatError);
                }
                UpdateProgress();
            } while (!feof && (ferror == 0) && !userabort);
            if ((ferror == 0) && !userabort) {
                FilePos = FileSize;
                UpdateProgress();
            }
            src.close();
        }
        else
            emit ErrorRpt(ferror = C_Err_FileOpen, C_ErrMsg_FileOpen);
    return ferror;
}
