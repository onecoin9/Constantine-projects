#pragma once
#include <windows.h>
#include <iostream>
#include <fstream>
#include <ostream>
#include <string>
#include <iomanip>
#include <vector>
#include <filesystem>
#include <list>
#include "srec.h"
#include "adrv.h"

/* Data structures */
static srec_info_t mySrecInfo;
static uint8_t SR_Buf[SREC_MAX_BYTES];
static uint8_t sr_data_buf[SREC_DATA_MAX_BYTES];

void printHelp() {
	std::cout << "Usage: mytool [options]\n"
		<< "Options:\n"
		<< "  -h, --help     Display this help message\n"
		<< "  -v, --version  Display the version\n"
		<< "  -o, --output   Save the output content in the specified file\n";
}

void printVersion() {
	std::cout << "MyTool Version 1.0\n";
}

void greetUser(const std::string& name) {
	std::cout << "Hello, " << name << "!\n";
}

extern "C" {void eatup_srec_line(uint8* bufs, uint8 count); }
int calc_crc16sum(const uint8_t* buf, size_t size, uint16_t* pCRC16Sum);
int calc_crc16sum_checkvirgin(uint8_t* buf, size_t size, uint16_t* pCRC16Sum, uint8_t Virgin, size_t* pIsVirgin);
extern uint16_t CRC16Table[256];

void printHex(const unsigned char* data, std::size_t size) {
	for (std::size_t i = 0; i < size; ++i) {
		// 设置输出宽度为2，不足两位的在前面填充0
		std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(data[i]) << " ";
	}
	std::cout << std::dec << std::endl; // 恢复输出为十进制，换行
}

uint16_t calculateCRC16(const uint8_t* data, size_t size) {
	uint16_t crc = 0xFFFF;

	for (size_t i = 0; i < size; ++i) {
		crc = (crc << 8) ^ CRC16Table[((crc >> 8) ^ data[i]) & 0xFF];
	}

	return crc;
}

std::string getFileExtension(const std::string& filename) {
	size_t dotIndex = filename.find_last_of('.');

	if (dotIndex != std::string::npos) {
		// 找到了点，提取后缀
		return filename.substr(dotIndex + 1);
	}
	else {
		// 没有找到点，文件名没有后缀
		return "";
	}
}

std::string getFilenameWithoutExtension(const std::string& pathOrFilename) {
	size_t lastSlash = pathOrFilename.find_last_of("/\\");
	size_t lastDot = pathOrFilename.rfind('.');

	if (lastDot != std::string::npos && (lastSlash == std::string::npos || lastDot > lastSlash)) {
		// 找到了点，去掉后缀
		return pathOrFilename.substr(lastSlash + 1, lastDot - lastSlash - 1);
	}
	else {
		// 没有找到点，文件名没有后缀
		return pathOrFilename.substr(lastSlash + 1);
	}
}

void addAlVarea(uint8* _FirstDataPos, std::vector<uint8_t>& _alvDataVec, uint32_t& _len, std::vector<tDrvALV*>& _alvVec, uint32_t& _alvSize)
{
	
#if 0
	tDrvALV* binAlv = (tDrvALV*)malloc(sizeof(tDrvALV) + _alvDataVec.size());
	binAlv->Offset = reinterpret_cast<uint64_t>(_FirstDataPos);
	binAlv->Offset &= 0x00000000FFFFFFFF;//高位设置为0
	binAlv->Len = _len;
	for (int i = 0; i < _alvDataVec.size(); ++i)
	{
		binAlv->Data[i] = _alvDataVec[i];
	}
	_alvVec.push_back(binAlv);

	_alvSize += sizeof(binAlv->Offset) + sizeof(binAlv->Len) + _alvDataVec.size();
#else
	int ALVInfoSize = sizeof(tDrvALV) + _alvDataVec.size();
	tDrvALV* binAlv = (tDrvALV*)malloc(ALVInfoSize);
	binAlv->Offset = (reinterpret_cast<uint64_t>(_FirstDataPos))& 0x00000000FFFFFFFF;
	binAlv->Len = _len;
	for (int i = 0; i < _alvDataVec.size(); ++i)
	{
		binAlv->Data[i] = _alvDataVec[i];
	}
	//memcpy(binAlv->Data, AL)
	_alvVec.push_back(binAlv);

	_alvSize += ALVInfoSize;
#endif
}

void outputFile(const std::string& srcName, const std::string& destName)
{
	_tADrvHeader testHeader;
	uint32_t alvSize = 0;
	int nALVNum = 0;
	//Magic
	int magicLen = sizeof(testHeader.Magic) / sizeof(testHeader.Magic[0]);
	std::string strMagic = "ADRV";
	strcpy_s((char*)testHeader.Magic, magicLen + 1, strMagic.c_str());

	//UUID
	memset(testHeader.UUID, 0, sizeof(testHeader.UUID) / sizeof(testHeader.UUID[0]));
	//ALVOffset
	testHeader.ALVOffset = 128;

	//EncrypType
	testHeader.EncrypType = 0;

	//Reserved
	memset(testHeader.Reserved, 0, sizeof(testHeader.Reserved));
	std::cout << sizeof(testHeader) << std::endl;
	//return 0;

	std::vector<tDrvALV*> alvVec;
	std::vector<uint8_t> alvDataVec;
	uint8* FirstDataPos = 0;
	//tDrvALV结构数据
	uint64_t offset = 0;
	uint32_t Len = 0;

	//uint8* preAddr = 0;
	uint32_t preAddr = 0;
	std::ifstream ifs;
	ifs.open(srcName, std::ios::in);
	std::ofstream ofs;

	if (!SetFileAttributesA(destName.c_str(), GetFileAttributesA(destName.c_str()) & ~FILE_ATTRIBUTE_READONLY)) {
		std::cout << "Error changing file attribute" << std::endl;
	}
	else {
		std::cout << "File attribute changed successfully" << std::endl;
	}

	ofs.open(destName, std::ios::out | std::ios::binary);
	ofs.seekp(sizeof(testHeader), std::ios::beg);
	if (ifs.is_open() && ofs.is_open())
	{
		std::cout << "文件打开成功" << std::endl;
		mySrecInfo.sr_data = sr_data_buf;
		std::string buf;
		while (getline(ifs, buf))
		{
			//std::cout << buf << std::endl;
			//srec_info_t mySrecInfo;
			//initializeSrecInfo(&mySrecInfo);
			uint8 testChar = decode_srec_line((uint8*)(buf.c_str()), &mySrecInfo);
			if (mySrecInfo.sr_data)
			{
				switch (mySrecInfo.type) {
				case SREC_TYPE_0:
					break;
				case SREC_TYPE_1:
				case SREC_TYPE_2:
				case SREC_TYPE_3:
				{

					if (mySrecInfo.addr == (uint8_t*)0xC00399E8 || mySrecInfo.addr == (uint8_t*)0xC003A130)
					{
						std::cout << "join" << std::endl;
					}

					//if (AddrEnd == mySrecInfo.addr) {
					//	// WriteData to File
					//}
					//else {
					//	// Write AL
					//}

					if (preAddr == 0)
					{
						FirstDataPos = mySrecInfo.addr;
						for (int i = 0; i < mySrecInfo.dlen; ++i)
						{
							alvDataVec.push_back(mySrecInfo.sr_data[i]);
						}
						Len += mySrecInfo.dlen;
					}
					else if (preAddr == (uint32)mySrecInfo.addr)
					{
						for (int i = 0; i < mySrecInfo.dlen; ++i)
						{
							alvDataVec.push_back(mySrecInfo.sr_data[i]);
						}
						Len += mySrecInfo.dlen;
					}
					else
					{
						preAddr = 0;
						//先对上一个进行封装，在进行下一个判断

						addAlVarea(FirstDataPos, alvDataVec, Len, alvVec, alvSize);

						FirstDataPos = 0;
						Len = 0;
						alvDataVec.clear();

						//新数据段封装
						FirstDataPos = mySrecInfo.addr;
						for (int i = 0; i < mySrecInfo.dlen; ++i)
						{
							alvDataVec.push_back(mySrecInfo.sr_data[i]);
						}
						Len += mySrecInfo.dlen;
					}


					preAddr = (uint32)mySrecInfo.addr + mySrecInfo.dlen;//直接强制转换回来，preAddr要定义为一个整形
					//preAddr = mySrecInfo.addr + mySrecInfo.dlen;//直接强制转换回来，preAddr要定义为一个整形
				}
					break;
				case SREC_TYPE_5:
					break;
				case SREC_TYPE_7:
				case SREC_TYPE_8:
				case SREC_TYPE_9:
					testHeader.EntryAddr = (uint32)mySrecInfo.addr;
					break;
				}

			}

			//freeSrecInfo(&mySrecInfo);
		}

		//文件最终读完可能没有字段了，此时需要将缓存读取的数据在重新写入一次

		addAlVarea(FirstDataPos, alvDataVec, Len, alvVec, alvSize);
	}
	else
	{
		std::cout << "文件打开失败" << std::endl;
		ifs.close();
		ofs.close();
		return;
	}
	//ALVSize
	testHeader.ALVSize = alvSize;

	//ALVNum
	testHeader.ALVNum = alvVec.size();

	//Checksum
	testHeader.Checksum = 0;
	//ALVChksum
	testHeader.ALVChksum = 0;

	ofs.seekp(0, std::ios::beg);
	ofs.write(reinterpret_cast<char*>(&testHeader), sizeof(testHeader));
	//CRC16Sum = 0;
	for (int i = 0; i < alvVec.size(); ++i)
	{
		ofs.write(reinterpret_cast<char*>(alvVec[i]), sizeof(alvVec[i]->Offset) + sizeof(alvVec[i]->Len) + alvVec[i]->Len);
		ofs.flush();
		//calc_crc16sum()
	}
	//关闭文件   
	ifs.close();
	//ofs.close();
	ofs.flush();
	//重新读取原来文件写入chksum
	uint16_t alvChksum = 0;
	std::ifstream reIfs;
	reIfs.open(destName, std::ios::binary | std::ios::ate);
	std::streampos newfileSize;
	if (reIfs.is_open())
	{
		reIfs.seekg(0, std::ios::end);
		newfileSize = reIfs.tellg();
		reIfs.seekg(0, std::ios::beg);

		unsigned char* buffer = new unsigned char[newfileSize];

		if (reIfs.read(reinterpret_cast<char*>(buffer), newfileSize)) {

			// 在这里可以使用 buffer 存储文件的内容
			uint16_t* crc16num = (uint16_t*)malloc(sizeof(uint16_t));
			memset(crc16num, 0, sizeof(uint16_t));
			int calNewSize = newfileSize;
			calc_crc16sum(buffer + sizeof(testHeader), calNewSize - sizeof(testHeader), crc16num);
			testHeader.ALVChksum = *crc16num;
		}
		delete[] buffer;
	}


	reIfs.close();

	uint16_t* crc16headnum = (uint16_t*)malloc(sizeof(uint16_t));
	memset(crc16headnum, 0, sizeof(uint16_t));
	calc_crc16sum(reinterpret_cast<uint8_t*>(&testHeader) + 2, sizeof(testHeader) - 2, crc16headnum);

	testHeader.Checksum = *crc16headnum;
	ofs.seekp(0, std::ios::beg);
	ofs.write(reinterpret_cast<char*>(&testHeader), sizeof(testHeader));

	ofs.close();
	std::cout << "文件转换完成" << std::endl;
}

class A
{
public:
	A() {};
	~A() {};

	int getTem() { return tem; }

	std::list<int> getList() { return nlist; }
private:
	int tem;
	std::list<int> nlist;
};

int main(int argc, char* argv[])
{
	//outputFile("AG06_BPU0APP_V1.srec", "test.adrv");
	//return 0;

	if (argc < 2) {
		std::cerr << "Error: Missing command-line arguments. Use '-h' or '--help' for help.\n";
		return 1;
	}

	if (argc <= 3)
	{
		std::string srcFile = argv[1];
		if (srcFile == "-h" || srcFile == "--help") {
			printHelp();
		}
		else if (srcFile == "-v" || srcFile == "--version") {
			printVersion();
		}
		else if (srcFile == "-o" || srcFile == "--output") {
			std::cerr << "Error: Missing name for outputing. Use '-o path' or '--output path'.\n";
		}
		else {
			if (getFileExtension(srcFile) == "srec") {
				outputFile(srcFile, getFilenameWithoutExtension(srcFile) + ".adrv");
			}
			else {
				std::cerr << "Error: The file format is incorrect.\n";
			}
		}
		return 1;
	}

	std::string srcFile = argv[1];
	std::string arg = argv[2];
	std::string destFile = argv[3];
	std::cout << srcFile << " : " << destFile << std::endl;

	if (arg == "-h" || arg == "--help") {
		printHelp();
	}
	else if (arg == "-v" || arg == "--version") {
		printVersion();
	}
	else if (arg == "-o" || arg == "--output") {
		outputFile(srcFile, destFile);
	}
	else {
		std::cerr << "Error: Unknown option '" << arg << "'. Use '-h' or '--help' for help.\n";
		return 1;
	}

	return 0;
}