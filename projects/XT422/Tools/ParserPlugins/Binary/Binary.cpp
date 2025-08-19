#include "Binary.h"
#include "../parser_global.h"

Binary::Binary()
{
    isBinaryButSumError = 0;
    line.address = 0;
    line.tag = 0;
    line.cnt = 0;
    for (int i = 0; i < C_LineDataSize; i++)
        line.data[i] = 0;
}

char* Binary::getVersion()
{
    return "1.0.0.0";
}

char* Binary::getFormatName()
{
    return C_Binary;
}

bool Binary::ConfirmFormat(QString& filename)
{
    QFileInfo file(filename);
    if (!file.suffix().compare("bin"))
        return true;
    return false;
}

int Binary::TransferFile(QString& srcfn, QIODevice* dst)
{
    m_Dst = dst;
    qint64 OriPos, ext2addr = 0, ext4addr = 0, aaaa = 0, lastaddr = 0;

	QFile src(srcfn);
	InitControl();
	FileSize = src.size();
	if (FileSize <= 0)
		emit ErrorRpt(ferror = C_Err_FileZero, C_ErrMsg_FileZero);
	else {
		if (src.open(QIODevice::ReadOnly))
		{
            OriPos = src.pos();

            int readlen;
            do {
                if (myRead(&src, readlen)) {
                    dst->write((char*)work, readlen);
                }
                UpdateProgress();
            } while (!feof && (ferror == 0) && !userabort);
            if ((ferror == 0) && !userabort) {
                FilePos = FileSize;
                UpdateProgress();
            }
            src.close();
		}
        else {
            emit ErrorRpt(ferror = C_Err_FileOpen, C_ErrMsg_FileOpen);
        }
	}
    return ferror;
}