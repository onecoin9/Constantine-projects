#pragma once

enum IOChannel
{
	USB = 1,
	Ethernet = 2
};

enum DataArea
{
	SSD = 1,
	DDR = 2
};

typedef struct {
	std::string strName; ///当前Map的名称
	uint64_t OrgAddr;		///实际地址
	uint64_t MapAddr; ///目标映射地址	
	uint64_t Size;		///映射大小，所以实际地址是[OrgAddr,OrgAddr+Size) ---> [MapAddr,MapAddr+Size)
}RemapData;