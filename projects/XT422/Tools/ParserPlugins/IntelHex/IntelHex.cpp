#include "IntelHex.h"
#include <QFile>

// Intel Intellec 8/MDS Format/Intel MSC-86 Hexadecimal OBJ
IntelHex::IntelHex()
{
    isIntelHexButSumError = 0;
    line.address = 0;
    line.tag = 0;
    line.cnt = 0;
    for (int i = 0; i < C_LineDataSize; i++)
        line.data[i] = 0;
}

char* IntelHex::getVersion() 
{ 
    return "1.0.0.0"; 
}

char* IntelHex::getFormatName() 
{ 
    return C_IntelHex; 
}

bool IntelHex::ConfirmFormat(QString& filename)
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
        bool ret = isIntelHexRecord(work + step, RecordLength());

        fptr.close();
        return ret;
    }
    else
        return false; 
}

// ':hhhhhhhh...'
// Byte Count:  2 hex digit, 0..ff
// Address:     4 hex digit
// Record Type: 2 hex digit, 0: data record,
//                           1: end of file record
// Word data:   Byte Count*2 hex digit
// Checksum:    2 hex digit, 2's C sum from Byte Count
bool IntelHex::isIntelHexRecord(uint8_t* rec, int len)
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
                    if (++isIntelHexButSumError > 2) // Intel HexParser But checksum error
                        Result = true; // ???
        }
    return Result;
}

/// <summary>
/// Supports Intel Intellec 8/MDS Format/Intel MSC-86 Hexadecimal OBJ.
/// Auto Word Addressing
/// </summary>
/// <param name="srcfn"> soure data filename </param>
/// <param name="dst"> destination QFile pointer </param>
/// <returns>  </returns>
int IntelHex::TransferFile(QString& srcfn, QIODevice* dst)
{
    bool WordAddressing = false;
    qint64 OriPos, ext2addr = 0, ext4addr = 0, aaaa = 0, lastaddr = 0;
    int tt, cnt, i, lastcnt = -1;
    uint32_t ss;
    bool HaveEndRecord = false;

    InitControl();
    QFile src(srcfn);
    FileSize = src.size();
    m_Dst = dst;
    if (FileSize <= 0)
        emit ErrorRpt(ferror = C_Err_FileZero, C_ErrMsg_FileZero);
    else
    if (src.open(QIODevice::ReadOnly) && SkipNullHead(&src))
    {
        OriPos = src.pos();
        do {
            if (myfgets(&src) && (work[0] == C_Colon))
            {
                i = removeBackNul(work); // trim tailed white space and control key, return length
                if (isTextHex(work + 1, i - 1))
                {
                    ss = head_sum(work + 1, 0); // checksum from BB to TT
                    cnt = asc2hex(work + 1);
                    if (cnt * 2 + 14 < i) // may be INHX16 or Word Addressing
                        emit ErrorRpt(ferror = C_Err_FormatError, C_ErrMsg_FormatError);
                    else 
                    {
                        aaaa = asc2int(work + 3);
                        if (WordAddressing)
                            aaaa *= 2;
                        else
                        {
                            if ((lastcnt > 0) && (aaaa == lastaddr + lastcnt / 2))
                            {
                                WordAddressing = true;
                                ext2addr = 0;
                                ext4addr = 0;
                                src.seek(OriPos); // back to original position 
                                continue; // goto do {}
                            }
                            lastcnt = cnt;
                            lastaddr = aaaa;
                        }
                        tt = asc2hex(work + 7); // get TT, Record Type
                        if ((tt == 1) && (cnt == 0))
                            HaveEndRecord = true; // keep reading line, some hex have more data after End Record
                        else
                            if (tt == 2)
                                if (cnt != 2)
                                    emit ErrorRpt(ferror = C_Err_FormatError, C_ErrMsg_FormatError);
                                else
                                {
                                    ext2addr = asc2int(work + 9); // USBA
                                    ext2addr <<= 4;
                                    if (WordAddressing)
                                        ext2addr *= 2;
                                }
                            else
                                if (tt==4)
                                    if (cnt != 2)
                                        emit ErrorRpt(ferror = C_Err_FormatError, C_ErrMsg_FormatError);
                                    else
                                    {
                                        ext4addr = asc2int(work + 9); // ULBA
                                        ext4addr <<= 16;
                                        if (WordAddressing)
                                            ext4addr *= 2;
                                    }
                                else // 2017 0508 add 0x20 HexParser Data Type for Samsung
                                    if (((tt == 0) || (tt == 0x20)) && (cnt > 0))
                                    {
                                        ss += get_rec(work + 9, cnt, ext2addr + ext4addr + aaaa, 0, dst);
                                        if ((ss & 0xff) != asc2hex(work + cnt * 2 + 9))
                                            emit ErrorRpt(ferror = C_Err_RecordSum, C_ErrMsg_RecordSum);
                                    }
                    }
                }
            }
            UpdateProgress();
        } while (!feof && (ferror==0) && !userabort);
        src.close();            
        if ((ferror == 0) && !userabort)
        {
            FilePos = FileSize;
            UpdateProgress();
        }
    }
    else
        emit ErrorRpt(ferror = C_Err_FileOpen, C_ErrMsg_FileOpen);
    return ferror;
}
