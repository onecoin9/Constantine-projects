#include "ExtendedTekHex.h"
#include <QFile>

// Intel Intellec 8/MDS Format/Intel MSC-86 Hexadecimal OBJ
ExtendedTekHex::ExtendedTekHex()
{
    isExtendedTekHexButSumError = 0;

    line.address = 0;
    line.tag = 0;
    line.cnt = 0;
    for (int i = 0; i < C_LineDataSize; i++)
        line.data[i] = 0;
}

char* ExtendedTekHex::getVersion() 
{ 
    return "1.0.0.0"; 
}

char* ExtendedTekHex::getFormatName() 
{ 
    return C_ExtendedTekHex;
}

bool ExtendedTekHex::ConfirmFormat(QString& filename)
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
        bool ret = isExtendedTekHexRecord(work + step, RecordLength());

        fptr.close();
        return ret;
    }
    else
        return false; 
}

// Character   Value (Decimal)
// '0'..'9'    0..9
// 'A'..'Z'    10..35
// '$'         36
// '%'         37
// '_'         39    (underscroe)
// 'a'..'z'    40..65
uint32_t ExtendedTekHex::SumExtTek(uint8_t* str, int len)
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
bool ExtendedTekHex::isExtendedTekHexRecord(uint8_t* rec, int len)
{
   bool Result = false;
    int i;
    // check the ASCII code of Block Length, Block Type, Checksum.
    if (*rec++ == '%')
        if (isTextHex(rec, 6))
        {
            uint8_t* str = rec;
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
                    if (++isExtendedTekHexButSumError > 2)
                        Result = true;
        }
    return Result;}

/// <summary>
/// Extended Tektronix HexParser Format
/// </summary>
/// <param name="srcfn"> soure data filename </param>
/// <param name="dst"> destination QFile pointer </param>
/// <returns>  </returns>
int ExtendedTekHex::TransferFile(QString& srcfn, QIODevice* dst)
{
    qint64 aaaa = 0, lastaddr = 0;
    int t, cnt, lastcnt = -1;
    uint32_t ss, j;
    bool HaveEndRecord = false, chktag = false, WordAddressing = false;
    
    InitControl();
    QFile src(srcfn);
    FileSize = src.size();
    m_Dst = dst;
    if (FileSize <= 0)
        emit ErrorRpt(ferror = C_Err_FileZero, C_ErrMsg_FileZero);
    else
        if (src.open(QIODevice::ReadOnly))
        {
            chktag = false;
            do {
                if (SkipNullHeadToTag(&src, '%', chktag) && myfgets(&src))
                {
                    chktag = true;
                    t = removeBackNul(work); // trim tailed white space and control key, return length
                    if (isTextHex(work + 1, t - 1))
                    {
                        ss = head_sum(work + 1, 4); // checksum from BB to TT
                        t = aschex(work[3]);
                        if (t==8)
                            HaveEndRecord = true;
                        else // if (tt==3) continue;
                            if (t==6)
                            {
                                j = aschex(work[6]);
                                ss += j;
                                for (int i = 0, aaaa = 0; i < j; i++)
                                { 
                                    aaaa = (16*aaaa) + aschex(work[7+i]);
                                    ss += aschex(work[7+i]);
                                } 
                                if (WordAddressing)
                                    aaaa *= 2;
                                else
                                    if ((lastcnt > 0) && (aaaa == lastaddr + lastcnt / 2))
                                    {
                                        WordAddressing = true;
                                        src.seek(0); // do the whole file again with WordAddressing.  
                                        continue;
                                    }
                                lastaddr = aaaa;                                   
                                cnt = (asc2hex(work + 1) - j - 6) / 2;
                                lastcnt = cnt;
                                ss += get_rec(work+j+7, cnt, aaaa, 1, dst);
                                if ((ss & 0xff) != asc2hex(work + 4))
                                    emit ErrorRpt(ferror = C_Err_RecordSum, C_ErrMsg_RecordSum);
                            }
                    }
                }
              //else
              //    if (!HaveEndRecord)
              //        emit ErrorRpt(ferror = C_Err_FormatError, C_ErrMsg_FormatError);
              //    else
              //        feof = true;
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
