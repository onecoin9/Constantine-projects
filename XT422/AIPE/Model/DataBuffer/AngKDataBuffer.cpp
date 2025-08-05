#include "AngKDataBuffer.h"
#include "AngKCommonTools.h"
#include "AngkLogger.h"
#include "AngKPathResolve.h"
#include <Windows.h>
#include <QBuffer>
#include <QDataStream>
#include <QDebug>
#include <QRandomGenerator>
#include <QTime>

#define SAFEDEL(_ptr) do {if(_ptr)delete _ptr; _ptr=NULL;} while(0);
#define SAFEDEL_ARRAY(_aptr)  do {if(_aptr)delete[] _aptr; _aptr=NULL;} while(0);

AngKDataBuffer::AngKDataBuffer(QObject *parent)
	: IDataBuffer(parent)
	, m_WriteAddrMin(-1)
	, m_WriteAddrMax(0)
	, m_uiMapSize(0)
	, m_Virgin(0)
{
	m_vPartitionInfos.clear();
}

AngKDataBuffer::~AngKDataBuffer()
{
	ClearPartInfos(m_vPartitionInfos);
}

int AngKDataBuffer::InitBuffer(uint64_t nSize, uchar virgin)
{
	m_WriteAddrMin = -1;
	m_WriteAddrMax = 0;
	//ClearBufInfos(m_vpBufInfos);
	ClearPartInfos(m_vPartitionInfos);
	return CreateBuffer(nSize, virgin);
}

int AngKDataBuffer::CreateBuffer(uint64_t Size, uchar Virgin)
{
	int ret = 0;

	//解析XML
	ret = ParserCommonBinXml(m_selectChipCfg.strICDataXmlPath, Size, Virgin);


	return ret;
}

void AngKDataBuffer::SetChipConfig(uint unAlgoIC, QStringList mstDrvList, QStringList devDrvList, uchar virgin)
{
	m_selectChipCfg.nAlgoIC = unAlgoIC;
	m_selectChipCfg.OrgVirgin = virgin;

	for (auto path : devDrvList) {
		if (path.contains(".xml") && path.contains("chipData", Qt::CaseInsensitive)) {
			m_selectChipCfg.strICDataXmlPath = path.toStdString();
		}
		if (path.contains(".adrv") && path.contains("Drv")) {
			m_selectChipCfg.strDeviceDriverPath = path.toStdString();
		}
		else if (path.contains("chipconfig", Qt::CaseInsensitive)) {
			m_selectChipCfg.strICConfigXmlPath = path.toStdString();
		}
		else if (path.contains(".CHK", Qt::CaseInsensitive)) {
			m_selectChipCfg.strChksumDllPath = path.toStdString();
		}
	}

	for (auto path : mstDrvList) {
		if (path.contains(".adrv") && path.contains("Mst")) {
			m_selectChipCfg.strMasterDriverPath = path.toStdString();
		}
	}
}

ADR AngKDataBuffer::BufferRead(ADR adrStart, uchar* pBuf, ADR adrLen)
{
	uint64_t adrCpSize;
	uint64_t adrCompleteSize = 0;
	uint64_t adrEnd = adrStart + adrLen - 1;
	uint64_t ReadStart, ReadEnd;
	uint64_t TotalRead = 0;
	bool isBufMapHit = FALSE;

	//int BufCnt = m_vpBufInfos.size();
	memset(pBuf, m_Virgin, (size_t)adrLen);

	for (auto partInfo : m_vPartitionInfos) {
		if (!partInfo->PartitionShow)//不显示的也没有可读的
			continue;

		int BufCnt = partInfo->vecSubView.size();
		for (int i = 0; i < BufCnt; ++i) {
			tBufInfo* pBufInfo = partInfo->vecSubView[i];
			if (pBufInfo->llBufStart > adrEnd || pBufInfo->llBufEnd < adrStart) {///不命中该映射区域
			}
			else {
				if (!pBufInfo->m_bBufferShow)//不显示的没有可读的
					continue;
				uint64_t adrLenRead;
				uint64_t offsetInMapPage;
				isBufMapHit = TRUE;
				//获取在该映射区域中需要读取的起始地址和结束地址，为双闭区间
				ReadStart = pBufInfo->llBufStart >= adrStart ? pBufInfo->llBufStart : adrStart;
				ReadEnd = pBufInfo->llBufEnd >= adrEnd ? adrEnd : pBufInfo->llBufEnd;
				adrLenRead = ReadEnd - ReadStart + 1;
				adrCompleteSize = 0;
				//将需要读取的位置与Buffer文件的交叉区域的数据拷贝到Buffer的相应位置上。
				while (adrLenRead > 0) {
					if (MapBuffer(pBufInfo, ReadStart) != 0) {//将ReadStart对齐之后所在的m_uiMapSize大小映射出来
						TotalRead = 0;
						goto __end;
					}
					offsetInMapPage = ReadStart - pBufInfo->llBufStart - pBufInfo->m_adrMapOffset;//在一个Page内的偏移肯定小于m_uiMapSize
					adrCpSize = qMin(adrLenRead, m_uiMapSize - offsetInMapPage);
					memcpy(pBuf + ReadStart - adrStart, //需要拷贝到pBuf的偏移位置
						pBufInfo->m_pBuffMapped + offsetInMapPage,//从映射空间的哪个起始位置开始拷贝
						adrCpSize);//只能先拷贝这么多，否则越限

					adrCompleteSize += adrCpSize;
					adrLenRead -= adrCpSize;
					ReadStart += adrCpSize;
				}
				TotalRead += adrCompleteSize;
			}
		}
	}
__end:
	return TotalRead;
}

ADR AngKDataBuffer::BufferWrite(ADR adrStart, uchar* pBuf, ADR adrLen)
{
	ADR Ret = 0;
	bool HitMap = false;
	uint64_t adrCpSize;
	uint64_t adrCompleteSize = 0;
	uint64_t adrEnd = adrStart + adrLen - 1;
	uint64_t WriteStart, WriteEnd;
	uint64_t TotalWrite = 0;

	for (auto partInfo : m_vPartitionInfos){
		if(!partInfo->PartitionShow)//不显示的不需要写入
			continue;

		int BufCnt = partInfo->vecSubView.size();
		for (int i = 0; i < BufCnt; ++i) {
			tBufInfo* pBufInfo = partInfo->vecSubView[i];
			if (pBufInfo->llBufStart > adrEnd || pBufInfo->llBufEnd < adrStart) {///不命中该映射区域
			}
			else {
				if (!pBufInfo->m_bBufferShow)//不显示的不需要写入
					continue;

				uint64_t adrLenWrite;
				uint64_t offsetInMapPage;
				HitMap = true;
				//获取在该映射区域中需要读取的起始地址和结束地址，为双闭区间
				WriteStart = pBufInfo->llBufStart >= adrStart ? pBufInfo->llBufStart : adrStart;
				WriteEnd = pBufInfo->llBufEnd >= adrEnd ? adrEnd : pBufInfo->llBufEnd;
				adrLenWrite = WriteEnd - WriteStart + 1;
				adrCompleteSize = 0;
				//将需要读取的位置与Buffer文件的交叉区域的数据拷贝到Buffer的相应位置上
				while (adrLenWrite > 0) {
					if (MapBuffer(pBufInfo, WriteStart) != 0) {
						TotalWrite = 0;
						goto __end;
					}
					offsetInMapPage = WriteStart - pBufInfo->llBufStart - pBufInfo->m_adrMapOffset;
					adrCpSize = qMin(adrLenWrite, m_uiMapSize - offsetInMapPage);
					memcpy(pBufInfo->m_pBuffMapped + offsetInMapPage, pBuf + WriteStart - adrStart, adrCpSize);

					adrCompleteSize += adrCpSize;
					adrLenWrite -= adrCpSize;
					WriteStart += adrCpSize;
				}
				TotalWrite += adrCompleteSize;
			}
			pBufInfo->m_writeFileSize = TotalWrite;
		}
	}
__end:

	if (TotalWrite <= 0) {
		//ShowTipsDlg();
		Ret = 0;
	}
	else {
		Ret = TotalWrite;
	}
	if (adrStart < m_WriteAddrMin) {
		m_WriteAddrMin = adrStart;
	}
	if (adrStart + adrLen > m_WriteAddrMax) {
		m_WriteAddrMax = adrStart + adrLen;
	}
	//m_bDirty = TRUE;

	return TotalWrite;
}

int AngKDataBuffer::BufferSwitchPartition(int nPartIndex)
{
	return 0;
}

int AngKDataBuffer::BufferGetCurPartition()
{
	return 0;
}

void AngKDataBuffer::BufferAddTemporaryDataRemap()
{
}

void AngKDataBuffer::BufferRemoveTemporaryDataRemap()
{
}

void AngKDataBuffer::BufferCheckPartitionExist()
{
}

ADR AngKDataBuffer::GetSize()
{
	ADR TotalSize = 0;
	int partCnt;
	int Ret = 0;

	partCnt = GetPartitionCount();
	for (int partIdx = 0; partIdx < partCnt; ++partIdx)
	{
		const tPartitionInfo* partInfo;
		partInfo = GetPartitionInfo(partIdx);

		if (!partInfo->PartitionShow)
			continue;

		int bufCnt = GetBufferCount(partIdx);
		for (int bufIdx = 0; bufIdx < bufCnt; ++bufIdx)
		{
			const tBufInfo* BufInfo = nullptr;
			BufInfo = GetBufferInfo(bufIdx, partIdx);

			if (!BufInfo->m_bBufferShow)
				continue;

			if (BufInfo == nullptr) {
				break;
			}
			else {
				if (BufInfo->llBufEnd + 1 > TotalSize) {///取范围的最大上限
					TotalSize = BufInfo->llBufEnd + 1;
				}
			}
		}
	}

	return TotalSize;
}

int AngKDataBuffer::ParserCommonBinXml(std::string sFileName, uint64_t nBufSize, uchar Virgin)
{
	int ret = 0;

	pugi::xml_document doc;
	pugi::xml_parse_result result;
	if (Utils::AngKCommonTools::ContainsChinese(sFileName)) {
		const wchar_t* encodedName = reinterpret_cast<const wchar_t*>(QString::fromStdString(sFileName).utf16());
		result = doc.load_file(encodedName);
	}
	else
	{
		result = doc.load_file(sFileName.c_str());
	}

	if (!result)
		return XMLMESSAGE_LOAD_FAILED;

	//pugi::xml_node get_Buffer_Node;
	//bool bInvalid = CheckXmlAlgoInvalid(doc, m_selectChipCfg.nAlgoIC, get_Buffer_Node);
	pugi::xml_node* get_Buffer_Node = Utils::AngKCommonXMLParser::findNode(doc, XML_NODE_CHIPDATA_BUFFERMAPSET);

	if (get_Buffer_Node == nullptr){//检查算法ID不在bufMap中，通过buffSize创建，如果也为0，则报错返回
		if (nBufSize == 0){
			ALOG_FATAL("BufSize Is Not Defined In Both of The XML And PCDB.", "CU", "--");
			return XMLMESSAGE_LOAD_FAILED;
		}

		ret = CreateBufferBySize(nBufSize, Virgin);//基本创建完成
	}
	else{//根据算法ID创建
		ret = CreateBufferByAlgo(doc, m_selectChipCfg);
	}

	return ret;
}

bool AngKDataBuffer::CheckXmlAlgoInvalid(pugi::xml_document& _doc, uint algoIC, pugi::xml_node& getBufNode)
{
	pugi::xml_node bufMap_node = _doc.child(XML_NODE_BUFFERMAP);
	std::string strAlgo, strBufNum, strBufSelect;
	int count = 0;
	bool bGetAlgo = false;
	while (bufMap_node)
	{
		qDebug() << bufMap_node.name();
		std::vector<tRange> rangVec;
		if (strcmp(bufMap_node.name(), "BufMap") == 0){
			strAlgo = bufMap_node.attribute("Algo").as_string();
			strBufNum = bufMap_node.attribute("BufNum").as_string();
			strBufSelect = bufMap_node.attribute("BufSelect").as_string();//先不使用

			if (strBufNum.empty()) {
				ALOG_FATAL("Load XML failed: %s.", "CU", "--", "BufNum is empty");
				return bGetAlgo;
			}
			else{
				if (!GetRange(strAlgo, rangVec, true)) {
					ALOG_FATAL("Parse XML failed: %s.", "CU", "--", "Algo invalid");
					return false;
				}
				else {
					if (CheckAlgoInRange(algoIC, rangVec)) {
						bGetAlgo = true;
					}
				}
			}
			getBufNode = bufMap_node;
		}

		bufMap_node = bufMap_node.next_sibling();
	}

	return bGetAlgo;
}

int AngKDataBuffer::ParseXMLFromExtData(tagSelectChipCfg& pChipCfg, std::string strExtDataPath, std::vector<tBufInfo*>& vpBufInfos)
{
	int Ret = 0;
	uint64_t nBufMaxAddrEnd = 0;
	pugi::xml_document doc;
	const wchar_t* encodedName = reinterpret_cast<const wchar_t*>(QString::fromStdString(strExtDataPath).utf16());
	pugi::xml_parse_result result = doc.load_file(encodedName);
	Ret = ParserXMLDoc(doc, pChipCfg.nAlgoIC, m_vPartitionInfos);
	if (Ret == 0) {
		for (int i = 0; i < vpBufInfos.size(); ++i) {
			if (i == 0) {
				nBufMaxAddrEnd = vpBufInfos[i]->llBufEnd;
			}
			else {
				if (nBufMaxAddrEnd >= vpBufInfos[i]->llBufStart) {
					ALOG_FATAL("请注意，填入XML的Buffer信息第二个Buffer的起始地址要大于前一个Buffer的结束地址.", "CU", "--");
					ALOG_FATAL("Buffer[%d] Name=%s, 起始地址为0x%I64X,上一个Buffer的结束地址为0x%I64X.", "CU", "--", i,
						vpBufInfos[i]->strBufName, vpBufInfos[i]->llBufStart, nBufMaxAddrEnd);
					Ret = -1; return Ret;
				}
				nBufMaxAddrEnd = vpBufInfos[i]->llBufEnd;
			}
		}
	}

	return Ret;
}

int AngKDataBuffer::GetBufferCount(int partitionIdx)
{
	if (partitionIdx > m_vPartitionInfos.size())
		return -1;

	int bufCount = m_vPartitionInfos[partitionIdx]->vecSubView.size();

	return bufCount;
}

int AngKDataBuffer::GetPartitionCount()
{
	return m_vPartitionInfos.size();
}

ADR AngKDataBuffer::GetCondenseSize()
{
	ADR TotalSize = 0;
	int PartCnt = 0;
	int BufCnt = 0;
	int Ret = 0;

	PartCnt = GetPartitionCount();
	for (int partIdx = 0; partIdx < PartCnt; ++partIdx)
	{
		const tPartitionInfo* partInfo;
		partInfo = GetPartitionInfo(partIdx);

		if(!partInfo->PartitionShow)
			continue;

		BufCnt = GetBufferCount(partIdx);
		for (int bufIdx = 0; bufIdx < BufCnt; ++bufIdx)
		{
			const tBufInfo* BufInfo;
			BufInfo = GetBufferInfo(bufIdx, partIdx);

			if (!BufInfo->m_bBufferShow)
				continue;

			if (BufInfo != nullptr) {
				break;
			}
			else {
				TotalSize += (BufInfo->llBufEnd + 1 - BufInfo->llBufStart);
			}
		}
	}
	return TotalSize;
}

bool AngKDataBuffer::GetRange(std::string algoICrange, std::vector<tRange>& _rangVec, bool isHex)
{
	tRange TmpRange;
	QString splitAlgo = QString::fromStdString(algoICrange);
	QStringList strList = splitAlgo.split(",");
	for (auto strAlgo : strList)
	{
		if (strAlgo.contains("-")){
			if (!GetInteralVal(strAlgo, TmpRange)) {
				return false;
			}
			_rangVec.push_back(TmpRange);
		}
		else{
			bool bOk;
			uint64_t n2Hex = -1;
			n2Hex = strAlgo.toInt(&bOk, 16);
			if (bOk)
			{
				TmpRange.Min = n2Hex;
				TmpRange.Max = TmpRange.Min;
				_rangVec.push_back(TmpRange);
			}
		}
	}

	return true;
}

bool AngKDataBuffer::CheckAlgoInRange(uint algoIC, std::vector<tRange>& _rangVec)
{
	for (auto _range : _rangVec)
	{
		if (_range.Min == -1)
			return false;

		if (_range.Max == -1)
			return false;

		if (algoIC >= _range.Min && algoIC <= _range.Max)
		{
			return true;
		}
	}

	return false;
}

bool AngKDataBuffer::GetInteralVal(QString algoRange, tRange& _2Range)
{
	uint64_t vStart, vEnd;

	QStringList algoList = algoRange.split("-");

	if (algoList.size() < 2)
		return false;

	bool bOk;
	vStart = algoList[0].toInt(&bOk, 16);
	vEnd = algoList[algoList.count() - 1].toInt(&bOk, 16);

	_2Range.Min = vStart;
	_2Range.Max = vEnd;

	return true;
}

int AngKDataBuffer::CreateBufferBySize(uint64_t nSize, uchar Virgin)
{
	int Ret = 0;
	bool bErase = true;
	SYSTEM_INFO sInfo;
	Ret = GetChipBufAloneInfos(nSize, Virgin, m_vPartitionInfos);

	if (Ret == -1)
		return Ret;

	GetSystemInfo(&sInfo);
	m_uiMapSize = 64 * sInfo.dwAllocationGranularity;

	for (auto bufInfo : m_vPartitionInfos)
	{
		for (int i = 0; i < bufInfo->vecSubView.size(); ++i)
		{
			Ret = InitTmpBufFile(bufInfo->vecSubView[i], bufInfo->partitionIdx, bErase);
			if (Ret != 0)
				return Ret;
		}
	}

	return Ret;
}

int AngKDataBuffer::GetChipBufAloneInfos(uint64_t nSize, uchar Virgin, std::vector<tPartitionInfo*>& vpPartInfos)
{
	int Ret = 0;
	for (auto bufInfo : vpPartInfos)
	{
		tBufInfo* pNewBuf = new tBufInfo;
		ClearBufInfos(bufInfo->vecSubView);
		if (pNewBuf) {
			pNewBuf->strBufName = "ChipBuffer";
			pNewBuf->strStyle = "Byte";
			pNewBuf->uBufOrgValue = Virgin;
			pNewBuf->llBufStart = 0;
			pNewBuf->llBufEnd = nSize - 1;
			pNewBuf->m_BufFind.AddrStart = pNewBuf->llBufStart;
			pNewBuf->m_BufFind.AddrEnd = pNewBuf->llBufEnd;
			pNewBuf->m_BufFind.AddrFind = pNewBuf->llBufStart;
			pNewBuf->m_BufFill.AddrStart = pNewBuf->llBufStart;
			pNewBuf->m_BufFill.AddrEnd = pNewBuf->llBufEnd;
			bufInfo->vecSubView.push_back(pNewBuf);
		}
		else {
			Ret = -1;
		}
	}
	return Ret;
}

void AngKDataBuffer::ClearBufInfos(std::vector<tBufInfo*>& vpBufInfos)
{
	for (auto bufInfo : vpBufInfos)
	{
		if (bufInfo)
		{
			SAFEDEL(bufInfo->m_BufFind.pDataEx);

			if (bufInfo->m_pBuffMapped != nullptr) {
				bufInfo->m_hBufFile->unmap(bufInfo->m_pBuffMapped);
			}
			if (bufInfo->m_hBufFile != nullptr) {
				bufInfo->m_hBufFile->close();
				bufInfo->m_hBufFile->remove();
				delete bufInfo->m_hBufFile;
			}
			//if (bufInfo->m_pBuffMapped.isAttached()) {
			//	bufInfo->m_pBuffMapped.detach();
			//}
			if (bufInfo->m_hFileMap != nullptr) {
				bufInfo->m_hFileMap->close();
				bufInfo->m_hFileMap->remove();
				delete bufInfo->m_hFileMap;
			}
			bufInfo->ReInit();
		}

		SAFEDEL(bufInfo);
	}

	vpBufInfos.clear();
}

void AngKDataBuffer::ClearPartInfos(std::vector<tPartitionInfo*>& vpPartInfos)
{
	for (auto partInfo : vpPartInfos)
	{
		ClearBufInfos(partInfo->vecSubView);
	}

	vpPartInfos.clear();
}

int AngKDataBuffer::InitTmpBufFile(tBufInfo* pBufInfo, int partIdx, bool bErase)
{
	int ret = 0;
	int tryCnt = 0;
	uint64_t adrSize = pBufInfo->llBufEnd - pBufInfo->llBufStart + 1;
	uint64_t adrFileSize;

	ret = CreateTmpBufFile(pBufInfo, partIdx, bErase);
	if (ret != 0)
		return ret;

	//  计算临时文件大小，要求大于缓冲区大小且为m_uiMapSize整数倍
	adrFileSize = (adrSize + m_uiMapSize - 1) / m_uiMapSize * m_uiMapSize;

	pBufInfo->m_hBufFile->resize(adrFileSize);
	pBufInfo->m_pBuffMapped = pBufInfo->m_hBufFile->map(0, adrFileSize);
	pBufInfo->m_hBufFile->close();
	
	//写入数据测试，后续放入BufferWrite
	//QByteArray writeBuf;
	//QDataStream DataStream(&writeBuf, QIODevice::ReadWrite);
	//DataStream.setByteOrder(QDataStream::LittleEndian);
	//DataStream << "Hello, World!";
	//DataStream << "wodiaonimade, World!";
	//DataStream.device()->seek(0);
	//DataStream.readRawData(reinterpret_cast<char*>(pBufInfo->m_pBuffMapped), writeBuf.size());

	pBufInfo->m_adrMapOffset = 0;
	pBufInfo->m_adrSize = adrSize;
	pBufInfo->m_bDirty = FALSE;

	return ret;
}

int AngKDataBuffer::CreateTmpBufFile(tBufInfo* pBufInfo, int partIdx, bool bErase)
{
	//QRandomGenerator randGen;
	QString tempPath = Utils::AngKPathResolve::localTempBufFolderPath() +
		QString("aprog-%1[0-%2].tmp~").arg(QString::fromStdString(pBufInfo->strBufName)).arg(QRandomGenerator::global()->bounded(65536));

	pBufInfo->m_hBufFile = new QTemporaryFile(tempPath);
	if (!pBufInfo->m_hBufFile->open()) {
		ALOG_FATAL("Failed to create TemporaryFile.", "CU", "--");
		return -1;
	}
	return 0;
}

int AngKDataBuffer::CreateBufferByAlgo(pugi::xml_document& _doc, tagSelectChipCfg& _selectChip)
{
	int Ret = 0;
	int bufCnt = 0;
	bool bErase = true;
	uint64_t nBufMaxAddrEnd = 0;
	SYSTEM_INFO sInfo;
	m_Virgin = _selectChip.OrgVirgin;

	//解析XML计算
	Ret = ParserXMLDoc(_doc, _selectChip.nAlgoIC, m_vPartitionInfos);
	if (Ret == 0) {
		for (auto bufInfo : m_vPartitionInfos)
		{
			for (int i = 0; i < bufInfo->vecSubView.size(); ++i) {
				if (i == 0) {
					nBufMaxAddrEnd = bufInfo->vecSubView[i]->llBufEnd;
				}
				else {
					if (nBufMaxAddrEnd >= bufInfo->vecSubView[i]->llBufStart) {
						ALOG_FATAL("请注意，填入XML的Buffer信息第二个Buffer的起始地址要大于前一个Buffer的结束地址.", "CU", "--");
						ALOG_FATAL("Buffer[%d] Name=%s, 起始地址为0x%I64X,上一个Buffer的结束地址为0x%I64X.", "CU", "--", i,
							bufInfo->vecSubView[i]->strBufName.c_str(), bufInfo->vecSubView[i]->llBufStart, nBufMaxAddrEnd);
						Ret = -1; return Ret;
					}
					nBufMaxAddrEnd = bufInfo->vecSubView[i]->llBufEnd;
				}
			}
		}
	}

	if (m_vpBufInfos.size() == 0) {///XML中没有解析到Buf信息，为了兼容性要到extdata文件中查找
		//Ret = ParseXMLFromExtData(_selectChip, _selectChip.strExtdataPath, m_vpBufInfos);
	}

	GetSystemInfo(&sInfo);
	m_uiMapSize = 64 * sInfo.dwAllocationGranularity;

	for (auto bufInfo : m_vPartitionInfos)
	{
		bufCnt = bufInfo->vecSubView.size();
		for (int i = 0; i < bufCnt; ++i)
		{
			Ret = InitTmpBufFile(bufInfo->vecSubView[i], bufInfo->partitionIdx, bErase);
			if (Ret != 0)
				return Ret;
		}
	}
	return Ret;
}

int AngKDataBuffer::ParserXMLDoc(pugi::xml_document& _doc, uint algoIC, std::vector<tPartitionInfo*>& vpPartInfos)
{
	int Ret = 0;
	ClearPartInfos(vpPartInfos);
	//pugi::xml_node get_Buffer_Node;
	//bool isAlgoGet = CheckXmlAlgoInvalid(_doc, algoIC, get_Buffer_Node);
	pugi::xml_node* get_Buffer_Node = Utils::AngKCommonXMLParser::findNode(_doc, XML_NODE_CHIPDATA_BUFFERMAPSET);

	pugi::xml_node bufferMapTable = get_Buffer_Node->child(XML_NODE_CHIPDATA_BUFFERMAPTABLE);
	int bufferMapCount = std::distance(get_Buffer_Node->begin(), get_Buffer_Node->end());
	for (int i = 0; i < bufferMapCount; ++i)
	{
		if (Utils::AngKCommonXMLParser::CheckAlgoRange(algoIC, bufferMapTable.attribute("Algo").as_string()))
			break;

		bufferMapTable = bufferMapTable.next_sibling();
	}

	if (bufferMapTable == nullptr) {
		Ret = -1;
		return Ret;
	}

	pugi::xml_node bufferPartition = bufferMapTable.child(XML_NODE_CHIPDATA_BUFFERPARTITION);
	int bufferPartitionCount = std::distance(bufferMapTable.begin(), bufferMapTable.end());

	for (int i = 0; i < bufferPartitionCount; ++i){
		std::string strPartRange, strShowBuf;
		std::vector<tRange> vPartAddrRange;
		tPartitionInfo* partInfo = new tPartitionInfo();
		partInfo->partitionName = bufferPartition.attribute("Name").as_string();
		partInfo->partitionIdx = bufferPartition.attribute("Index").as_int();
		strShowBuf = bufferPartition.attribute("Visiable").as_string();
		GetIsShow(strShowBuf, partInfo->PartitionShow);

		pugi::xml_node viewNode = bufferPartition.child(XML_NODE_CHIPDATA_VIEW);
		int viewNodeCount = std::distance(bufferPartition.begin(), bufferPartition.end());
		for (int viewIdx = 0; viewIdx < viewNodeCount; ++viewIdx) {
			std::string strRange, strOrgValue, strShowBuf;
			std::vector<tRange> vAddrRange;

			tBufInfo* pNewBuf = new tBufInfo();
			pNewBuf->strBufName = viewNode.attribute("Name").as_string();
			strRange = viewNode.attribute("AddrRange").as_string();
			strOrgValue = viewNode.attribute("OrgValue").as_string();
			pNewBuf->strStyle = viewNode.attribute("Style").as_string();
			strShowBuf = viewNode.attribute("Visiable").as_string();
			GetIsShow(strShowBuf, pNewBuf->m_bBufferShow);
			if (pNewBuf->strStyle == "") {
				pNewBuf->strStyle = "Byte";
			}

			bool bOK;
			pNewBuf->uBufOrgValue = QString::fromStdString(strOrgValue).toInt(&bOK, 16);
			if (!bOK) {
				SAFEDEL(pNewBuf);
				ALOG_FATAL("Parse XML failed: %s.", "CU", "--", "OrgValue include invalid char");
				Ret = -1; goto __end;
			}
			if (GetRange(strRange, vAddrRange, TRUE) == FALSE /*|| vAddrRange.size() != 2*/) {
				SAFEDEL(pNewBuf);
				ALOG_FATAL("Parse XML failed: %s.", "CU", "--", "AddrRange invalid");
				Ret = -1; goto __end;
			}
			pNewBuf->llBufStart = vAddrRange[0].Min;
			pNewBuf->llBufEnd = vAddrRange[0].Max;
			pNewBuf->m_BufFind.AddrStart = pNewBuf->llBufStart;
			pNewBuf->m_BufFind.AddrEnd = pNewBuf->llBufEnd;
			pNewBuf->m_BufFind.AddrFind = pNewBuf->llBufStart;

			pNewBuf->m_BufFill.AddrStart = pNewBuf->llBufStart;
			pNewBuf->m_BufFill.AddrEnd = pNewBuf->llBufEnd;
			partInfo->vecSubView.push_back(pNewBuf);

			viewNode = viewNode.next_sibling();
		}

		vpPartInfos.push_back(partInfo);
		bufferPartition = bufferPartition.next_sibling();
	}

__end:
	if (Ret != 0) {
		ClearPartInfos(vpPartInfos);
	}
	return Ret;
}

int AngKDataBuffer::MapBuffer(tBufInfo* pBufInfo, uint64_t adrICOffset)
{
	int Ret = 0;
	uint64_t adrOffset = 0;
	if (pBufInfo->llBufStart > adrICOffset) {
		ALOG_FATAL("MapBuffer error: ICOffset=0x%I64X < BufferICOffset=0x%I64X.", "CU", "--", adrICOffset, pBufInfo->llBufStart);
		Ret = -1;
		goto __end;
	}

	//IC的空间偏移转为映射文件内的偏移
	adrOffset = adrICOffset - pBufInfo->llBufStart;
	adrOffset = (adrOffset / m_uiMapSize) * m_uiMapSize;	//对齐
	if (pBufInfo->m_adrMapOffset == adrOffset) {   //为当前映射区
		goto __end;
	}

	//缓存删除
	if (pBufInfo->m_pBuffMapped != nullptr) {
		pBufInfo->m_hBufFile->unmap(pBufInfo->m_pBuffMapped);
	}

	pBufInfo->m_pBuffMapped = pBufInfo->m_hBufFile->map(adrOffset, m_uiMapSize);

	if (pBufInfo->m_pBuffMapped == nullptr){
		ALOG_FATAL("Map file error.", "CU", "--");
		Ret = -3;
		goto __end;
	}

	pBufInfo->m_adrMapOffset = adrOffset;


__end:
	return Ret;
}

void AngKDataBuffer::GetIsShow(std::string strShow, bool& setBool)
{
	if (strShow == "") {
		setBool = true;
	}
	else {
		if (strcmp(strShow.c_str(), "TRUE") == 0) {
			setBool = true;
		}
		else {
			setBool = false;
		}
	}
}

tBufInfo* AngKDataBuffer::GetBufferInfo(int bufIdx, int partitionIdx)
{
	if (bufIdx <= m_vPartitionInfos[partitionIdx]->vecSubView.size()){
		return m_vPartitionInfos[partitionIdx]->vecSubView[bufIdx];
	}

	return nullptr;
}

tPartitionInfo* AngKDataBuffer::GetPartitionInfo(int partitionIdx)
{
	if (partitionIdx <= m_vPartitionInfos.size()) {
		return m_vPartitionInfos[partitionIdx];
	}

	return nullptr;
}

void AngKDataBuffer::GetPartitionInfo(std::vector<tPartitionInfo*>& partVec)
{
	partVec = m_vPartitionInfos;
}

void AngKDataBuffer::GetBufferMapByJson(nlohmann::json& bufferJson, std::vector<tPartitionInfo*>& vpPartInfos)
{
	int partGroupCnt = bufferJson["PartitionGroupCnt"];
	
	for (int partIdx = 0; partIdx < partGroupCnt; ++partIdx)
	{
		nlohmann::json partitionMap = bufferJson["Partitions"][partIdx];
		std::string test = partitionMap.dump();
		tPartitionInfo* partInfo = new tPartitionInfo();
		partInfo->partitionIdx = QString::fromStdString(partitionMap["Index"]).toUInt();
		partInfo->partitionName = partitionMap["Name"];
		partInfo->PartitionShow = partitionMap["Visiable"] == "TRUE" ? true : false;;
		QStringList rangeList = QString::fromStdString(partitionMap["AddrRange"]).split("-");
		bool bOk;
		if (!rangeList.isEmpty()){
			partInfo->llPartitionStart = rangeList[0].toULongLong(&bOk, 16);
			partInfo->llPartitionEnd = rangeList[1].toULongLong(&bOk, 16);
		}

		int viewCnt = partitionMap["ViewsCnt"];
		
		for (int viewIdx = 0; viewIdx < viewCnt; ++viewIdx)
		{
			bool bOk;
			tBufInfo* bufInfo = new tBufInfo();
			nlohmann::json viewsInfoMap = partitionMap["ViewsInfo"][viewIdx];
			bufInfo->strBufName = viewsInfoMap["Name"];
			bufInfo->uBufOrgValue = QString::fromStdString(viewsInfoMap["OrgValue"]).toUShort(&bOk, 16);
			//bufInfo->strStyle = viewsInfoMap["ShowStyle"];
			bufInfo->m_bBufferShow = viewsInfoMap["Visiable"] == "TRUE" ? true : false;
			QStringList rangeList = QString::fromStdString(viewsInfoMap["AddrRange"]).split("-");

			
			if (!rangeList.isEmpty()) {
				bufInfo->llBufStart = rangeList[0].toULongLong(&bOk, 16);
				bufInfo->llBufEnd = rangeList[1].toULongLong(&bOk, 16);
			}
			partInfo->vecSubView.push_back(bufInfo);
		}
		vpPartInfos.push_back(partInfo);
	}
}

void AngKDataBuffer::GetWriteRange(ADR& WriteAddrMin, ADR& WriteAddrMax)
{
	WriteAddrMin = m_WriteAddrMin;
	WriteAddrMax = m_WriteAddrMax;
}

void AngKDataBuffer::ResetWriteRange()
{
	m_WriteAddrMin = -1;
	m_WriteAddrMax = 0;
}

int AngKDataBuffer::ClearBuffer(tBufInfo* pBufInfo)
{
	int Ret = 0;
	int DataBufSize = 32 * 1024;
	uchar* pData = NULL;
	pData = new uchar[DataBufSize];
	if (!pData) {
		ALOG_FATAL("ClearBuffer: memory alloc failed.", "CU", "--");
		Ret = -1; goto __end;
	}

	uint64_t TotalWrite = pBufInfo->llBufEnd - pBufInfo->llBufStart + 1;
	uint64_t Byteswrite, Offset = 0;
	memset(pData, pBufInfo->uBufOrgValue, DataBufSize);
	while (TotalWrite > 0) {
		Byteswrite = TotalWrite > DataBufSize ? DataBufSize : TotalWrite;
		if (BufferWrite(pBufInfo->llBufStart + Offset, pData, Byteswrite) == 0) {
			ALOG_FATAL("ClearBuffer: WriteBuf failed.", "CU", "--");
			Ret = -1; goto __end;
		}
		TotalWrite -= Byteswrite;
		Offset += Byteswrite;
	}

__end:
	SAFEDEL_ARRAY(pData);
	return Ret;
}

QString AngKDataBuffer::GetChipFile(int chipType)
{
	QString chipFileName;

	switch (chipFileType(chipType))
	{
	case chipData:
		chipFileName = QString::fromStdString(m_selectChipCfg.strICDataXmlPath);
		break;
	case chipConfig:
		chipFileName = QString::fromStdString(m_selectChipCfg.strICConfigXmlPath);
		break;
	case masterDriver:
		chipFileName = QString::fromStdString(m_selectChipCfg.strMasterDriverPath);
		break;
	case deviceDriver:
		chipFileName = QString::fromStdString(m_selectChipCfg.strDeviceDriverPath);
		break;
	case checkFile:
		chipFileName = QString::fromStdString(m_selectChipCfg.strChksumDllPath);
		break;
	default:
		break;
	}

	return chipFileName;
}
