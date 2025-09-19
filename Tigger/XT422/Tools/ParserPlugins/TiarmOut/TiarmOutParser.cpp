#include "TiarmOutParser.h"
#include <QFileInfo>
#include <QApplication>
#include <QProcess>
#include <QPluginLoader>

#define STRMSG_LEN	(256)
TiarmOutParser::TiarmOutParser()
{
}


char* TiarmOutParser::getVersion()
{
	return "1.0.0.0";
}
char* TiarmOutParser::getFormatName()
{
	return C_TIARMOUT;
}
bool TiarmOutParser::TiarmOutParser::ConfirmFormat(QString& filename)
{
	if (!QFileInfo(filename).suffix().compare("out", Qt::CaseInsensitive)) {
		return false;
	}

	char TmpHex[256];
	sprintf(TmpHex, "%s/tools/thirdparty/tmp1234.hex", QApplication::applicationDirPath().toLocal8Bit().data());
	int Ret = ExecHexTIARMTool(filename, QString(TmpHex));

	QFile::remove(TmpHex);
	return Ret;
}


bool TiarmOutParser::ExecHexTIARMTool(QString strCurFile, QString& OutFile)
{
	unsigned long  NumberOfBytesWritten;
	unsigned long dwExitCode = 1;
	QString ToolPath;
	QString CmdLine;

	ToolPath = QString("%1/tools/thirdparty/Tiarm.exe").arg(QApplication::applicationDirPath());
	//ToolPath.Format("%s/NandImage.exe", GetAppPath());
	
	QStringList paraList;
	paraList << "-i" << "--memwidth=8" << QString("\"%1\"").arg(strCurFile.toLocal8Bit().data()) << "-o" << QString("\"%1\"").arg(OutFile.toLocal8Bit().data());
	QProcess subProcess;
	subProcess.start(ToolPath, paraList);

	while (1) {
		if (!subProcess.waitForReadyRead(10000)) {
			break;
		}
	}
	dwExitCode = subProcess.exitCode();
	subProcess.waitForFinished();
	if (dwExitCode) {///失败
		m_pOutput->Error("");
	}
__end:
	return dwExitCode;
}


int TiarmOutParser::TransferFile(QString& srcfn, QIODevice* dst)
{
	m_Dst = dst;
	int Rtn = 0;
	QString tmpHex = "%1/tools/thirdparty/tmp%2.hex";
	QString i10_path = srcfn.replace(".out", ".i10");
	tmpHex.arg(QApplication::applicationDirPath(), srcfn);
	Rtn = ExecHexTIARMTool(srcfn, tmpHex);
	if (Rtn == 0) {///生成Hex成功
		QPluginLoader pluginloader(QApplication::applicationDirPath() + "/parser/HexParser.dll");
		QObject* plugin = pluginloader.instance();
		if (plugin) // DLL 成功載入
		{
			AbstractParser* pHexParser = qobject_cast<AbstractParser*>(plugin);
			if (!pHexParser)
			{
				return -1;
			}
			QFile hexFile(tmpHex), i10File(i10_path);
			// 尝试打开文件，创建文件，如果它不存在
			if (!hexFile.open(QIODevice::ReadWrite | QIODevice::Append) || !i10File.open(QIODevice::ReadWrite | QIODevice::Append)) {

				return -1;
			}
			pHexParser->TransferFile(tmpHex, &i10File);

			pHexParser->TransferFile(i10_path, dst);
			hexFile.close();
			i10File.close();

			QFile::remove(tmpHex);
		}
		else {
			ferror = C_Err_RecordSum;
		}
	}
	else {
		ferror = C_Err_RecordSum;
		m_pOutput->Error("create hex fail!");
	}

	return ferror;
}
