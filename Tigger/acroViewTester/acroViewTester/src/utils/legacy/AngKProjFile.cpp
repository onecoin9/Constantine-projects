#include "AngKProjFile.h"
//#include "AngkLogger.h"
#include "Tag_Proj.h"
#include "qzipreader_p.h"
#include "qzipwriter_p.h"
#include "AngKPathResolve.h"
#include "ACMessageBox.h"
#include <QBuffer>
#include <QDataStream>
#include <QDebug>
extern "C" int sha256_compute_mac256(uchar * MT, int length, uchar * MAC);
/***
计算包含密码的MAC值，
passwd：6字节的密码首地址，ASCII值，
pmac:32个字节存放MAC的首地址
****/
extern "C" int Get_MAC(const char* passwd, uchar * pmac)
{
	uchar MT[64];
	int i = 0;
	memset(MT, 0xAA, 64);
	for (i = 0; i < 6; i++)
		MT[i] = passwd[i];
	for (i = 6; i < 12; i++)
		MT[i] = ~passwd[i - 6];
	sha256_compute_mac256(MT, 55, pmac);

	return 0;
}

AngKProjFile::AngKProjFile(QObject *parent, int projType)
	: QObject(parent)
	, m_PrjType(projType)
	, m_bDirty(false)
	, m_LoadOK(false)
{
	m_FileHeader = PFILEHEADER_t();

	if (projType == PRJTYPE_TPL) {
		memcpy(m_FileHeader.byMagic, GbyTplMagic, sizeof(GbyTplMagic));
	}
	else {
		memcpy(m_FileHeader.byMagic, GbyMagic, sizeof(GbyMagic));
	}
	m_FileHeader.dwCount = 0;
}

AngKProjFile::~AngKProjFile()
{
	CloseFile();
}


void AngKProjFile::PrintMessage(QWidget* parentWidget, int nResultCode, const QString& title)
{
	switch (FORMATFILE_RET(nResultCode))
	{
	case E_FMTFILE_OK:
		break;
	case E_TAG_NOTFOND:
	case E_TAG_UNEOUGH:
	case E_TAG_EXIST:
		ACMessageBox::showError(parentWidget, title, tr("Import project failed."));
		//ALOG_ERROR("Import project failed.", "CU", "--");
		break;
	case E_FMTFILE_ERRER:
		ACMessageBox::showError(parentWidget, title, tr("Load project file error, please check if the file is valid."));
		//ALOG_ERROR("Load project file error, please check if the file is valid.", "CU", "--");
		break;
	case E_TAG_CHECKSUM:
		ACMessageBox::showError(parentWidget, title, tr("Check the task file failed, please check if the file is valid."));
		//ALOG_ERROR("Check the task file failed, please check if the file is valid.", "CU", "--");
		break;
	case E_PWD_ERROR:
		ACMessageBox::showError(parentWidget, title, tr("Password error."));
		//ALOG_ERROR("Password error.", "CU", "--");
		break;
	case E_PWD_CANCEL:
		ACMessageBox::showError(parentWidget, title, tr("Password not entered."));
		//ALOG_ERROR("Password not entered.", "CU", "--");
		break;
	default:
		break;
	}
}

int AngKProjFile::CreateProjFile(QString strFilePath, uint dwTagCnt, uint uiMaxCnt, uint uiCurCnt, const QString strDevType, const QString strDevSn, const QString strPasswd, uint uiFlags)
{
	PTAG_t pTag{};
	m_projFile.setFileName(strFilePath);
	if (!m_projFile.open(QIODevice::ReadWrite | QIODevice::Truncate))
	{
		ALOG_FATAL("Create project file %s failed, error msg : %s.", "CU", "--", strFilePath.toStdString().c_str(), m_projFile.errorString().toStdString().c_str());
		return E_FMTFILE_ERRER;
	}

	m_FileHeader.uiVersion = PRJ_FILE_VERSION;
	m_FileHeader.dwCount = dwTagCnt;
	m_FileHeader.dwExecCount = uiMaxCnt;
	m_FileHeader.dwExecCurCnt = uiCurCnt;
	m_FileHeader.uiFlags = uiFlags;

	if (!strDevType.isEmpty()) {
		strcpy((char*)m_FileHeader.byDevType, strDevType.toStdString().c_str());
	}

	if (!strDevSn.isEmpty()) {
		strcpy((char*)m_FileHeader.byDevSn, strDevSn.toStdString().c_str());
	}

	if (!strPasswd.isEmpty() && strPasswd.length() == PASSWD_LENGTH) {
		Get_MAC(strPasswd.toStdString().c_str(), m_FileHeader.byPasswd);
	}
	else {
		memset(m_FileHeader.byPasswd, 0, sizeof(m_FileHeader.byPasswd));
	}

	m_projFile.write((char*)&m_FileHeader, sizeof(m_FileHeader));   //todo 需要序列化

	for (size_t i = 0; i < m_FileHeader.dwCount; i++) {
		m_projFile.write((char*)&pTag, sizeof(pTag));
	}
	m_bDirty = true;

	return E_FMTFILE_OK;
}

int AngKProjFile::LoadFile(QString strFilePath)
{
	PTAG_t pTag{};
	QString strMsg;
	CloseFile();	//清空原来加载的文件

	m_projFile.setFileName(strFilePath);
	if (!m_projFile.open(QIODevice::ReadWrite))
	{
		ALOG_WARN("Open project file %s failed, error msg : %s.", "CU", "--", strFilePath.toStdString().c_str(), m_projFile.errorString().toStdString().c_str());
		return E_FMTFILE_ERRER;
	}

	if (m_projFile.read((char*)(&m_FileHeader), sizeof(m_FileHeader)) < sizeof(m_FileHeader)) {
		return E_FMTFILE_ERRER;
	}

	if (m_PrjType == PRJTYPE_PRJ) {
		int ret = memcmp(m_FileHeader.byMagic, GbyMagic, sizeof(GbyMagic));
		if (ret != 0) {
			return E_FMTFILE_ERRER;
		}
	}
	else {
		if (memcmp(m_FileHeader.byMagic, GbyTplMagic, sizeof(GbyTplMagic)) != 0) {
			return E_FMTFILE_ERRER;
		}
	}

	ulong dwChecksum = 0;
	ulong* dwpTmp = (ulong*)&m_FileHeader;
	for (size_t i = 0; i < (sizeof(m_FileHeader) / 4 - 1); i++) {
		dwChecksum += *dwpTmp;
		dwpTmp++;
	}

	dwChecksum = ~dwChecksum;
	dwChecksum = dwChecksum << 3 | (dwChecksum & 0xE0000000) >> 29;
	if (dwChecksum != m_FileHeader.dwCheckSum) {
		ALOG_WARN("The project file has been damaged!", "CU", "--");
		return E_TAG_CHECKSUM;
	}

	for (size_t i = 0; i < m_FileHeader.dwCount; i++) {
		if (m_projFile.read((char*)(&pTag), TAG_SIZE) == TAG_SIZE) {
			if (pTag.dwOffset == 0) {
				break;
			}
			m_TagList.push_back(pTag);

			if (pTag.TagFlag != 0) {
				m_MustHandleTagList.push_back(pTag.byType);
			}
		}
		else {
			return E_FMTFILE_ERRER;
		}
	}
	m_bDirty = false;

	m_LoadOK = true;

	qDebug() << m_FileHeader;
	//qDebug() << m_TagList;

	return E_FMTFILE_OK;
}

extern "C" unsigned int get_mem_crc32(char* buf, unsigned int size);

int AngKProjFile::AddTag(uint uiTag, uchar* lpBuff, uint uiLen, bool NeedBeHandle, uint uiVersion, char* pcName)
{
	if (!m_projFile.isOpen()) {
		return E_FMTFILE_ERRER;
	}

	if (m_TagList.size() >= m_FileHeader.dwCount) {
		return E_TAG_UNEOUGH;
	}

	PTAG_t tag = { 0 };

	tag.byType = uiTag;
	tag.dwLength = uiLen;
	tag.wVersion = uiVersion;
	if (NeedBeHandle) {
		tag.TagFlag |= 0x01;
	}
	if (pcName != NULL) {
		if (strlen(pcName) >= 12) {
			return E_FMTFILE_ERRER;
		}
		strncpy(tag.chName, pcName, sizeof(tag.chName) - 1);
	}
	tag.dwOffset = m_projFile.size();
	if ((tag.dwOffset % 32) != 0) {		//对齐32字节
		char byTemp[32] = { 0 };
		m_projFile.seek(m_projFile.size());
		m_projFile.write(byTemp, 32 - (tag.dwOffset % 32));
		tag.dwOffset = m_projFile.size();
	}
	tag.dwCRC = get_mem_crc32((char*)lpBuff, uiLen);

	//tag写入
	m_projFile.seek((sizeof(m_FileHeader) + sizeof(tag) * m_TagList.size()));
	m_projFile.write((char*)&tag, sizeof(tag));

	//tag内容写入
	m_projFile.seek(m_projFile.size());
	m_projFile.write((char*)lpBuff, uiLen);

	m_TagList.push_back(tag);
	m_bDirty = true;

	return E_FMTFILE_OK;
}

static int __GetTagName(int TagID, QString& strTagName)
{
	switch (TagID) {
	case TAG_CHIPSPBIT:
		strTagName = "Config";
		break;
	default:
		strTagName = QString("Tag[%1]").arg(TagID);
	}
	return 1;
}

int AngKProjFile::GetTagData(uint uiTag, std::string& dataStream)
{
	uchar* lpBuf = NULL;
	PTAG_t pTag{};
	uint Crc32;
	if (!m_projFile.isOpen()) {
		return E_FMTFILE_ERRER;
	}

	PTAGLIST::iterator it;
	for (it = m_TagList.begin(); it != m_TagList.end(); it++) {
		pTag = *it;
		if (pTag.byType == uiTag) {
			break;
		}
	}

	if (it == m_TagList.end()) {
		return E_TAG_NOTFOND;
	}

	if (pTag.TagFlag != 0) {
		m_MustHandleTagList.removeAll(uiTag);
	}

	try {
		m_projFile.seek(pTag.dwOffset);
		lpBuf = new uchar[pTag.dwLength];
		memset(lpBuf, 0, pTag.dwLength);
		if (NULL == lpBuf) {
			return E_FMTFILE_ERRER;
		}
		if (m_projFile.read((char*)lpBuf, pTag.dwLength) < pTag.dwLength) {
			delete[] lpBuf;
			return E_FMTFILE_ERRER;
		}

		if (pTag.dwCRC != (Crc32 = get_mem_crc32((char*)lpBuf, pTag.dwLength))) {
			delete[] lpBuf;

			QString strTagName;
			__GetTagName(uiTag, strTagName);
			QString strErrMsg = QString("%1 : Data Checksum Compare Error, Desired:0x%2, Really:0x%3").arg(strTagName).arg(pTag.dwCRC, 0, 8).arg(Crc32, 0, 8);
			//ALOG_FATAL(strErrMsg.toStdString().c_str());
			ALOG_FATAL("%s : Data Checksum Compare Error, Desired:0x%d, Really:0x%d.", "CU", "--", strTagName.toStdString().c_str(), pTag.dwCRC, Crc32);

			return E_TAG_CHECKSUM;
		}

		dataStream.assign((char*)lpBuf, pTag.dwLength);

		delete[] lpBuf;

	}
	catch (...) {
		delete[] lpBuf;
		return E_FMTFILE_ERRER;
	}

	return E_FMTFILE_OK;
}

int AngKProjFile::AddTagHeader(uint uiTag, uint uiLen, uint uiVersion, char* pcName)
{
	if (!m_projFile.isOpen()) {
		return E_FMTFILE_ERRER;
	}

	if (m_TagList.size() >= m_FileHeader.dwCount) {
		return E_TAG_UNEOUGH;
	}

	PTAG_t tag = { 0 };

	tag.byType = uiTag;
	tag.dwLength = uiLen;
	tag.wVersion = uiVersion;
	if (pcName != NULL) {
		strncpy(tag.chName, pcName, sizeof(tag.chName) - 1);
	}
	tag.dwOffset = m_projFile.size();
	if ((tag.dwOffset % 32) != 0) {		//对齐32字节
		uchar byTemp[32] = { 0 };
		m_projFile.seek(m_projFile.size());
		m_projFile.write((char*)byTemp, 32 - (tag.dwOffset % 32));
		tag.dwOffset = m_projFile.size();
	}

	m_projFile.seek(sizeof(m_FileHeader) + sizeof(tag) * m_TagList.size());
	m_projFile.write((char*)&tag, sizeof(tag));
	m_projFile.flush();
	m_projFile.seek(m_projFile.size());

	m_TagList.push_back(tag);
	m_bDirty = true;

	return E_FMTFILE_OK;
}

int AngKProjFile::UpdateExistTag(uint uiTag, uint uiLen, uint uiVersion, char* pcName, uint Checksum)
{
	int TagIdx = 0;
	PTAG_t TargetTag = { 0 };
	if (!m_projFile.isOpen()) {
		return E_FMTFILE_ERRER;
	}

	if (m_TagList.size() >= m_FileHeader.dwCount) {
		return E_TAG_UNEOUGH;
	}

	PTAGLIST::iterator it;
	for (it = m_TagList.begin(); it != m_TagList.end(); it++) {
		TargetTag = *it;
		if (TargetTag.byType == uiTag) {
			break;
		}
		TagIdx++;
	}
	if (it == m_TagList.end()) {
		return E_TAG_NOTFOND;
	}

	TargetTag.byType = uiTag;
	TargetTag.dwLength = uiLen;
	TargetTag.wVersion = uiVersion;
	TargetTag.dwCRC = Checksum;
	m_projFile.seek(sizeof(m_FileHeader) + sizeof(TargetTag) * TagIdx);
	ALOG_DEBUG("UpdateTAGPos:0x%08X, TAGID:%d, TagIdx:%d", "CU", "--", m_projFile.pos(), TargetTag.byType, TagIdx);
	m_projFile.write((char*)&TargetTag, sizeof(TargetTag));
	m_projFile.seek(m_projFile.size());

	return E_FMTFILE_OK;
}

int AngKProjFile::GetTagHeader(uint uiTag, PTAG_t& lTag)
{
	if (!m_projFile.isOpen()) {
		return E_FMTFILE_ERRER;
	}

	PTAGLIST::iterator it;
	for (it = m_TagList.begin(); it != m_TagList.end(); it++) {
		lTag = *it;
		if (lTag.byType == uiTag) {
			m_projFile.seek(lTag.dwOffset);
			return E_FMTFILE_OK;
		}
	}

	return E_FMTFILE_ERRER;
}

int AngKProjFile::ReadTagBuf(uchar* lpBuff, uint uiLen)
{
	if (!m_projFile.isOpen()) {
		return E_FMTFILE_ERRER;
	}

	return m_projFile.read((char*)lpBuff, uiLen);
}

int AngKProjFile::WriteTagBuf(uchar* lpBuff, uint uiLen)
{
	if (!m_projFile.isOpen()) {
		return E_FMTFILE_ERRER;
	}

	qint64 ret = m_projFile.write((char*)lpBuff, uiLen);
	m_projFile.flush();
	m_bDirty = true;

	return E_FMTFILE_OK;
}

int AngKProjFile::CloseFile()
{
	if (m_projFile.isOpen())
	{
		if (m_bDirty) {
			m_FileHeader.dwCount = m_TagList.size();
			ulong dwChecksum = 0;
			ulong* dwpTmp = (ulong*)&m_FileHeader;
			for (size_t i = 0; i < (sizeof(m_FileHeader) / 4 - 1); i++) {
				dwChecksum += *dwpTmp;
				dwpTmp++;
			}
			dwChecksum = ~dwChecksum;
			m_FileHeader.dwCheckSum = dwChecksum << 3 | (dwChecksum & 0xE0000000) >> 29;

			m_projFile.seek(0);
			m_projFile.write((char*)&m_FileHeader, sizeof(m_FileHeader));
		}

		m_projFile.close();
	}

	m_TagList.clear();
	m_LoadOK = false;

	return E_FMTFILE_OK;
}

uint AngKProjFile::ReadFile(uchar* lpBuff, uint uiCount)
{
	if (!m_projFile.isOpen()) {
		return E_FMTFILE_ERRER;
	}

	return m_projFile.read((char*)lpBuff, uiCount);
}

uint AngKProjFile::GetLength()
{
	if (!m_projFile.isOpen()) {
		return E_FMTFILE_ERRER;
	}

	return m_projFile.size();
}

int AngKProjFile::SeekTagBuf(PTAG_t& BufTag, int64_t lOff, uint nFrom)
{
	int64_t NewPos = BufTag.dwOffset;
	uint64_t CurPos = m_projFile.pos();
	if (nFrom == 0){
		NewPos = BufTag.dwOffset + lOff;
	}
	else if (nFrom == m_projFile.size()) {
		NewPos = BufTag.dwOffset + BufTag.dwLength + lOff;
	}
	else {
		NewPos = CurPos - BufTag.dwOffset + lOff;
	}

	if (NewPos < BufTag.dwOffset || NewPos > BufTag.dwOffset + BufTag.dwLength) {
		return -1;
	}
	else {
		m_projFile.seek(NewPos);
	}

	return 0;
}

void AngKProjFile::encodeBase64_Json(std::string& baseJson)
{
	QByteArray encodedData = QString::fromStdString(baseJson).toUtf8().toBase64();
	baseJson = QString::fromUtf8(encodedData).toStdString();
}

void AngKProjFile::dencodeBase64_Json(std::string& baseJson)
{
	QByteArray decodedData = QByteArray::fromBase64(QString::fromStdString(baseJson).toLocal8Bit());
	baseJson = QString::fromUtf8(decodedData).toStdString();
}

void AngKProjFile::AddBinFileTag(uint uiTag, QString strfilePath)
{
	QFile binFile(strfilePath);
	if (binFile.open(QFile::ReadOnly))
	{
		//size_t fileLen = binFile.size();
		//uchar* buf = new uchar[fileLen];
		//memset(buf, 0, fileLen);
		//binFile.read((char*)buf, fileLen);
		//AddTag(uiTag, buf, fileLen);
		//delete[] buf;


		QByteArray data = binFile.readAll();
		AddTag(uiTag, (uchar*)data.data(), data.size());

	}
	else
	{
		ALOG_FATAL("AddBinFileTag File %s Failed, errorMsg: %s.", "CU", "--", strfilePath.toStdString().c_str(), binFile.errorString().toStdString().c_str());
	}
}

void AngKProjFile::DepressBinFile(std::string& binZip, QStringList& pathList)
{
	QByteArray compressedByteArray(binZip.c_str(), binZip.size());

	// 创建一个 QBuffer，并设置其数据源为 QByteArray
	QBuffer buffer(&compressedByteArray);
	buffer.open(QIODevice::ReadOnly);

	QZipReader zipReader(&buffer);
	for (const auto& fileInfo : zipReader.fileInfoList())
	{
		QFile file(Utils::AngKPathResolve::localTempFolderPath() + fileInfo.filePath);
		if (file.open(QIODevice::WriteOnly))
		{
			QByteArray array = zipReader.fileData(fileInfo.filePath);
			file.write(array);
			file.close();

			pathList.push_back(Utils::AngKPathResolve::localTempFolderPath() + fileInfo.filePath);
		}
	}
	zipReader.close();
}

bool AngKProjFile::VerifyPasswd(const char* strPasswd)
{
	uchar byPasswd[32] = { 0 };
	Get_MAC(strPasswd, byPasswd);

	if (memcmp(byPasswd, m_FileHeader.byPasswd, 32) == 0) {
		return true;
	}
	return false;
}

bool AngKProjFile::HasPasswd()
{
	uchar byPasswd[32] = { 0 };
	if (memcmp(byPasswd, m_FileHeader.byPasswd, 32) == 0) {
		return false;
	}
	return true;
}
