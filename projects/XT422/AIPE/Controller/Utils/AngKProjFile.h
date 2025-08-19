#pragma once

#include <QObject>
#include <QFile>
#include <QTextStream>
#include <fstream>
#include <QDebug>
#define PRJ_FILE_V100		(0x100)
#define PRJ_FILE_V110		(0x110)
#define PRJ_FILE_VERSION PRJ_FILE_V110

#define PASSWD_LENGTH	6

enum FORMATFILE_RET {
	E_FMTFILE_OK = 0,
	E_FMTFILE_ERRER = -1,
	E_TAG_NOTFOND = -2,
	E_TAG_UNEOUGH = -3,
	E_TAG_CHECKSUM = -4,
	E_TAG_EXIST	= -5,
	E_PWD_ERROR = -6,
	E_PWD_CANCEL = -7
};

const unsigned char GbyMagic[4] = { 'P', 'R', 'J', 0x19 };

const unsigned char GbyTplMagic[4] = { 'T','P','L',0x19 };

#define PRJTYPE_PRJ  (0)	//对应使用什么魔数
#define PRJTYPE_TPL  (1)
#define MAX_TAGCOUNT (30)

//四字节对齐
typedef struct FILEHEADER {
	uchar	byMagic[4];
	ulong	dwCount;
	ushort  wEncryp[2];
	uchar	byDevType[16];
	uchar	byDevSn[16];
	ulong	dwExecCount;
	ulong	dwExecCurCnt;
	uchar	byPasswd[32];
	uint	uiVersion;
	uint	uiFlags;
	ulong	dwCheckSum;

	friend QDebug& operator<<(QDebug out, const FILEHEADER& info)
	{
		out <<(char*)info.byMagic << info.dwCount << (char*)info.wEncryp << (char*)info.byDevType << (char*)info.byDevSn << info.dwExecCount << info.dwExecCurCnt << (char*)info.byPasswd << info.uiVersion << info.uiFlags << info.dwCheckSum;
		return out;
	}

}PFILEHEADER_t;

typedef struct TAG {
	ulong	dwOffset;		//文件内偏移  4个字节
	ulong	dwLength;		//资源长度	  4个字节
	ushort	wVersion;		//当前版本	  2个字节
	uchar	byType;			//数据段类型, 1,bin, 2, so, 3,cfg
	ulong	dwCRC;			//保留区
	char	chName[12];	    //filename, \0 结尾
	uchar	TagFlag;		//Tag是否一定要被处理，如果bit0为表示一定要被处理，否则可做兼容处理
	char    dwReserved[4];	//保留
}PTAG_t;

#define TAG_SIZE sizeof(PTAG_t)	//不包括uiOffset

typedef QList<PTAG_t> PTAGLIST;

class AngKProjFile : public QObject
{
	Q_OBJECT

public:
	AngKProjFile(QObject *parent = nullptr, int projType = PRJTYPE_PRJ);
	~AngKProjFile();

	static void PrintMessage(QWidget* parentWidget, int nResultCode, const QString& title);
	//创建工程文件
	int CreateProjFile(QString strFilePath, uint dwTagCnt, uint uiMaxCnt = 0, uint uiCurCnt = 0,
		const QString strDevType = "", const QString strDevSn = "", const QString strPasswd = "", uint uiFlags = 0);

	//从文件加载数据到对象
	int LoadFile(QString strFilePath);

	//添加到tag到工程文件
	int AddTag(uint uiTag, uchar* lpBuff, uint uiLen, bool NeedBeHandle = 0, uint uiVersion = 1, char* pcName = NULL);

	//获取Tag数据
	//int GetTagData(uint uiTag, CSerial& lSerial);
	int GetTagData(uint uiTag, std::string& dataStream);

	//添加到tag到工程文件
	int AddTagHeader(uint uiTag, uint uiLen, uint uiVersion = 1, char* pcName = NULL);
	int UpdateExistTag(uint uiTag, uint uiLen, uint uiVersion, char* pcName, uint Checksum);

	//获取Tag头信息
	int GetTagHeader(uint uiTag, PTAG_t& lTag);

	int ReadTagBuf(uchar* lpBuff, uint uiLen);
	int WriteTagBuf(uchar* lpBuff, uint uiLen);

	//关闭文件
	int CloseFile();

	//获取文件数据
	uint ReadFile(uchar* lpBuff, uint uiCount);

	//获取文件长度
	uint GetLength();

	int SeekTagBuf(PTAG_t& BufTag, int64_t lOff, uint nFrom);

	QFile& getFileStream() { return m_projFile; }

	QString getFilePath() { return m_projFile.fileName(); }

	//数据进行base64加解密
	void encodeBase64_Json(std::string& baseJson);

	void dencodeBase64_Json(std::string& baseJson);

	//将bin目录档案相关压缩文件写入工程目录中
	void AddBinFileTag(uint uiTag, QString strfilePath);

	void DepressBinFile(std::string& binZip, QStringList& pathList);

	bool VerifyPasswd(const char* strPasswd);

	bool HasPasswd();

	bool TagHandleCheck() { return m_MustHandleTagList.isEmpty(); }
private:
	QFile			m_projFile;
	//std::fstream	m_projFile;
	PFILEHEADER_t	m_FileHeader;
	PTAGLIST		m_TagList;
	bool			m_bDirty;
	bool			m_LoadOK;
	int				m_PrjType;

	QList<uint>		m_MustHandleTagList;
};
