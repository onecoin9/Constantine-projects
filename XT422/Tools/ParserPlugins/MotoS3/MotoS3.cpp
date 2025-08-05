#include "MotoS3.h"
#include <QFile>

// Motorola Exorciser Format / Motolola Exermax Format
MotoS3::MotoS3()
{
    isMotorolaSButSumError = 0;
    line.address = 0;
    line.tag = 0;
    line.cnt = 0;
    for (int i = 0; i < C_LineDataSize; i++)
        line.data[i] = 0;
}

char* MotoS3::getVersion() 
{ 
    return "1.0.0.0"; 
}

char* MotoS3::getFormatName() 
{ 
    return C_MotorolaS; 
}

bool MotoS3::ConfirmFormat(QString& filename)
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
        bool ret = isMotorolaSRecord(work + step, RecordLength());

        fptr.close();
        return ret;
    }
    else
        return false; 
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
bool MotoS3::isMotorolaSRecord(uint8_t* rec, int len)
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

/// <summary>
/// Supports Intel Intellec 8/MDS Format/Intel MSC-86 Hexadecimal OBJ.
/// Auto Word Addressing
/// </summary>
/// <param name="srcfn"> soure data filename </param>
/// <param name="dst"> destination QFile pointer </param>
/// <returns>  </returns>
int MotoS3::TransferFile(QString& srcfn, QIODevice* dst)
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

                if (SkipNullHeadToTag(&src, 'S', chktag) && myfgets(&src))
                {
                    chktag = true;
                    t = removeBackNul(work); // trim tailed white space and control key, return length
                    if (isTextHex(work + 1, t - 1))
                    {
                        ss = head_sum(work + 1, 2); // checksum from BB to TT
                        t = aschex(work[1]);
                        if ((t <= 9) && (t >= 7))
                            HaveEndRecord = true;
                        else
                            if ((t != 0) && (t <= 4))
                            {
                                cnt = asc2hex(work + 2) - t - 2;
                                if (cnt > 0)  // data record but length is not zero.
                                {
                                    aaaa = str2long(work + 4, (t + 1) * 2); // S1, S2, S3
                                    if (WordAddressing)
                                        aaaa *= 2;
                                    else
                                    {
                                        if ((lastcnt > 0) && (aaaa == lastaddr + lastcnt / 2))
                                        {
                                            WordAddressing = true;
                                            src.seek(0); // do the whole file again.  
                                            continue;
                                        }
                                        lastcnt = cnt;
                                        lastaddr = aaaa;
                                    }
                                    ss += get_rec(work + t * 2 + 6, cnt, aaaa, 2, dst);
                                    ss--; // 1's Complement=2's Complement-1
                                    if ((ss & 0xff) != asc2hex(work + cnt * 2 + t * 2 + 6))
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
