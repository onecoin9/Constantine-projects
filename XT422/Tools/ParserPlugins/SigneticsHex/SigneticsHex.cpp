#include "SigneticsHex.h"
#include <QFile>

SigneticsHex::SigneticsHex()
{
    isSigneticsHexButSumError = 0;
    line.address = 0;
    line.tag = 0;
    line.cnt = 0;
    for (int i = 0; i < C_LineDataSize; i++)
        line.data[i] = 0;
}

char* SigneticsHex::getVersion() 
{ 
    return "1.0.0.0"; 
}

char* SigneticsHex::getFormatName() 
{ 
    return C_SigneticsHex;
}

bool SigneticsHex::ConfirmFormat(QString& filename)
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
        bool ret = isSigneticsHexRecord(work + step, RecordLength());

        fptr.close();
        return ret;
    }
    else
        return false; 
}

// For the Signetic Absolute Object Format
uint32_t SigneticsHex::XorRotateSum(uint8_t* str, int len)
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

// ':hhhhhh....'
// Address: 4 hex digit, 0..ffff
// Byte Count: 2 hex digit
// Address Checksum: 2 hex digit sum, each byte xor with previous byte and rotate left
// data list: 2 * Byte Count hex digit
// Data Checksum: each byte xor with previous byte and rotate left
//
// Note, No tag, terminated by Byte Count=0
bool SigneticsHex::isSigneticsHexRecord(uint8_t* rec, int len)
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

/// <summary>
/// Signetics Absolute Object Format
/// SS: every bytes is exclusive ORed with the previous byte,
/// then rotated left one byte. see head_sum, put_rec
/// </summary>
/// <param name="srcfn"> soure data filename </param>
/// <param name="dst"> destination QFile pointer </param>
/// <returns>  </returns>
int SigneticsHex::TransferFile(QString& srcfn, QIODevice* dst)
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

                if (SkipNullHeadToTag(&src, ':', chktag) && myfgets(&src))
                {
                    chktag = true;
                    t = removeBackNul(work); // trim tailed white space and control key, return length
                    if (isTextHex(work + 1, t - 1))
                    {
                        ss = head_sum(work + 1, 3); // checksum from BB to TT
                        t = asc2hex(work + 7);
                        if (t != (ss & 0xff))
                            emit ErrorRpt(ferror = C_Err_RecordSum, C_ErrMsg_RecordSum);
                        else
                            {
                                cnt = asc2hex(work + 5);
                                if (cnt == 0) 
                                {
                                    HaveEndRecord = true;
                                }
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
                                    ss = get_rec(work + 9, cnt, aaaa, 3, dst);
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
