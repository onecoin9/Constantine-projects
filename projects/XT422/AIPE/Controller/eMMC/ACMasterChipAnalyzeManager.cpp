#include "ACMasterChipAnalyzeManager.h"
#include "AngKMessageHandler.h"
#include "json.hpp"
#include "ACDeviceManager.h"
#include "AngkLogger.h"
#include "AngKGlobalInstance.h"
#include "CRC/Crc32_Comm.h"
#include "Thread/ThreadPool.h"
#include <QDir>

extern Acro::Thread::ThreadPool g_ThreadPool;

int calc_crc16sum(unsigned char* buf, unsigned int size, unsigned short* pCRC16Sum);

ACMasterChipAnalyzeManager::ACMasterChipAnalyzeManager(QObject *parent)
	: QObject(parent)
{
	ResetDocument();
}

ACMasterChipAnalyzeManager::~ACMasterChipAnalyzeManager()
{
}

int ACMasterChipAnalyzeManager::ExecuteChipAnaylze(std::string devIP, uint32_t HopNum, uint32_t SKTEnable, uint32_t nPartition, uint32_t nDefaultValue, uint32_t nAnalyzeSize, uint32_t nAnalyzeGrain, uint64_t nSSDOffset)
{
	nlohmann::json chipAnaylzeJson;
	chipAnaylzeJson["AnalyzeGrain"] = nAnalyzeGrain;
	chipAnaylzeJson["AnalyzeSize"] = nAnalyzeSize;
	chipAnaylzeJson["DefaultValue"] = nDefaultValue;
	chipAnaylzeJson["PartitionEn"] = QString::number(nPartition, 16).toUpper().insert(0, "0x").toStdString();
	chipAnaylzeJson["SSDOffset"] = QString("0x%1").arg(nSSDOffset, 8, 16, QLatin1Char('0')).toUpper().toStdString();
	return AngKMessageHandler::instance().Command_RemoteDoPTCmd(devIP, HopNum, 0, 0, (uint16_t)eSubCmdID::SubCmd_MU_MasterChipAnalyze, SKTEnable, 8, QByteArray(chipAnaylzeJson.dump().c_str()));
}

int ACMasterChipAnalyzeManager::ExecuteReadDataAndSaveToFile(std::string devIP, QString strFilePath, QString Type, uint32_t HopNum, uint64_t SrcAddr, uint64_t Length)
{
	tJsonParaHash ResultHash;
	QFile file(strFilePath);//重复文件先删除，在创建覆盖
	file.remove();

	//读取文件时，对长度进行分块
	int ret = 0;
	int nRecoverTrySend = AngKGlobalInstance::instance()->ReadValue("DeviceComm", "Retransmission").toInt();
	int nSSDReadSize = AngKGlobalInstance::instance()->ReadValue("DeviceComm", "SSDReadSize").toInt();
	int nChunkSize = nSSDReadSize * 1024 * 1024; // 写入SSD每次256M,可通过配置文件修改
	//if (Length < nChunkSize) {
	//	nChunkSize = Length;
	//}
	uint64_t nFileOffset = 0;//写入文件的偏移位置
	uint64_t readAddr = SrcAddr;
	float recordCurSize = 0;
	float fProgressValue = 0;
	for (int64_t i = 0; i < Length; i += nChunkSize) {
		qint64 bytesToRead = qMin((uint64_t)nChunkSize, Length - i);
		int nTryReSend = nRecoverTrySend;
		while (nTryReSend != 0)
		{
			ret = AngKMessageHandler::instance().Command_ReadDataAndSaveToFile(devIP, strFilePath, Type, HopNum, 0, readAddr, bytesToRead, nFileOffset, ResultHash);
			if (ret == 0) {
				readAddr += bytesToRead;
				nFileOffset += bytesToRead;
				recordCurSize += bytesToRead;
				nTryReSend = nRecoverTrySend;
				fProgressValue = (static_cast<float>(recordCurSize) / Length) * 100;
				emit sgnSendWriteProgressValue(devIP, HopNum, fProgressValue);
				break;
			}
			else {
				nTryReSend--;
				QThread::msleep(100);
				ALOG_FATAL("MasterChip analyze read file : %s:%d SSD to %s failed(errorCode: %d), Try to Retrans, %d remaining times", "FP", "CU", devIP.c_str(), HopNum
					, strFilePath.toStdString().c_str(), ret, nTryReSend);
			}
		}

		if (nTryReSend <= 0 && ret != 0) {
			emit sgnSendWriteProgressValue(devIP, HopNum, 100);
			return ret;
		}
	}

	emit sgnSendWriteProgressValue(devIP, HopNum, 100);
	return ret;
}

void ACMasterChipAnalyzeManager::AppendNode(std::string nodeName, nlohmann::json& nodeInfoJson)
{
	pugi::xml_node emmcNode = m_pDocument.child("eMMC");
	if (nodeName == "Checksum") {
		pugi::xml_node checksumNode = emmcNode.child("Checksum");
		pugi::xml_node bytesumNode = checksumNode.append_child("ByteSum");

		for (int i = 0; i < nodeInfoJson.size(); ++i) {
			nlohmann::json partJson = nodeInfoJson[i];
			pugi::xml_node partNode = bytesumNode.append_child("Partition");
			partNode.append_attribute("Sum") = partJson["ByteSum"].get<std::string>().c_str();
			partNode.append_attribute("Name") = partJson["Name"].get<std::string>().c_str();
		}
		pugi::xml_node crc16Node = checksumNode.append_child("CRC16");
		crc16Node.append_attribute("Enable") = "TRUE";
	}else if (nodeName == "ChipID") {
		if (nodeInfoJson.contains("ChipID")) {
			pugi::xml_node chipIDNode = emmcNode.child(XML_NODE_EMMC_CHIP);
			std::string chipid = nodeInfoJson["ChipID"].get<std::string>();
			chipIDNode.append_attribute("Value") = chipid.c_str();
			ALOG_WARN("MasterChip AppendNode ACXML File with ChipID: %s.", "CU", "--", chipid.c_str());
		}
		else {
			ALOG_WARN("MasterChip AppendNode ACXML File without ChipID.", "CU", "--");
		}
	}
	else if (nodeName == "Partitions") {
		pugi::xml_node partitionsNode = emmcNode.child("Partitions");
		pugi::xml_node entryNode = partitionsNode.append_child("Entry");
		entryNode.append_attribute("Name") = nodeInfoJson["Name"].get<std::string>().c_str();
		entryNode.append_attribute("FileBlockPos") = nodeInfoJson["FileBlockPos"].get<std::string>().c_str();
		entryNode.append_attribute("ChipBlockPos") = nodeInfoJson["ChipBlockPos"].get<std::string>().c_str();
		entryNode.append_attribute("BlockNum") = nodeInfoJson["BlockNum"].get<std::string>().c_str();
		entryNode.append_attribute("CRC16") = nodeInfoJson["CRC16"].get<std::string>().c_str();
		entryNode.append_attribute("Partition") = nodeInfoJson["Partition"].get<std::string>().c_str();
	}

}

void ACMasterChipAnalyzeManager::AppendNode(std::string nodeName, uchar* extCSDInfo)
{
	if (nodeName == "ExtCSD") {
		pugi::xml_node emmcNode = m_pDocument.child("eMMC");
		pugi::xml_node extCSDNode = emmcNode.child("ExtCSD");
		int extSize = 512;
		bool bOK;
		
		for (int i = 0; i < extSize; ++i) {
			QString dataString;
			if (i == 168) {
				pugi::xml_node partNode = extCSDNode.append_child("PartitionSize");
				partNode.append_attribute("Name") = "RPMB";
				dataString += QString("%1").arg(extCSDInfo[i], 2, 16, QLatin1Char('0'));
				m_readExtCSD.modify_extcsd.rpmb_size_mult = extCSDInfo[i];
				partNode.append_attribute("Size") = __GetRPMBAreaSize(&m_readExtCSD.modify_extcsd);
				partNode.append_attribute("Unit") = "KBytes";
			}
			else if (i == 226) {
				pugi::xml_node partNode = extCSDNode.append_child("PartitionSize");
				partNode.append_attribute("Name") = "BOOT";
				m_readExtCSD.modify_extcsd.boot_size_mult = extCSDInfo[i];
				partNode.append_attribute("Size") = __GetBootAreaSize(&m_readExtCSD.modify_extcsd);
				partNode.append_attribute("Unit") = "KBytes";
			}
			else if (i == 143) {
				pugi::xml_node partNode = extCSDNode.append_child("PartitionSize");
				partNode.append_attribute("Name") = "GPP1";
				dataString += QString("%1").arg(extCSDInfo[i], 2, 16, QLatin1Char('0'));
				dataString += QString("%1").arg(extCSDInfo[i + 1], 2, 16, QLatin1Char('0'));
				dataString += QString("%1").arg(extCSDInfo[i + 2], 2, 16, QLatin1Char('0'));

				for (int i = 0; i < dataString.size() / 2; i++) {
					QString twoDigits = dataString.mid(i * 2, 2); // 获取两位数字的子字符串
					m_readExtCSD.modify_extcsd.gp_size_mult[i] = twoDigits.toUInt(&bOK, 16); // 将子字符串转换为整数并存储
				}
				m_readExtCSD.modify_part.GPP1Size = __GetGPPAreaSize(&m_readExtCSD.modify_extcsd, 0);
				partNode.append_attribute("Size") = m_readExtCSD.modify_part.GPP1Size;
				partNode.append_attribute("Unit") = "MBytes";
			}
			else if (i == 146) {
				pugi::xml_node partNode = extCSDNode.append_child("PartitionSize");
				partNode.append_attribute("Name") = "GPP2";
				dataString += QString("%1").arg(extCSDInfo[i], 2, 16, QLatin1Char('0'));
				dataString += QString("%1").arg(extCSDInfo[i + 1], 2, 16, QLatin1Char('0'));
				dataString += QString("%1").arg(extCSDInfo[i + 2], 2, 16, QLatin1Char('0'));

				for (int i = 0; i < dataString.size() / 2; i++) {
					QString twoDigits = dataString.mid(i * 2, 2); // 获取两位数字的子字符串
					m_readExtCSD.modify_extcsd.gp_size_mult[i] = twoDigits.toUInt(&bOK, 16); // 将子字符串转换为整数并存储
				}
				m_readExtCSD.modify_part.GPP1Size = __GetGPPAreaSize(&m_readExtCSD.modify_extcsd, 1);
				partNode.append_attribute("Size") = m_readExtCSD.modify_part.GPP2Size;
				partNode.append_attribute("Unit") = "MBytes";
			}
			else if (i == 149) {
				pugi::xml_node partNode = extCSDNode.append_child("PartitionSize");
				partNode.append_attribute("Name") = "GPP3";
				dataString += QString("%1").arg(extCSDInfo[i], 2, 16, QLatin1Char('0'));
				dataString += QString("%1").arg(extCSDInfo[i + 1], 2, 16, QLatin1Char('0'));
				dataString += QString("%1").arg(extCSDInfo[i + 2], 2, 16, QLatin1Char('0'));

				for (int i = 0; i < dataString.size() / 2; i++) {
					QString twoDigits = dataString.mid(i * 2, 2); // 获取两位数字的子字符串
					m_readExtCSD.modify_extcsd.gp_size_mult[i] = twoDigits.toUInt(&bOK, 16); // 将子字符串转换为整数并存储
				}
				m_readExtCSD.modify_part.GPP1Size = __GetGPPAreaSize(&m_readExtCSD.modify_extcsd, 2);
				partNode.append_attribute("Size") = m_readExtCSD.modify_part.GPP3Size;
				partNode.append_attribute("Unit") = "MBytes";
			}
			else if (i == 152) {
				pugi::xml_node partNode = extCSDNode.append_child("PartitionSize");
				partNode.append_attribute("Name") = "GPP4";
				dataString += QString("%1").arg(extCSDInfo[i], 2, 16, QLatin1Char('0'));
				dataString += QString("%1").arg(extCSDInfo[i + 1], 2, 16, QLatin1Char('0'));
				dataString += QString("%1").arg(extCSDInfo[i + 2], 2, 16, QLatin1Char('0'));

				for (int i = 0; i < dataString.size() / 2; i++) {
					QString twoDigits = dataString.mid(i * 2, 2); // 获取两位数字的子字符串
					m_readExtCSD.modify_extcsd.gp_size_mult[i] = twoDigits.toUInt(&bOK, 16); // 将子字符串转换为整数并存储
				}
				m_readExtCSD.modify_part.GPP1Size = __GetGPPAreaSize(&m_readExtCSD.modify_extcsd, 3);
				partNode.append_attribute("Size") = m_readExtCSD.modify_part.GPP4Size;
				partNode.append_attribute("Unit") = "MBytes";
			}
		}
		for (int i = 0; i < extSize; ++i) {
			QString dataString;
			if (i == 16) {
				pugi::xml_node regNode = extCSDNode.append_child("Reg");
				regNode.append_attribute("Addr") = i;
				dataString += QString("%1").arg((uchar)extCSDInfo[i], 2, 16, QLatin1Char('0'));
				regNode.append_attribute("Value") = dataString.toUpper().toStdString().c_str();
				regNode.append_attribute("Name") = "SEC_REMOVAL_TYPE";
				m_readExtCSD.modify_extcsd.secure_removal_type = extCSDInfo[i];
			}
			else if (i == 52) {
				pugi::xml_node regNode = extCSDNode.append_child("Reg");
				QString strAddr = QString("%1:%2").arg(i + 1).arg(i);
				regNode.append_attribute("Addr") = strAddr.toStdString().c_str();
				dataString += QString("%1").arg(extCSDInfo[i + 1], 2, 16, QLatin1Char('0'));
				dataString += QString("%1").arg(extCSDInfo[i], 2, 16, QLatin1Char('0'));
				regNode.append_attribute("Value") = dataString.toUpper().toStdString().c_str();
				regNode.append_attribute("Name") = "EXT_PARTITIONS_ATTRIBUTE";
				for (int i = 0; i < dataString.size() / 2; i++) {
					QString twoDigits = dataString.mid(i * 2, 2); // 获取两位数字的子字符串
					m_readExtCSD.modify_extcsd.ext_partitions_attribute[i] = twoDigits.toUInt(&bOK, 16); // 将子字符串转换为整数并存储
				}
			}
			else if (i == 134) {
				pugi::xml_node regNode = extCSDNode.append_child("Reg");
				regNode.append_attribute("Addr") = i;
				dataString += QString("%1").arg((uchar)extCSDInfo[i], 2, 16, QLatin1Char('0'));
				regNode.append_attribute("Value") = dataString.toUpper().toStdString().c_str();
				regNode.append_attribute("Name") = "SEC_BAD_BLK_MGMNT";
				m_readExtCSD.modify_extcsd.sec_bad_blk_mgmnt = extCSDInfo[i];
			}
			else if (i == 136) {
				pugi::xml_node regNode = extCSDNode.append_child("Reg");
				QString strAddr = QString("%1:%2").arg(i + 3).arg(i);
				regNode.append_attribute("Addr") = strAddr.toStdString().c_str();
				dataString += QString("%1").arg(extCSDInfo[i + 3], 2, 16, QLatin1Char('0'));
				dataString += QString("%1").arg(extCSDInfo[i + 2], 2, 16, QLatin1Char('0'));
				dataString += QString("%1").arg(extCSDInfo[i + 1], 2, 16, QLatin1Char('0'));
				dataString += QString("%1").arg(extCSDInfo[i], 2, 16, QLatin1Char('0'));
				regNode.append_attribute("Value") = dataString.toUpper().toStdString().c_str();
				regNode.append_attribute("Name") = "ENH_START_ADDR";
				for (int i = 0; i < dataString.size() / 2; i++) {
					QString twoDigits = dataString.mid(i * 2, 2); // 获取两位数字的子字符串
					m_readExtCSD.modify_extcsd.enh_start_addr[i] = twoDigits.toUInt(&bOK, 16); // 将子字符串转换为整数并存储
				}
			}
			else if (i == 140) {
				pugi::xml_node regNode = extCSDNode.append_child("Reg");
				QString strAddr = QString("%1:%2").arg(i + 2).arg(i);
				regNode.append_attribute("Addr") = strAddr.toStdString().c_str();
				dataString += QString("%1").arg(extCSDInfo[i + 2], 2, 16, QLatin1Char('0'));
				dataString += QString("%1").arg(extCSDInfo[i + 1], 2, 16, QLatin1Char('0'));
				dataString += QString("%1").arg(extCSDInfo[i], 2, 16, QLatin1Char('0'));
				regNode.append_attribute("Value") = dataString.toUpper().toStdString().c_str();
				regNode.append_attribute("Name") = "ENH_SIZE_MULT";
				for (int i = 0; i < dataString.size() / 2; i++) {
					QString twoDigits = dataString.mid(i * 2, 2); // 获取两位数字的子字符串
					m_readExtCSD.modify_extcsd.enh_size_mult[i] = twoDigits.toUInt(&bOK, 16); // 将子字符串转换为整数并存储
				}

				m_readExtCSD.modify_part.EnhancedUserSize = __GetEnhancedUserAreaSize(&m_readExtCSD.modify_extcsd);
			}
			else if (i == 155) {
				pugi::xml_node regNode = extCSDNode.append_child("Reg");
				regNode.append_attribute("Addr") = i;
				dataString += QString("%1").arg((uchar)extCSDInfo[i], 2, 16, QLatin1Char('0'));
				regNode.append_attribute("Value") = dataString.toUpper().toStdString().c_str();
				regNode.append_attribute("Name") = "PARTITION_SETTING_COMPLETED";
				m_readExtCSD.modify_extcsd.partition_setting_completed = extCSDInfo[i];
			}
			else if (i == 156) {
				pugi::xml_node regNode = extCSDNode.append_child("Reg");
				regNode.append_attribute("Addr") = i;
				dataString += QString("%1").arg((uchar)extCSDInfo[i], 2, 16, QLatin1Char('0'));
				regNode.append_attribute("Value") = dataString.toUpper().toStdString().c_str();
				regNode.append_attribute("Name") = "PARTITIONS_ATTRIBUTE";
				m_readExtCSD.modify_extcsd.partitions_attribute = extCSDInfo[i];
			}
			else if (i == 162) {
				pugi::xml_node regNode = extCSDNode.append_child("Reg");
				regNode.append_attribute("Addr") = i;
				dataString += QString("%1").arg((uchar)extCSDInfo[i], 2, 16, QLatin1Char('0'));
				regNode.append_attribute("Value") = dataString.toUpper().toStdString().c_str();
				regNode.append_attribute("Name") = "RST_n_FUNCTION";
				m_readExtCSD.modify_extcsd.rst_n_function = extCSDInfo[i];
			}
			else if (i == 163) {
				pugi::xml_node regNode = extCSDNode.append_child("Reg");
				regNode.append_attribute("Addr") = i;
				dataString += QString("%1").arg((uchar)extCSDInfo[i], 2, 16, QLatin1Char('0'));
				regNode.append_attribute("Value") = dataString.toUpper().toStdString().c_str();
				regNode.append_attribute("Name") = "BKOPS_EN";
				m_readExtCSD.modify_extcsd.bkops_en = extCSDInfo[i];
			}
			else if (i == 167) {
				pugi::xml_node regNode = extCSDNode.append_child("Reg");
				regNode.append_attribute("Addr") = i;
				dataString += QString("%1").arg((uchar)extCSDInfo[i], 2, 16, QLatin1Char('0'));
				regNode.append_attribute("Value") = dataString.toUpper().toStdString().c_str();
				regNode.append_attribute("Name") = "WR_REL_SET";
				m_readExtCSD.modify_extcsd.wr_rel_set = extCSDInfo[i];
			} 
			else if (i == 169) {
				pugi::xml_node regNode = extCSDNode.append_child("Reg");
				regNode.append_attribute("Addr") = i;
				dataString += QString("%1").arg((uchar)extCSDInfo[i], 2, 16, QLatin1Char('0'));
				regNode.append_attribute("Value") = dataString.toUpper().toStdString().c_str();
				regNode.append_attribute("Name") = "FW_CONFIG";
				m_readExtCSD.modify_extcsd.fw_config = extCSDInfo[i];
			}
			else if (i == 171) {
				pugi::xml_node regNode = extCSDNode.append_child("Reg");
				regNode.append_attribute("Addr") = i;
				dataString += QString("%1").arg((uchar)extCSDInfo[i], 2, 16, QLatin1Char('0'));
				regNode.append_attribute("Value") = dataString.toUpper().toStdString().c_str();
				regNode.append_attribute("Name") = "USER_WP";
				m_readExtCSD.modify_extcsd.user_wp = extCSDInfo[i];
			}
			else if (i == 173) {
				pugi::xml_node regNode = extCSDNode.append_child("Reg");
				regNode.append_attribute("Addr") = i;
				dataString += QString("%1").arg((uchar)extCSDInfo[i], 2, 16, QLatin1Char('0'));
				regNode.append_attribute("Value") = dataString.toUpper().toStdString().c_str();
				regNode.append_attribute("Name") = "BOOT_WP";
				m_readExtCSD.modify_extcsd.boot_wp = extCSDInfo[i];
			}
			else if (i == 175) {
				pugi::xml_node regNode = extCSDNode.append_child("Reg");
				regNode.append_attribute("Addr") = i;
				dataString += QString("%1").arg((uchar)extCSDInfo[i], 2, 16, QLatin1Char('0'));
				regNode.append_attribute("Value") = dataString.toUpper().toStdString().c_str();
				regNode.append_attribute("Name") = "ERASE_GROUP_DEF";
				m_readExtCSD.modify_extcsd.erase_group_def = extCSDInfo[i];
			}
			else if (i == 177) {
				pugi::xml_node regNode = extCSDNode.append_child("Reg");
				regNode.append_attribute("Addr") = i;
				dataString += QString("%1").arg((uchar)extCSDInfo[i], 2, 16, QLatin1Char('0'));
				regNode.append_attribute("Value") = dataString.toUpper().toStdString().c_str();
				regNode.append_attribute("Name") = "BOOT_BUS_CONDITIONS";
				m_readExtCSD.modify_extcsd.boot_bus_conditions = extCSDInfo[i];
			}
			else if (i == 178) {
				pugi::xml_node regNode = extCSDNode.append_child("Reg");
				regNode.append_attribute("Addr") = i;
				dataString += QString("%1").arg((uchar)extCSDInfo[i], 2, 16, QLatin1Char('0'));
				regNode.append_attribute("Value") = dataString.toUpper().toStdString().c_str();
				regNode.append_attribute("Name") = "BOOT_CONFIG_PROT";
				m_readExtCSD.modify_extcsd.boot_config_prot = extCSDInfo[i];
			}
			else if (i == 179) {
				pugi::xml_node regNode = extCSDNode.append_child("Reg");
				regNode.append_attribute("Addr") = i;
				dataString += QString("%1").arg((uchar)extCSDInfo[i], 2, 16, QLatin1Char('0'));
				regNode.append_attribute("Value") = dataString.toUpper().toStdString().c_str();
				regNode.append_attribute("Name") = "PARTITION_CONFIG";
				m_readExtCSD.modify_extcsd.partition_config = extCSDInfo[i];
			}
			else if (i == 212) {
				pugi::xml_node regNode = extCSDNode.append_child("Reg");
				QString strAddr = QString("%1:%2").arg(i + 3).arg(i);
				regNode.append_attribute("Addr") = strAddr.toStdString().c_str();
				dataString += QString("%1").arg(extCSDInfo[i + 3], 2, 16, QLatin1Char('0'));
				dataString += QString("%1").arg(extCSDInfo[i + 2], 2, 16, QLatin1Char('0'));
				dataString += QString("%1").arg(extCSDInfo[i + 1], 2, 16, QLatin1Char('0'));
				dataString += QString("%1").arg(extCSDInfo[i], 2, 16, QLatin1Char('0'));
				regNode.append_attribute("Value") = dataString.toUpper().toStdString().c_str();
				regNode.append_attribute("Name") = "SEC_COUNT";

				m_readExtCSD.modify_extcsd.sec_count = dataString.toUInt(&bOK, 16); // 将子字符串转换为整数并存储
				m_readExtCSD.modify_part.UserSize = __GetUserAreaSize(&m_readExtCSD.modify_extcsd);
			}
		}
	}
}

void ACMasterChipAnalyzeManager::SaveACXMLFile(QString& strFilePath)
{
	QFile file(strFilePath);//重复文件先删除，在创建覆盖
	file.remove();
	if (!m_pDocument.save_file(reinterpret_cast<const wchar_t*>(strFilePath.utf16()), "\t", pugi::format_default, pugi::encoding_utf8)) {
		ALOG_ERROR("MasterChip analyze save ACXML File %s failed.", "CU", "--", strFilePath.toStdString().c_str());
	}
	else
		ALOG_INFO("MasterChip analyze save ACXML File %s successfully.", "CU", "--", strFilePath.toStdString().c_str());
}

void ACMasterChipAnalyzeManager::GetExtCSDInfo(std::string devIP, uint32_t HopNum, uint32_t uExtSize, QString& strDDROffset, uint32_t uCrc16, QString& extLog)
{
	bool bOk = false;
	QByteArray readExtCSDByte;
	DeviceStu devInfo = ACDeviceManager::instance().getDevInfo(QString::fromStdString(devIP), HopNum);
	int nRecoverTrySend = AngKGlobalInstance::instance()->ReadValue("DeviceComm", "Retransmission").toInt();
	int nTryReSend = nRecoverTrySend;
	int ret = 0;
	while (nTryReSend != 0)
	{
		ret = AngKMessageHandler::instance().Command_ReadDataFromSSD(devIP, "DDR2FIBER", HopNum, 0, strDDROffset.toInt(&bOk, 16), uExtSize, readExtCSDByte, devInfo.tMainBoardInfo.strHardwareSN, "extCSDData");
		if (ret == 0) {
			ALOG_INFO("MasterChip analyze getExtInfo complete.", "CU", "--");
			break;
		}
		else {
			nTryReSend--;
			ALOG_FATAL("MasterChip analyze getExtInfo failed(errcode=%d).", "CU", "--", ret);
		}
	}

	if (nTryReSend <= 0 && ret != 0) {
		return;
	}
	ushort calCRC16 = 0;
	calc_crc16sum((uchar*)readExtCSDByte.constData(), 512, &calCRC16);

	if (calCRC16 != uCrc16) {
		ALOG_FATAL("MasterChip analyze getExtInfo failed : calculate CRC16 different", "CU", "--");
		return;
	}

	uchar* extCSDInfo = new uchar[512];

	// 初始化所有位为 0
	memset(extCSDInfo, 0, 512);
	memcpy(extCSDInfo, readExtCSDByte.constData(), 512);
	AppendNode("ExtCSD", extCSDInfo);
	delete[] extCSDInfo;

	RecordExtCSDInfo(extLog);
}

void ACMasterChipAnalyzeManager::RecordExtCSDInfo(QString& extcsdInfo)
{
	extcsdInfo += QString("ExtCSD[16] SECURE_REMOVAL_TYPE:%1\n").arg(m_readExtCSD.modify_extcsd.secure_removal_type, 2, 16, QLatin1Char('0')).toUpper();
	extcsdInfo += QString("ExtCSD[53:52] EXT_PARTITIONS_ATTRIBUTE:%1\n").arg(*(uint16_t*)m_readExtCSD.modify_extcsd.ext_partitions_attribute, 4, 16, QLatin1Char('0')).toUpper();
	extcsdInfo += QString("ExtCSD[134] SEC_BAD_BLK_MGMNT:%1\n").arg(m_readExtCSD.modify_extcsd.sec_bad_blk_mgmnt, 2, 16, QLatin1Char('0')).toUpper();
	extcsdInfo += QString("ExtCSD[139:136] ENH_START_ADDR:");
	for (int i = 0; i < 4; i++) {
		extcsdInfo += QString("%1").arg(m_readExtCSD.modify_extcsd.enh_start_addr[3 - i], 2, 16, QLatin1Char('0')).toUpper();
	}
	extcsdInfo += "\n";
	extcsdInfo += QString("ExtCSD[142:140] ENH_SIZE_MULT:");
	for (int i = 0; i < 3; i++) {
		extcsdInfo += QString("%1").arg(m_readExtCSD.modify_extcsd.enh_size_mult[2 - i], 2, 16, QLatin1Char('0')).toUpper();
	}
	extcsdInfo += "\n";
	extcsdInfo += QString("ExtCSD[145:143] GPP1_SIZE_MULT:");
	for (int i = 0; i < 3; i++) {
		extcsdInfo += QString("%1").arg(m_readExtCSD.modify_extcsd.gp_size_mult[2 - i], 2, 16, QLatin1Char('0')).toUpper();
	}
	extcsdInfo += "\n";
	extcsdInfo += QString("ExtCSD[148:146] GPP2_SIZE_MULT:");
	for (int i = 0; i < 3; i++) {
		extcsdInfo += QString("%1").arg(m_readExtCSD.modify_extcsd.gp_size_mult[5 - i], 2, 16, QLatin1Char('0')).toUpper();
	}
	extcsdInfo += "\n";
	extcsdInfo += QString("ExtCSD[151:149] GPP3_SIZE_MULT:");
	for (int i = 0; i < 3; i++) {
		extcsdInfo += QString("%1").arg(m_readExtCSD.modify_extcsd.gp_size_mult[8 - i], 2, 16, QLatin1Char('0')).toUpper();
	}
	extcsdInfo += "\n";
	extcsdInfo += QString("ExtCSD[154:152] GPP4_SIZE_MULT:");
	for (int i = 0; i < 3; i++) {
		extcsdInfo += QString("%1").arg(m_readExtCSD.modify_extcsd.gp_size_mult[11 - i], 2, 16, QLatin1Char('0')).toUpper();
	}
	extcsdInfo += "\n";

	extcsdInfo += QString("ExtCSD[155] PARTITION_SETTING_COMPLETED:%1\n").arg(m_readExtCSD.modify_extcsd.partition_setting_completed, 2, 16, QLatin1Char('0')).toUpper();
	extcsdInfo += QString("ExtCSD[156] PARTITION_ATTRIBUTE:%1\n").arg(m_readExtCSD.modify_extcsd.partitions_attribute, 2, 16, QLatin1Char('0')).toUpper();
	extcsdInfo += QString("ExtCSD[162] RST_n_FUNCTION:%1\n").arg(m_readExtCSD.modify_extcsd.rst_n_function, 2, 16, QLatin1Char('0')).toUpper();
	extcsdInfo += QString("ExtCSD[163] BKOPS_EN:%1\n").arg(m_readExtCSD.modify_extcsd.bkops_en, 2, 16, QLatin1Char('0')).toUpper();
	extcsdInfo += QString("ExtCSD[167] WR_REL_SET:%1\n").arg(m_readExtCSD.modify_extcsd.wr_rel_set, 2, 16, QLatin1Char('0')).toUpper();
	extcsdInfo += QString("ExtCSD[168] RPMB_SIZE_MULT: %1\n").arg(m_readExtCSD.modify_extcsd.rpmb_size_mult, 2, 16, QLatin1Char('0')).toUpper();
	extcsdInfo += QString("ExtCSD[169] FW_CONFIG:%1\n").arg(m_readExtCSD.modify_extcsd.fw_config, 2, 16, QLatin1Char('0')).toUpper();
	extcsdInfo += QString("ExtCSD[171] USER_WP:%1\n").arg(m_readExtCSD.modify_extcsd.user_wp, 2, 16, QLatin1Char('0')).toUpper();
	extcsdInfo += QString("ExtCSD[173] BOOT_WP:%1\n").arg(m_readExtCSD.modify_extcsd.boot_wp, 2, 16, QLatin1Char('0')).toUpper();
	extcsdInfo += QString("ExtCSD[175] ERASE_GROUP_DEF:%1\n").arg(m_readExtCSD.modify_extcsd.erase_group_def, 2, 16, QLatin1Char('0')).toUpper();
	extcsdInfo += QString("ExtCSD[177] BOOT_BUS_CONDITIONS:%1\n").arg(m_readExtCSD.modify_extcsd.boot_bus_conditions, 2, 16, QLatin1Char('0')).toUpper();
	extcsdInfo += QString("ExtCSD[178] BOOT_CONFIG_PORT:%1\n").arg(m_readExtCSD.modify_extcsd.boot_config_prot, 2, 16, QLatin1Char('0')).toUpper();
	extcsdInfo += QString("ExtCSD[179] PARTITION_CONFIG:%1\n").arg(m_readExtCSD.modify_extcsd.partition_config, 2, 16, QLatin1Char('0')).toUpper();
	extcsdInfo += QString("ExtCSD[226] BOOT_SIZE_MULT:%1").arg(m_readExtCSD.modify_extcsd.boot_size_mult, 2, 16, QLatin1Char('0')).toUpper();
}

void ACMasterChipAnalyzeManager::ResetDocument()
{
	m_pDocument.reset();
	pugi::xml_node emmcNode = m_pDocument.append_child("eMMC");
	pugi::xml_node chipIDNode = emmcNode.append_child(XML_NODE_EMMC_CHIP);
	pugi::xml_node versionNode = emmcNode.append_child(XML_NODE_EMMC_VERSION);
	versionNode.text() = XML_EMMC_VERSION;
	pugi::xml_node checksumNode = emmcNode.append_child("Checksum");
	pugi::xml_node partitionsNode = emmcNode.append_child("Partitions");
	pugi::xml_node extCSDNode = emmcNode.append_child("ExtCSD");
}

uint32_t ACMasterChipAnalyzeManager::__GetEnhancedUserAreaSize(UI_CFG_EXTCSD* pRawExtCSD)
{
	if (pRawExtCSD->hc_erase_grp_size == 0 || pRawExtCSD->hc_wp_grp_size == 0) {
		return 0;
	}
	else {
		return ((pRawExtCSD->enh_size_mult[2] << 16) + (pRawExtCSD->enh_size_mult[1] << 8) + pRawExtCSD->enh_size_mult[0]) * pRawExtCSD->hc_wp_grp_size * pRawExtCSD->hc_erase_grp_size / 2;
	}
}

uint32_t ACMasterChipAnalyzeManager::__GetUserAreaSize(UI_CFG_EXTCSD* pRawExtCSD)
{
	return pRawExtCSD->sec_count / 2048;
}

uint32_t ACMasterChipAnalyzeManager::__GetGPPAreaSize(UI_CFG_EXTCSD* pRawExtCSD, int idx)
{
	if (pRawExtCSD->hc_erase_grp_size == 0 || pRawExtCSD->hc_wp_grp_size == 0) {
		return 0;
	}
	else {
		return ((pRawExtCSD->gp_size_mult[idx * 3 + 2] << 16) + (pRawExtCSD->gp_size_mult[idx * 3 + 1] << 8) + pRawExtCSD->gp_size_mult[idx * 3 + 0]) * pRawExtCSD->hc_wp_grp_size * pRawExtCSD->hc_erase_grp_size / 2;
	}
}

uint32_t ACMasterChipAnalyzeManager::__GetBootAreaSize(UI_CFG_EXTCSD* pRawExtCSD)
{
	return pRawExtCSD->boot_size_mult * 128;
}

uint32_t ACMasterChipAnalyzeManager::__GetRPMBAreaSize(UI_CFG_EXTCSD* pRawExtCSD)
{
	return pRawExtCSD->rpmb_size_mult * 128;
}
