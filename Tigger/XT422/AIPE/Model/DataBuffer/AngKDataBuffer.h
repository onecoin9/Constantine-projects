#pragma once

#include "IDataBuffer.h"
#include "DataBufferDefine.h"
#include "XmlDefine.h"
#include "json.hpp"

/*AP8000原使用bufMap.dll，新客户端全部基于外挂式，所以现在将bufferInfo结构体移到此处*/
enum chipFileType {
	chipData,
	chipConfig,
	masterDriver,
	deviceDriver,
	checkFile
};

class AngKDataBuffer : public IDataBuffer
{
public:
	AngKDataBuffer(QObject* parent = nullptr);
	~AngKDataBuffer();

	//初始化buffer区域，从
	int InitBuffer(uint64_t nSize, uchar virgin);

	int CreateBuffer(uint64_t Size, uchar Virgin);

	//virgin 获取根据操作掩码ulOpCfgMask(program、erase等等)计算，新版操作掩码更改为了json值，是否使用再看
	void SetChipConfig(uint unAlgoIC, QStringList mstDrvList, QStringList devDrvList, uchar virgin = NULL);

	virtual ADR BufferRead(ADR adrStart, uchar* pBuf, ADR adrLen);

	virtual ADR BufferWrite(ADR adrStart, uchar* pBuf, ADR adrLen);

	virtual int BufferSwitchPartition(int nPartIndex);

	virtual int BufferGetCurPartition();

	virtual void BufferAddTemporaryDataRemap();

	virtual void BufferRemoveTemporaryDataRemap();

	virtual void BufferCheckPartitionExist();

	//获取实际覆盖区域大小，以最后一个Buffer的上限加1
	ADR GetSize();

	//解析通用bin目录下xml
	int ParserCommonBinXml(std::string sFileName, uint64_t nBufSize, uchar Virgin);

	//检查选择算法ID在XML中是否有效
	bool CheckXmlAlgoInvalid(pugi::xml_document& _doc, uint algoIC, pugi::xml_node& getBufNode);

	//解析extdata文件
	int ParseXMLFromExtData(tagSelectChipCfg& pChipCfg, std::string strXMLPath, std::vector<tBufInfo*>& vpBufInfos);

	//获取buffer数量
	int GetBufferCount(int partitionIdx);

	//获取partition数量
	int GetPartitionCount();

	//获取压缩buffer地址区间大小
	ADR GetCondenseSize();

	//获取buffer信息
	tBufInfo* GetBufferInfo(int bufIdx, int partitionIdx);

	//获取Partition信息
	tPartitionInfo* GetPartitionInfo(int partitionIdx);

	//获取Partition信息
	void GetPartitionInfo(std::vector<tPartitionInfo*>& partVec);

	//读工程文件获取buffer并进行创建
	void GetBufferMapByJson(nlohmann::json& bufferJson, std::vector<tPartitionInfo*>& vpPartInfos);

	//获取当前阶段Write的范围,为左闭右开[Min,Max)，用于Swap操作或者其他提示
	void GetWriteRange(ADR& WriteAddrMin, ADR& WriteAddrMax); 

	///重新设置Write的范围
	void ResetWriteRange();

	//清空Buffer内容
	int ClearBuffer(tBufInfo* pBufInfo);

	QString GetChipFile(int chipType);

	uchar GetVirgin() { return m_Virgin; }
private:
	//获取algo范围
	bool GetRange(std::string algoICrange, std::vector<tRange>& _rangVec, bool isHex);

	//检查algo是否在bufMap中
	bool CheckAlgoInRange(uint algoIC, std::vector<tRange>& _rangVec);

	//获取算法值，转10进制
	bool GetInteralVal(QString algoRange, tRange& _2Range);

	//通过bufferSize进行创建，相关函数
	int CreateBufferBySize(uint64_t nSize, uchar Virgin);
	int GetChipBufAloneInfos(uint64_t nSize, uchar Virgin, std::vector<tPartitionInfo*>& vpPartInfos);
	void ClearBufInfos(std::vector<tBufInfo*>& vpBufInfos);
	void ClearPartInfos(std::vector<tPartitionInfo*>& vpPartInfos);

	///InitTmpBufFile：根据pBufInfo的初始化Buffer文件
	int InitTmpBufFile(tBufInfo* pBufInfo, int partIdx, bool bErase);
	int CreateTmpBufFile(tBufInfo* pBufInfo, int partIdx, bool bErase);

	//通过AlgoID进行创建，相关函数
	int CreateBufferByAlgo(pugi::xml_document& _doc, tagSelectChipCfg& _selectChip);
	int ParserXMLDoc(pugi::xml_document& _doc, uint algoIC, std::vector<tPartitionInfo*>& vpPartInfos);

	int MapBuffer(tBufInfo* pBufInfo, uint64_t adrICOffset);

	void GetIsShow(std::string strShow, bool& setBool);
private:
	ADR m_WriteAddrMin;
	ADR m_WriteAddrMax;
	uchar m_Virgin;			//非空白部分使用的Virgin值
	uint m_uiMapSize;		//映射视图大小
	tagSelectChipCfg		m_selectChipCfg;
	std::vector<tBufInfo*>	m_vpBufInfos;	//保存Buffer信息指针
	std::vector<tPartitionInfo*> m_vPartitionInfos;	//保存Partition指针
};
