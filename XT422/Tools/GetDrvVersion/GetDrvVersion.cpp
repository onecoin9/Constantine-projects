// GetDrvVersion.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <chrono>
#include <cstdio>  
#include <stdexcept>  
#include <string>  
#include <array>  

#pragma warning(disable :4996)


std::string GetDate()
{
    //std::string date;
    auto t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::stringstream ss;
    ss << std::put_time(localtime(&t), "%Y-%m-%d:%H:%M:%S");
    return ss.str();
}

std::string getSvnVersion() {
    std::string command = "svn info";
    // 打开管道执行命令  
    std::array<char, 128> buffer;
    std::string result;
    FILE* pipe = _popen(command.c_str(), "r");
    if (!pipe) {
        throw std::runtime_error("_popen() failed!");
    }

    // 读取命令输出  
    try {
        while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
            result += buffer.data();
        }
    }
    catch (...) {
        _pclose(pipe);
        throw;
    }

    // 关闭管道  
    _pclose(pipe);


    size_t verStrIndex, verValIndex, endLineIndex;
    std::string svnVersion;

    if (result.length() == 0) {
        goto read_svn_err;
    }

    verStrIndex = result.find("Revision");
    if (verStrIndex == result.npos)
        goto read_svn_err;

    verValIndex = result.find(':', verStrIndex);
    if (verValIndex == result.npos)
        goto read_svn_err;

    while (++verValIndex < result.length())
    {
        if (result[verValIndex] != '\n' && result[verValIndex] != '\t' && result[verValIndex] != ' ' && result[verValIndex] != '\r')
            break;
    }

    endLineIndex = result.find('\n', verValIndex);
    if (endLineIndex == result.npos)
        endLineIndex = result.length();

    svnVersion = result.substr(verValIndex, endLineIndex - verValIndex);
    std::cout << "===SVN Version:" << svnVersion << std::endl;

    return svnVersion;

read_svn_err:
    std::cout << "Read SVN version failed!" << std::endl;
    return "";

}


int GetVersion(std::string strFile,std::string strVFile)
{
    int Ret = 0;
    std::ifstream file(strFile); 
    std::ofstream fileV(strVFile);
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            if (line.find("STR_VERSION")!=line.npos) {//找到Version
                int postart = line.find("(\"");
                int posend = line.find("\")");
                std::string strVersion;
                strVersion = line.substr(postart + 2, posend - (postart + 2));
                std::cout << "===Module Version:" << strVersion << std::endl;

                if (fileV.is_open()) {
                    fileV << "Version    : " << strVersion << std::endl;
                    fileV << "Date       : " << GetDate() << std::endl;
                    fileV << "SVNVersion : " << getSvnVersion() << std::endl;
                    file.close();
                }
            }
        }
        file.close();
    }
    else {
        std::cout << "Can't Open File: "<< strFile << std::endl;
        Ret = -1;
    }
    return Ret;
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        std::cout << "Argc Must Be 3!" << std::endl;
        return -1;
    }
    else {
        return GetVersion(argv[1], argv[2]);
    }
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
