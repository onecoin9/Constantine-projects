#include "./include/xml/tinyxml.h"
#include <iostream>
#include <windows.h>

#pragma comment(lib, "tinyxml.lib")


static bool bFind = false;

std::string postbuildStep;


// 读取 INI 文件中指定的键值对
std::string readIniValue(const std::string& iniFile, const std::string& section, const std::string& key) {
	char buffer[512]; // 缓冲区
	GetPrivateProfileStringA(LPCSTR(section.c_str()), LPCSTR(key.c_str()), LPCSTR(""), buffer, 512, LPCSTR(iniFile.c_str()));
	return std::string(buffer);
}



void TraverseNode(TiXmlElement* element, int depth = 0) {
	if (element == nullptr) return;

	// 打印当前节点的名称和缩进  
	//for (int i = 0; i < depth; ++i) std::cout << "  ";
	//std::cout << "Node: " << element->Value() << std::endl;


	if (strcmp(element->Value(), "configuration") == 0 && element->Attribute("artifactExtension") != nullptr && strcmp(element->Attribute("artifactExtension"), "elf") == 0 &&
		element->Attribute("artifactExtension") != nullptr && (strcmp(element->Attribute("name"), "Release") == 0 || strcmp(element->Attribute("name"), "Debug") == 0)) 
	{
		element->SetAttribute("postbuildStep", postbuildStep.c_str());
		bFind = true;
	}

	// 遍历所有子节点  
	for (TiXmlElement* child = element->FirstChildElement(); child != nullptr; child = child->NextSiblingElement()) {
		TraverseNode(child, depth + 1);
	}
}


#define ERR_RETURN {std::cout << "Project configuration failed!" << std::endl;std::cout << "========  Project Configuration End  ========" << std::endl;return -1;}

int main(int argc, char* argv[])
{
	std::cout << "========  Project Configuration Start  ========" << std::endl;
	std::string path = "";
	for (int i = 1; i < argc; ++i) {
		std::string arg = argv[i];
		if (arg == "-p" && i + 1 < argc) {
			path = argv[i + 1];
			++i;
			break;
		}
	}

	if (path.empty())
	{
		std::cerr << "Usage: " << argv[0] << " -p <path>" << std::endl;
		ERR_RETURN;
	}
	path += "/.cproject";


	while (path.find('\\') != std::string::npos)
	{
		path.replace(path.find('\\'), 1, "/");
	}



	char exePath[MAX_PATH];
	GetModuleFileNameA(NULL, exePath, MAX_PATH);

	// 提取目录  
	char* last_backslash = strrchr(exePath, '\\');
	if (last_backslash) {
		*last_backslash = '\0';
	}

	strcat_s(exePath, "\\config.ini");
	std::cout << "Read config.ini from: " << exePath << std::endl;


	postbuildStep = readIniValue(exePath, "projectConfig", "postbuildStep");
	if (postbuildStep.empty()) {
		std::cerr << "Get postbuildStep failed!" << std::endl;
		ERR_RETURN;
	}

	TiXmlDocument doc;
	if (!doc.LoadFile(path.c_str()))
	{
		std::cerr << doc.ErrorDesc() << std::endl;
		ERR_RETURN;
	}
	

	TiXmlElement* root = doc.RootElement();
	if (root == nullptr) {
		std::cerr << "Get root element failed!" << std::endl;
		ERR_RETURN;
	}

	TraverseNode(root);


	if (!bFind) {
		std::cerr << "Not find elf file!" << std::endl;
		ERR_RETURN;
	}

	doc.SaveFile(path.c_str());

	std::cout << "Project configuration successful" << std::endl;
	std::cout << "Set .cproject postbuildStep = " << postbuildStep << std::endl;
	std::cout << "========  Project Configuration end  ========" << std::endl;

	return 0;


	
}