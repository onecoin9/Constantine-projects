#ifndef _IDATAWRITER_H_
#define _IDATAWRITER_H_

#include "ACError.h"
#include "ICrcCheck.h"
#include <QFile>

/// <summary>
/// 数据写抽象类
/// </summary>
class IDataWriter
{
public:
    IDataWriter() 
        :m_CurDataSize(0)
        , m_MaxDataSize(0)
        , m_pCrcCheck(NULL)
    {};
    /// <summary>
    /// 设置最大期望写入数据字节数
    /// </summary>
    /// <param name="MaxDataSize"></param>
    void SetMaxDataSize(qint64 MaxDataSize) 
    {
        m_MaxDataSize = MaxDataSize;
    };
    qint64 GetCurDataSize()
    {
        return m_CurDataSize;
    }
    void AttachICrcCheck(ICrcCheck* pCrcCheck)
    {
        m_pCrcCheck = pCrcCheck;
    }
    /// <summary>
    /// 写入数据
    /// </summary>
    /// <param name="pData">实际数据</param>
    /// <param name="Size">外部实际会给出Writer实际期望字节数的数据，WriteData函数内部可根据最大接收字节数进行截断</param>
    /// <returns>大于等于0表示写入的字节数，小于0表示失败</returns>
    virtual qint64 WriteData(char* pData, qint64 Size) = 0;

protected:
    qint64 m_MaxDataSize;       //最大接收多少个字节
    qint64 m_CurDataSize;       //当前接收多少个字节
    ICrcCheck* m_pCrcCheck;
};

/// <summary>
/// 文件写
/// </summary>
class CFileWriter : public IDataWriter
{
public:
    CFileWriter()
        :m_pFile(NULL)
        , m_bDataWritetoFile(true)
    {
    };
    void SetQFile(QFile* pFile) {
        m_pFile = pFile;
    }
    qint64 WriteData(char* pData, qint64 Size)
    {
		qint64 BytesWrite;
		qint64 BytesNeedWrite = m_MaxDataSize - m_CurDataSize;
		if (BytesNeedWrite > Size) {
			BytesNeedWrite = Size;
		}
		//实际写入的字节数要根据需要进行调整
		if (m_pCrcCheck) {///如果有挂了校验值指针，则进行校验值计算
			m_pCrcCheck->CalcSubRoutine((uint8_t*)pData, BytesNeedWrite);
		}

		if (m_pFile) {//如果没有实际关联到某个文件的写，那么表示不需要写磁盘
			BytesWrite = m_pFile->write((char*)pData, BytesNeedWrite);
		}
		else {
			BytesWrite = BytesNeedWrite;
		}
		if (BytesWrite >= 0) {
			m_CurDataSize += BytesWrite;
		}
		return BytesWrite;
    }

    void SetFileSeek(uint64_t _fileOffset) {
        m_pFile->seek(_fileOffset);
    }

private:
    QFile* m_pFile;
    bool m_bDataWritetoFile; ///<是否将数据保存到文件中？

};

class CByteArrayWriter : public IDataWriter
{
public:
    CByteArrayWriter():
        m_pByteArray(NULL)
    {
    };
    void SetByteArray(QByteArray* pByteArray) {
        m_pByteArray = pByteArray;
    }
    qint64 WriteData(char* pData, qint64 Size)
    {
        if (m_pByteArray) {
            qint64 BytesWrite;
            qint64 BytesNeedWrite = m_MaxDataSize - m_CurDataSize;
            if (BytesNeedWrite > Size) {
                BytesNeedWrite = Size;
            }

            //实际写入的字节数要根据需要进行调整
            if (m_pCrcCheck) {///如果有挂了校验值指针，则进行校验值计算
                m_pCrcCheck->CalcSubRoutine((uint8_t*)pData, BytesNeedWrite);
            }
            m_pByteArray->append(pData, Size);
            BytesWrite = BytesNeedWrite;
            return BytesWrite;
        }
        else {
            return ERR_DataPtrNull;
        }
    }

private:
    QByteArray* m_pByteArray;
};


#endif 