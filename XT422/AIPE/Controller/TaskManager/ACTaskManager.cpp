#include "ACTaskManager.h"
#include "AngkLogger.h"
#include "pugixml.hpp"
#include "ACDeviceManager.h"
#include "ACMessageBox.h"
#include <sstream>  
#include <QCryptographicHash>

int calc_crc16sum(unsigned char* buf, unsigned int size, unsigned short* pCRC16Sum);

ACTaskManager::ACTaskManager(QObject *parent)
	: QObject(parent)
	, m_curOpenTask("")
{
}

ACTaskManager::~ACTaskManager()
{
}

int ACTaskManager::CreateTask(std::unique_ptr<QStandardItemModel>& _taskTable, QString _saveTaskPath)
{
	pugi::xml_document doc;

	pugi::xml_node root = doc.append_child("Task");
	for (int i = 0; i < _taskTable->rowCount(); ++i) {
		if (i == _taskTable->rowCount() - 1)
			continue;

		QString projPath = _taskTable->data(_taskTable->index(i, 0)).toString();
		QString devBindInfo = _taskTable->data(_taskTable->index(i, 1)).toString();
		QString doubleAdp = _taskTable->data(_taskTable->index(i, 2)).toString();
		QString AdpValue = _taskTable->data(_taskTable->index(i, 3)).toString();

		if (projPath.isEmpty()) {
			return XMLMESSAGE_PROJ_EMPTY_FAILED;
		}

		pugi::xml_node projectNode = root.append_child("Project");
		projectNode.append_attribute("BindProg").set_value(devBindInfo.toStdString().c_str());
		projectNode.append_attribute("AdapterType").set_value(doubleAdp.toStdString().c_str());
		projectNode.append_attribute("AdapterUse").set_value(AdpValue.toStdString().c_str());

		QString tmpCRC16 = calculateFileCRC16(projPath).toLocal8Bit().data();
		if (tmpCRC16.isEmpty()) {
			ALOG_ERROR("calculate \"%s\" CRC16 error", "CU", "--", projPath.toLocal8Bit().data());
			return XMLMESSAGE_CRC_FAILED;
		}

		projectNode.append_attribute("CRC16").set_value(tmpCRC16.toLocal8Bit().data());

		projectNode.text().set(Utils::AngKCommonTools::Full2RelativePath(projPath, _saveTaskPath).c_str());
	}


	std::ostringstream oss;
	root.print(oss, "\t");
	std::string xmlString = oss.str();

	unsigned short crc16Sum = 0;
	calc_crc16sum((unsigned char*)xmlString.c_str(), xmlString.size(), &crc16Sum);

	QString crc16Str = QString::number(crc16Sum);
	pugi::xml_node md5Node = doc.append_child(XML_NODE_CRC16);
	md5Node.text().set(crc16Str.toLocal8Bit().data());

	const wchar_t* encodedName = reinterpret_cast<const wchar_t*>(_saveTaskPath.utf16());
	if (!doc.save_file(encodedName, "\t", pugi::format_default, pugi::encoding_utf8)) {
		ALOG_ERROR("%s", "CU", "--", "Failed to save task xml document to file.");
		return XMLMESSAGE_LOAD_FAILED;
	}

	return XMLMESSAGE_SUCCESS;
}

int ACTaskManager::OpenTask(QString _taskPath, std::vector<TaskProperty>& _vecProp)
{
	pugi::xml_document doc;
	pugi::xml_node task_node;
	const wchar_t* encodedName = reinterpret_cast<const wchar_t*>(_taskPath.utf16());
	pugi::xml_parse_result result = doc.load_file(encodedName);

	if (!result) {
		ALOG_FATAL("Open task file error.", "CU", "--");
		return result;
	}

	task_node = doc.child(XML_NODE_TASK);
	std::ostringstream oss;
	task_node.print(oss, "\t");
	std::string xmlString = oss.str();

	unsigned short crc16Sum = 0;
	calc_crc16sum((unsigned char*)xmlString.c_str(), xmlString.size(), &crc16Sum);

	QString crc16Str = QString::number(crc16Sum);
	
	if (crc16Str != doc.child(XML_NODE_CRC16).text().as_string()) {
		//emit sgnErrorMessage("CRC Error", tr("Please check if file \"%1\" has been modified").arg(_taskPath));
		ALOG_FATAL("Task file \"%s\" CRC error.", "CU", "--", _taskPath.toLocal8Bit().constData());
		return -1;
	}


	pugi::xml_node taskProj_node = task_node.child(XML_NODE_TASK_PROJECT);
	int projCnt = std::distance(task_node.begin(), task_node.end());

	bool errFlag = false;
	for (int i = 0; i < projCnt; ++i) {
		TaskProperty taskProp;
		taskProp.taskPath = Utils::AngKCommonTools::Relative2FullPath(taskProj_node.text().as_string(), _taskPath);
		taskProp.devBindInfo = taskProj_node.attribute("BindProg").as_string();
		taskProp.doubleCheck = taskProj_node.attribute("AdapterType").as_string();
		taskProp.adpValue = taskProj_node.attribute("AdapterUse").as_string();
		taskProp.crc16Str = taskProj_node.attribute("CRC16").as_string();

		if (taskProp.crc16Str != calculateFileCRC16(taskProp.taskPath.c_str()).toStdString()) {
			//emit sgnErrorMessage("CRC Error", tr("Please check if file \"%1\" has been modified.").arg(taskProp.taskPath.c_str()));
			ALOG_FATAL("Check project: \"%s\" CRC error.", "CU", "--", taskProp.taskPath.c_str());
			errFlag = true;
		}

		taskProj_node = taskProj_node.next_sibling();
		_vecProp.push_back(taskProp);
	}

	return errFlag ? -1 : XMLMESSAGE_SUCCESS;
}

int ACTaskManager::LoadTask(QString _taskPath, std::map<QString, int>& _DevResult)
{
	pugi::xml_document doc;
	pugi::xml_node task_node;
	const wchar_t* encodedName = reinterpret_cast<const wchar_t*>(_taskPath.utf16());
	pugi::xml_parse_result result = doc.load_file(encodedName);

	if (!result) {
		ALOG_FATAL("Open task file error.", "CU", "--");
		return -1;
	}

	int retLoad = 0;
	task_node = doc.child(XML_NODE_TASK);

	// 校验CRC16
	std::ostringstream oss;
	task_node.print(oss, "\t");
	std::string xmlString = oss.str();
	unsigned short crc16Sum = 0;
	calc_crc16sum((unsigned char*)xmlString.c_str(), xmlString.size(), &crc16Sum);
	QString crc16Str = QString::number(crc16Sum);
	if (crc16Str != doc.child(XML_NODE_CRC16).text().as_string()) {
		//emit sgnErrorMessage("CRC Error", tr("Please check if file \"%1\" has been modified").arg(_taskPath));
		ALOG_FATAL("Task file: \"%s\" CRC error.", "CU", "--", _taskPath.toLocal8Bit().constData());
		return E_TAG_CHECKSUM;
	}


	pugi::xml_node taskProj_node = task_node.child(XML_NODE_TASK_PROJECT);
	int projCnt = std::distance(task_node.begin(), task_node.end());

	ALOG_INFO("Loading Task : %s", "CU", "--", _taskPath.toStdString().c_str());

	for (int i = 0; i < projCnt; ++i) {

		ACProjManager* progMgr = new ACProjManager(this);
		QString projPath = QString::fromStdString(Utils::AngKCommonTools::Relative2FullPath(taskProj_node.text().as_string(), _taskPath));

		QString adapterUse = taskProj_node.attribute("AdapterUse").as_string();
		QString bindProg = taskProj_node.attribute("BindProg").as_string();

		int bBindCount = 0;
		for (auto& devRet : _DevResult) {	//校验Task与序列号和别名是否绑定
			QStringList strIPHopList = devRet.first.split(":");
			DeviceStu tmpDev = ACDeviceManager::instance().getDevInfo(strIPHopList[0], strIPHopList[1].toInt());
			if (bindProg.contains("SN-")) {
				QStringList progList = bindProg.split("SN-");
				QString _bindSN = QString::fromStdString(tmpDev.tMainBoardInfo.strHardwareSN);
				if (progList.size() > 1 && progList[1] != _bindSN) {
					ALOG_FATAL("Device SN:%s and Task SN:%s do not match.", "CU", "--", _bindSN.toStdString().c_str(), progList[1].toStdString().c_str());
					emit sgnErrorMessage("Task Error", tr("Device SN:%1 and Task SN:%2 do not match.").arg(_bindSN).arg(progList[1]));
					devRet.second = 2;
					bBindCount++;
				}
			}
			else if (bindProg.contains("Alias-")) {
				QStringList progList = bindProg.split("Alias-");
				QString _bindAlias = QString::fromStdString(tmpDev.strSiteAlias);
				if (progList.size() > 1 && progList[1] != _bindAlias) {
					ALOG_FATAL("Device Alias:%s and Task Alias:%s do not match.", "CU", "--", _bindAlias.toStdString().c_str(), progList[1].toStdString().c_str());
					emit sgnErrorMessage("Task Error", tr("Device Alias:%1 and Task Alias:%2 do not match.").arg(_bindAlias).arg(progList[1]));
					devRet.second = 2;
					bBindCount++;
				}
			}
		}
		if (bBindCount == _DevResult.size()) {
			return E_TAG_EXIST;
		}
		// 校验eapr CRC16
		QString eaprCRC16 = calculateFileCRC16(projPath);
		if (eaprCRC16 != QString(taskProj_node.attribute("CRC16").as_string())) {
			emit sgnErrorMessage("CRC Error", tr("Please check if file \"%1\" has been modified.").arg(projPath));
			ALOG_FATAL("Check project file: \"%s\" CRC error.", "CU", "--", projPath.toLocal8Bit().data());
			return E_TAG_CHECKSUM;
		}

		bool bOk;
		int nAdapterUse = adapterUse.toInt(&bOk, 16);
		QString bindBPU;
		for (int j = 0; j < 8; ++j) {
			if ((nAdapterUse & (3 << (j * 2)))) {
				bindBPU += QString("BPU%1,").arg(j);
			}
		}
		if (bindBPU.endsWith(",")) {
			// 删除最后一个字符
			bindBPU.chop(1);
		}

		ALOG_INFO("Task Bind Device BPU Include : %s", "CU", "--", bindBPU.toStdString().c_str());
		retLoad = progMgr->ImportProject(projPath);
		if (retLoad != 0) {
			progMgr->ErrorClearFile();
			break;
		}

		projAdp bindProj(adapterUse, progMgr);
		m_mapBindProjMgr[projPath] = bindProj;

		taskProj_node = taskProj_node.next_sibling();
	}

	if (retLoad == 0) {
		m_curOpenTask = _taskPath;
	}

	return retLoad;
}

projAdp& ACTaskManager::GetProjInfo(QString _projName)
{
	return m_mapBindProjMgr[_projName];
}

QMap<QString, QPair<QString, ACProjManager*>>& ACTaskManager::GetAllProjInfo()
{
	return m_mapBindProjMgr;
}


QString ACTaskManager::calculateFileCRC16(const QString& fileName) {
	QFile file(fileName);

	if (!file.open(QIODevice::ReadOnly)) {
		ALOG_FATAL("Open \"%s\" error.", "CU", "--", fileName.toLocal8Bit().data());
		return QString();
	}

	auto fileBytes = file.readAll();

	unsigned short crc16Sum = 0;
	calc_crc16sum((unsigned char*)fileBytes.data(), fileBytes.size(), &crc16Sum);

	return QString::number(crc16Sum);
}