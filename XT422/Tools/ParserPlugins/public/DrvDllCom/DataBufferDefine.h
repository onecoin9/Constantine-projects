#pragma once
#include <iostream>
#include <QMetaType>
#include <QTemporaryFile>
#include <QSharedMemory>

#define CHKSUMNAME_LEN (16)
#define BUFFER_RW_SIZE (1024 * 32)

enum eSwapType {
	SWAPTYPE_NONE,  ///不进行Swap
	SWAPTYPE_BYTE,	///字节Swap 01 02 03 04 -> 02 01 04 03
	SWAPTYPE_WORD,  ///双字节Swap 01 02 03 04 -> 03 04 01 02
	SWAPTYPE_BW,		///字节+双字节Swap  01 02 03 04 -> 04 03 02 01
};

typedef struct tagBufferInfo {  ///一个Buffer的信息
	std::string strBufName;  /// Buffer的名称
	uint64_t	uAddrStart;  ///Buffer的起始地址，为闭区间
	uint64_t	uAddrEnd;    ///Buffer的结束地址，为闭区间
	int			nType;		 ///buffer类型
}tagBufInfo;

typedef struct tagBufferInfoExt {  ///一个Buffer的信息
	std::string strBufName;		/// Buffer的名称
	uint64_t	uAddrStart;		///Buffer的起始地址，为闭区间
	uint64_t	uAddrEnd;		///Buffer的结束地址，为闭区间
	uchar		DefalutV;		///默认值
	int			nType;			///buffer类型
	uchar		Reserved[16];	///预留使用
}tagBufInfoExt;

typedef struct tagBufFind {
	uint64_t AddrStart;
	uint64_t AddrEnd;
	uint64_t AddrFind;
	uchar	pData[32];
	uchar*	pDataEx;
	int		DataExSize;
	bool	AsciiTypeEn;
	bool	RandomEn;
}tBufFind;

typedef tBufFind tBufFill;

typedef struct {
	uint64_t	StartAddr;///校验值显示的起始位置
	uint64_t	Size;    ///校验值显示的大小
	char		ChecksumType[CHKSUMNAME_LEN];///校验值显示名称
}tChksumFix;

typedef struct tagRange {
	uint64_t Min;
	uint64_t Max;

	tagRange()
	{
		Min = -1;
		Max = -1;
	}
}tRange;

typedef struct BufInfo {
	std::string		strBufName;			//Buffer的名称
	uint64_t		llBufStart;			//Buffer对应IC地址空间的起始地址 ，闭区间
	uint64_t		llBufEnd;			//Buffer对应IC地址空间的末尾 , 闭区间
	uchar			uBufOrgValue;		//Buffer初始空白值
	std::string		strStyle;			//BufferEditor的时候显示的默认风格，Byte为8bits，Word为16bits
	std::string		strBufFileName;		//Buffer文件的名称
	QTemporaryFile* m_hBufFile;			//打开的Buffer文件的句柄
	QTemporaryFile* m_hFileMap;			//映射的文件句柄
	//QSharedMemory	m_pBuffMapped;		//当前映射视图位置
	uchar*			m_pBuffMapped;		//当前映射视图位置
	uint64_t		m_adrMapOffset;		//当前映射视图在文件中的偏移位置
	bool			m_bDirty;			//映射视图是否无效
	uint64_t		m_adrSize;			//文件总大小，
	int				m_SwapSetting;		//是否进行Swap
	uint64_t		m_CurGotoPos;		//记录Goto的位置
	tBufFind		m_BufFind;			//这个结构体用于存储查找缓冲区中的数据
	tBufFill		m_BufFill;			//这个结构体用于填充缓冲区中的数据
	bool			m_bWordDisp;		//是否显示Word模式
	tChksumFix		m_ChksumFix;		//固件显示的校验值信息
	bool			m_bBufferShow;		//是否显示，如果为TRUE则表示显示，否则隐藏不可见
	uint32_t		m_partitionIndex;	//所属分区索引
	uint64_t		m_writeFileSize;	//档案数据写入的大小

	BufInfo() {
		ReInit();
	};
	void ReInit() {
		strBufName = "";
		llBufStart = 0;
		llBufEnd = 0;
		uBufOrgValue = 0;
		strStyle = "Byte";
		strBufFileName = "";
		m_hBufFile = nullptr;
		m_hFileMap= nullptr;
		m_pBuffMapped = nullptr;
		m_adrMapOffset = 0;
		m_bDirty = true;
		m_adrSize = 0;
		m_SwapSetting = SWAPTYPE_NONE;
		m_CurGotoPos = 0;
		m_BufFind.AddrStart = 0;
		m_BufFind.AddrEnd = 0;
		m_BufFind.AsciiTypeEn = 0;
		m_BufFind.DataExSize = 0;
		memset(m_BufFind.pData, 0, 32);
		m_BufFind.pDataEx = nullptr;

		m_BufFill.AddrStart = 0;
		m_BufFill.AddrEnd = 0;
		m_BufFill.AsciiTypeEn = 0;
		m_BufFill.DataExSize = 0;
		m_BufFill.RandomEn = 0;
		memset(m_BufFill.pData, 0, 32);
		m_BufFill.pDataEx = nullptr;
		m_bWordDisp = false;
		m_ChksumFix.Size = 0;
		m_ChksumFix.StartAddr = 0;
		memset(m_ChksumFix.ChecksumType, 0, CHKSUMNAME_LEN);
		m_bBufferShow = true;
	}
}tBufInfo;

typedef struct PartitionInfo {
	std::string partitionName;			//分区名称
	uint32_t	partitionIdx;			//分区索引
	uint64_t	llPartitionStart;		//Partition对应IC地址空间的起始地址 ，闭区间
	uint64_t	llPartitionEnd;			//Partition对应IC地址空间的末尾 , 闭区间
	bool		PartitionShow;			//是否显示，如果为TRUE则表示显示，否则隐藏不可见
	std::vector<tBufInfo*> vecSubView;	//分区内部对应保存的
	PartitionInfo() {
		ReInit();
	};
	void ReInit() {
		partitionName = "";
		partitionIdx = -1;
		llPartitionStart = 0;
		llPartitionEnd = 0;
		PartitionShow = true;
		vecSubView.clear();
	}
}tPartitionInfo;

typedef struct tagSelectChipCfg {
	std::string		strDriverZipPath;		//外挂压缩文件路径，全路径。
	uint			nAlgoIC;				//IC算法索引
	uchar			OrgVirgin;				//Buffer的Virgin值，如果XML中有配置则使用XML中的值
	std::string		strChksumDllPath;		//ChksumDll路径所在文件夹目录
	std::string		strICDataXmlPath;		//data.xml文件路径
	std::string		strICConfigXmlPath;		//config.xml文件路径
	std::string		strMasterDriverPath;	//MasterDriver文件路径
	std::string		strDeviceDriverPath;	//DeviceDriver文件路径
}tSelectChipCfg;