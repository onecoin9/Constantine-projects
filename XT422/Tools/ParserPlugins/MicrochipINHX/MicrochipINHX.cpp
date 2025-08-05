#include "MicrochipINHX.h"
#include <QFile>

// "INHX16(16-bit word)": Intellec 16-Bit HexParser Format
// This format is particularly useful to send PIC16C5x Series Object code to
// Microchip's proprietary "PICPRO" EPROM programmer.
MicrochipINHX::MicrochipINHX()
{
    isMicrochipINHXButSumError = 0;
    line.address = 0;
    line.tag = 0;
    line.cnt = 0;
    for (int i = 0; i < C_LineDataSize; i++)
        line.data[i] = 0;
}

char* MicrochipINHX::getVersion() 
{ 
    return "1.0.0.0"; 
}

char* MicrochipINHX::getFormatName() 
{ 
    return C_MicrochipINHX; 
}

bool MicrochipINHX::ConfirmFormat(QString& filename)
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
        bool ret = isMicrochipINHXRecord(work + step, RecordLength());

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
// Word data:   Byte Count*4 hex digit
// Checksum:    2 hex digit, 2's C sum from Byte Count
// Adjust the word address and byte count after the record is correct
bool MicrochipINHX::isMicrochipINHXRecord(uint8_t* rec, int len)
{
    bool Result = false;
    if (*rec++ == ':')
        if (isTextHex(rec, 8)) // check area of byte count, address, record type
        {
            line.cnt = asc2hex(rec); // count the checksum from Byte Count
            if ((line.cnt != 0) &&
                (len >= (line.cnt * 4 + 11)) && 
                isTextHex(rec, line.cnt * 4 + 10)) // check the length of this word type format
                if (SumHexText(rec, line.cnt * 2 + 5) == 0) // 2's C false is pass
                {
                    rec += 2;
                    line.address = str2long(rec, 4) * 2; rec += 4;
                    line.tag = asc2hex(rec); rec += 2;
                    line.cnt *= 2; // Word Data
                    for (int i = 0; i < line.cnt; i++)
                    {
                        line.data[i] = asc2hex(rec);
                        rec += 2;
                    }
                    Result = true;
                }
                else
                    if (++isMicrochipINHXButSumError > 2) // format correct but checksum error
                        Result = true;
        }
    return Result;
}

/// <summary>
/// Supports "INHX16(16-bit word)": Intellec 16-Bit HexParser Format.
/// This format is particularly useful to send PIC16C5x Series Object code to
/// Microchip's proprietary "PICPRO" EPROM programmer.
/// </summary>
/// <param name="srcfn"> soure data filename </param>
/// <param name="dst"> destination QFile pointer </param>
/// <returns>  </returns>
int MicrochipINHX::TransferFile(QString& srcfn, QIODevice* dst)
{
    m_Dst = dst;
    bool WordAddressing = false;
    qint64 ext2addr = 0, ext4addr = 0, aaaa = 0, lastaddr = 0;
    int tt, cnt, i, lastcnt = -1;
    uint32_t ss;
    bool HaveEndRecord = false, chktag = false;

    InitControl();
    QFile src(srcfn);
    FileSize = src.size();
    if (FileSize <= 0)
        emit ErrorRpt(ferror = C_Err_FileZero, C_ErrMsg_FileZero);
    else
    if (src.open(QIODevice::ReadOnly))
    {
        do {
            if (SkipNullHeadToTag(&src, ':', chktag) && myfgets(&src) && (work[0] == C_Colon))
            {
                chktag = true;
                i = removeBackNul(work); // trim tailed white space and control key, return length
                if (isTextHex(work + 1, i - 1))
                {
                    ss = head_sum(work + 1, 0); // checksum from BB to TT
                    cnt = asc2hex(work + 1);
                    if (cnt * 4 + 11 > i) // may be INHX16 or Word Addressing
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
                                src.seek(0); 
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
                                if (cnt != 1)
                                    emit ErrorRpt(ferror = C_Err_FormatError, C_ErrMsg_FormatError);
                                else
                                {
                                    ext2addr = asc2int(work + 9); // USBA
                                    ext2addr <<= 4;
                                    if (WordAddressing)
                                        ext2addr *= 2;
                                }
                            else
                                if (tt == 4)
                                    if (cnt != 1)
                                        emit ErrorRpt(ferror = C_Err_FormatError, C_ErrMsg_FormatError);
                                    else
                                    {
                                        ext4addr = asc2int(work + 9); // ULBA
                                        ext4addr <<= 16;
                                        if (WordAddressing)
                                            ext4addr *= 2;
                                    }
                                else 
                                    if ((tt == 0) && (cnt > 0))
                                    {
                                        ss += get_rec(work + 9, cnt*2, (ext2addr + ext4addr + aaaa)*2, 0, dst);
                                        if ((ss & 0xff) != asc2hex(work + cnt * 4 + 9))
                                            emit ErrorRpt(ferror = C_Err_RecordSum, C_ErrMsg_RecordSum);
                                    }
                    }
                }
            }
            UpdateProgress();
        } while (!feof && (ferror == 0) && !userabort);
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
