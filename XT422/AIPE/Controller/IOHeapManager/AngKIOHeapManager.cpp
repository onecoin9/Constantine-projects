#include "AngKIOHeapManager.h"
#include "AngKCommonTools.h"
#include "AngKPathResolve.h"
#include "IOHeapDeviceDefine.h"
#include "AngKDataBuffer.h"
#include <QDir>
AngKIOHeapManager::AngKIOHeapManager(QObject *parent)
	: AngKIOHeapDevice(parent)
	, m_nBaseAddr(0)
{
	
}

AngKIOHeapManager::~AngKIOHeapManager()
{

}

int32_t AngKIOHeapManager::HeapWrite(uint64_t offset, uint32_t uAlgo, IOChannel channelType, DataArea areaType, QByteArray& pData)
{
	int ret = 0;
	uint64_t RealAddr = 0;
	uint32_t RealSize = 0;

	RealAddr = m_nBaseAddr + offset; // offset由界面设置Relocation获取
	RealSize = pData.size();

	//DataRemap
	pugi::xml_document doc;
	std::vector<RemapData> vecRemapData;
	pugi::xml_node* remapData_node = Utils::AngKCommonXMLParser::findNode(doc, XML_NODE_CHIPDATA_DATAREMAPSET);
	if (remapData_node !=NULL && !remapData_node->empty())
	{
		//获取DataRemap节点的值			
		nlohmann::json cmdJson;

		Utils::AngKCommonXMLParser::DataRemapInfo_Json(*remapData_node, uAlgo, cmdJson);

		if (!cmdJson.is_null()) {
			int mapCount = cmdJson["DataMapCnt"];
			for (int i = 0; i < mapCount; ++i)
			{
				nlohmann::json dataMap = cmdJson["DataMaps"][i];

				RemapData rData;
				bool ok;
				rData.OrgAddr = QString::fromStdString(dataMap["OrgAddr"]).toULongLong(&ok, 16);
				rData.MapAddr = QString::fromStdString(dataMap["MapAddr"]).toULongLong(&ok, 16);
				rData.Size = QString::fromStdString(dataMap["Size"]).toULongLong(&ok, 16);
				vecRemapData.push_back(rData);
			}
		}

		remapData_node = nullptr;
		delete remapData_node;
	}

	//buffMap地址映射判断
	ADR RtnBytes = 0;
	ADR writeBytes = 0;
	for (auto mapData : vecRemapData)
	{
		if(RealAddr >= mapData.OrgAddr + mapData.Size
			|| (RealAddr + RealSize <= mapData.OrgAddr)) {//没有交集
			continue;
		}
		else {//有交集的地方需要重新映射，没有交集则正常写入
			uint64_t AddrStart = RealAddr;
			uint64_t TmpStart, TmpLen = 0;

			while (RealSize > 0){
				if (RealAddr < mapData.OrgAddr) {//正常写入映射位置
					TmpStart = RealAddr;
					TmpLen = mapData.OrgAddr - RealAddr;
					writeBytes = m_DataBuffer->BufferWrite(TmpStart, (uchar*)pData.data(), TmpLen);
					if (writeBytes != TmpLen) {
						goto __end;
					}

					///计算剩余部分
					RealAddr += TmpLen;
					RealSize -= TmpLen;
					RtnBytes += writeBytes;
					continue;
				}
				else if (RealAddr >= mapData.OrgAddr && RealAddr < mapData.OrgAddr + mapData.Size) {//起始地址在remap映射地址范围内
					uint64_t MaxLen;
					MaxLen = mapData.OrgAddr + mapData.Size - RealAddr;///该映射区间最大长度
					TmpStart = RealAddr - mapData.OrgAddr + mapData.MapAddr;///进行地址映射
					TmpLen = RealSize > MaxLen ? MaxLen : RealSize; ///计算映射区间的长度
					writeBytes = m_DataBuffer->BufferWrite(TmpStart, (uchar*)pData.data() + RealAddr, TmpLen);///将数据写入映射区间
					if (writeBytes != TmpLen) {
						goto __end;
					}
					///计算剩余部分,有可能还有一部分残留
					RealAddr += TmpLen;
					RealSize -= TmpLen;
					RtnBytes += writeBytes;
					continue;
				}
				else if (RealAddr >= mapData.OrgAddr + mapData.Size) {///超过映射区域的另外一头,可能是另外一个映射区域，留给下一个映射区域处理，或者最后处理
					break;
				}
			}
		}
	}

	//写入有两种情况：1、将remap剩余的地址与数据写入BufMap 2、没有remap的情况下数据写入BufMap
	if (RealSize > 0) {
		writeBytes = m_DataBuffer->BufferWrite(RealAddr, (uchar*)pData.data(), RealSize);
		if (writeBytes != RealSize) {
			goto __end;
		}
		RtnBytes += writeBytes;
	}

	//ALV计算


	switch (channelType)
	{
	case USB:
		break;
	case Ethernet:
	{

	}
		break;
	default:
		break;
	}

	if (RealSize == 0)
		return -1;

__end:
	//return m_DataBuffer->BufferWrite(RealAddr, (uchar*)pData.data(), RealSize);
	return RtnBytes;
}

int32_t AngKIOHeapManager::HeapRead(uint64_t offset, uint32_t uAlgo, IOChannel channelType, DataArea areaType, QByteArray& pData)
{
	return int32_t();
}

void AngKIOHeapManager::setDataBuffer(IDataBuffer* _dataBuf)
{
	m_DataBuffer = _dataBuf;
}
