#ifndef __DECXML_H__
#define __DECXML_H__

#include <string>
#include <list>
#include "../global.h"

#define MAX_OUTPUT_RANGE_NUM 10
struct DCPS_xml_info
{
	char manufacturer[20];
	char product[20];
	char id[20];

	char outPutRangeListStr[MAX_OUTPUT_RANGE_NUM][20];
};

struct SMU_xml_info
{
};

union dev_xml_info
{
	dev_xml_info() {
		memset(&dcps_info, 0, sizeof(DCPS_xml_info));
	}
	DCPS_xml_info dcps_info;
	SMU_xml_info smu_info;
};

class DevXmlHandler {

public:
	DevXmlHandler(const std::string& path);
	virtual ~DevXmlHandler();

	virtual bool addDevInfo(const dev_xml_info& devinfo) { return false; };
	virtual dev_xml_info getDevInfo(const std::string& product, bool* ok = nullptr) { return dev_xml_info(); };
	bool crearXmlFile();
	
	
	const std::string getXmlPath() const { return m_strXmlPath; }


	std::string getDevOperation(const std::string& product, const std::string& cmd);
private:
	std::string m_strXmlPath;
};

class DCPSXmlHandler : public DevXmlHandler {

#define DCPS_XML_FILE_PATH (getLibraryPath() + "/../xmlDir/DCPS.xml")
public:
	DCPSXmlHandler(const std::string& path = DCPS_XML_FILE_PATH);
	~DCPSXmlHandler();

	bool addDevInfo(const dev_xml_info& devinfo) override;
	dev_xml_info getDevInfo(const std::string& product, bool* ok = nullptr) override;

};

class SMUXmlHandler : public DevXmlHandler {

#define SMU_XML_FILE_PATH (getLibraryPath() + "/../xmlDir/SMU.xml")
public:
	SMUXmlHandler(const std::string& path = SMU_XML_FILE_PATH);
	~SMUXmlHandler();
};


class DMXmlHandler : public DevXmlHandler {

#define DM_XML_FILE_PATH (getLibraryPath() + "/../xmlDir/DM.xml")
public:
	DMXmlHandler(const std::string& path = DM_XML_FILE_PATH);
	~DMXmlHandler();
};

class FGXmlHandler : public DevXmlHandler {

#define FG_XML_FILE_PATH (getLibraryPath() + "/../xmlDir/FG.xml")
public:
	FGXmlHandler(const std::string& path = FG_XML_FILE_PATH);
	~FGXmlHandler();
};



#endif // !__DECXML_H__

