#include "AngKFilesImport.h"
#include "ui_AngKFilesImport.h"
#include "StyleInit.h"
#include "DataTransform.h"
#include "AngKSwapSetting.h"
#include "parser_global.h"
#include "ACMessageBox.h"
#include <QCoreApplication>
#include <QButtonGroup>
#include <QDesktopWidget>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>

#define CHANGE_HEIGHT 300

AngKFilesImport::AngKFilesImport(QWidget *parent)
	: AngKDialog(parent)
	, m_swapGroup(nullptr)
	, isDoubleClicked(false)
{
	this->setObjectName("AngKFilesImport");
	QT_SET_STYLE_SHEET(objectName());

	ui = new Ui::AngKFilesImport();
	ui->setupUi(setCentralWidget());

	this->setFixedSize(636, 690);
	this->SetTitle(tr("Files Import"));
	setAttribute(Qt::WA_TranslucentBackground, true);

	InitDlg();

}

AngKFilesImport::~AngKFilesImport()
{
	delete ui;
}

void AngKFilesImport::InitText()
{
	ui->nameText->setText(tr("Name"));
	ui->fileText->setText(tr("File"));
	ui->formatText->setText(tr("Format"));
	ui->wordAddressCheck->setText(tr("Word Address"));
	ui->AutoFormatCheck->setText(tr("Auto Detect Format"));
	ui->fileCheckText->setText(tr("File expected checksum"));
	ui->fileLineEdit->setReadOnly(true);
	ui->fileCheckTextEdit->setReadOnly(true);

	ui->settingCheck->setText(tr("Advanced Setting"));
	ui->addressGroup->setTitle(tr("Address Relocation"));
	ui->swapGroup->setTitle(tr("Swap Configuration"));

	ui->fileAddrText->setText(tr("File Address(hex):"));
	ui->bufferAddrText->setText(tr("Buffer Address(hex):"));
	ui->lengthText->setText(tr("Length(hex):"));

	ui->bufferAddrCheck->setText(tr("Use Negative Offset"));
	ui->lengthTip->setText(tr("(0 for whole file)"));

	ui->fileAddrTextEdit->setPlaceholderText("0");
	ui->bufferAddrEdit->setPlaceholderText("0");

	ui->addButton->setText(tr("Add"));
	ui->cancelButton->setText(tr("Cancel"));

	ui->HashAlgoText->setText(tr("Hash Algorithm"));
	ui->HashButton->setText(tr("Generate"));

	ui->fileTagText->setText(tr("FileTag"));
	//ui->fileTagComboBox->addItem("APP");
	//ui->fileTagComboBox->addItem("BLE");
	//ui->fileTagComboBox->addItem("FLASH");
	//ui->fileTagComboBox->addItem("SEC");
}

void AngKFilesImport::InitGroup()
{
	connect(ui->settingCheck, &QCheckBox::stateChanged, this, [=](int state) {
		
		if (state == Qt::CheckState::Checked)
		{
			ui->addressGroup->hide();
			ui->swapGroup->hide();
			this->setFixedHeight(this->height() - CHANGE_HEIGHT);
		}
		else if(state == Qt::CheckState::Unchecked)
		{
			ui->addressGroup->show();
			ui->swapGroup->show();
			this->setFixedHeight(this->height() + CHANGE_HEIGHT);
		}
	});

	m_swapGroup = new QButtonGroup(ui->swapGroup);

	connect(m_swapGroup, static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked), [&](int index) {
		qDebug() << "单击编号为 " << index;
		});

	ui->radioButton_1->setText(tr("No Swap"));
	ui->radioButton_2->setText(tr("Byte Swap (01 02 03 04 > 02 01 04 03)"));
	ui->radioButton_3->setText(tr("Word Swap (01 02 03 04 > 02 01 04 03)"));
	ui->radioButton_4->setText(tr("B+W Swap (01 02 03 04 > 02 01 04 03)"));

	m_swapGroup->addButton(ui->radioButton_1, AngKSwapSetting::SwapMode::NoMode);
	m_swapGroup->addButton(ui->radioButton_2, AngKSwapSetting::SwapMode::ByteMode);
	m_swapGroup->addButton(ui->radioButton_3, AngKSwapSetting::SwapMode::WordMode);
	m_swapGroup->addButton(ui->radioButton_4, AngKSwapSetting::SwapMode::BWMode);

	ui->radioButton_1->setChecked(true);
}

void AngKFilesImport::InitDetectInfo()
{
	DataTransform dt(this);
	for (int i = ENUMFORMAT::Binary; i < ENUMFORMAT::LatticeFEA + 1; i++)
	{
		ui->formatComboBox->addItem(dt.EnumToName((ENUMFORMAT)i));
	}
}

void AngKFilesImport::InitDlg()
{
	InitText();
	InitGroup();
	InitParsers();
	//InitDetectInfo();
	connect(this, &AngKFilesImport::sgnClose, this, &AngKFilesImport::close);
	connect(ui->fileBtn, &QPushButton::clicked, this, &AngKFilesImport::onSlotSelectFile);
	connect(ui->addButton, &QPushButton::clicked, this, &AngKFilesImport::onSlotAddFile);
	connect(ui->cancelButton, &QPushButton::clicked, this, [=]() 
	{
		clearDlg();
		close();
	});

	connect(ui->HashButton, &QPushButton::clicked, this, [=]()
	{
		if(!ui->fileLineEdit->text().isEmpty())
			ui->fileCheckTextEdit->setText(fileChecksum(ui->fileLineEdit->text(), static_cast<QCryptographicHash::Algorithm>(ui->HashAlgoComboBox->currentIndex())));
	});

	ui->AutoFormatCheck->setChecked(true);//默认开启档案自动检查

	QDesktopWidget* m = QApplication::desktop();
	QRect desk_rect = m->screenGeometry(m->screenNumber(QCursor::pos()));
	this->move(desk_rect.width() / 2 - this->width() / 2 + desk_rect.left(), desk_rect.height() / 2 - this->height() / 2 + desk_rect.top());
}

void AngKFilesImport::InitParsers()
{
	QString parserPath = Utils::AngKPathResolve::localParsersPath();

	QDir parserDir(parserPath);
	// get all available parser dll filenames
	foreach(QFileInfo info, parserDir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot)) {
		QPluginLoader pluginloader(info.absoluteFilePath());
		QObject* plugin = pluginloader.instance();
		if (plugin) {
			AbstractParser* parser = qobject_cast<AbstractParser*>(plugin);
			if (parser) {
				QString name = parser->getFormatName();
				ui->formatComboBox->addItem(name);
			}
		}
	}
}

void AngKFilesImport::clearDlg()
{
	ui->nameLineEdit->clear();
	ui->fileLineEdit->clear();
	ui->formatComboBox->setCurrentIndex(-1);
	ui->wordAddressCheck->setChecked(false);
	ui->fileCheckTextEdit->clear();
	ui->addressGroup->setChecked(false);
	ui->bufferAddrCheck->setChecked(false);//use Negative Offset
	ui->fileAddrTextEdit->clear();
	ui->bufferAddrEdit->clear();
	ui->lengthEdit->clear();
	ui->swapGroup->setChecked(false);
	m_swapGroup->buttonClicked(0);
}

QString AngKFilesImport::fileChecksum(const QString& fileName, QCryptographicHash::Algorithm hashAlgorithm)
{
	QFile sourceFile(fileName);
	qint64 fileSize = sourceFile.size();
	qint64 cnt = fileSize;
	const qint64 bufferSize = 10240;

	if (sourceFile.open(QIODevice::ReadOnly) && (fileSize > 0))
	{
		char buffer[bufferSize];
		int bytesRead;
		int readSize = qMin(cnt, bufferSize);
		int p;

		QCryptographicHash hash(hashAlgorithm);
		while ((readSize > 0) && ((bytesRead = sourceFile.read(buffer, readSize)) > 0))
		{
			cnt -= bytesRead;
			hash.addData(buffer, bytesRead);
			readSize = qMin(cnt, bufferSize);
		}

		sourceFile.close();
		return QString(hash.result().toHex());
	}
	else
		return QString(); // Clear ui.eHashedit
}

void AngKFilesImport::SetFileTagHide(bool bHide)
{
	ui->fileTagWgt->setHidden(!bHide);
}

bool AngKFilesImport::GetFileTagHide()
{
	return ui->fileTagWgt->isHidden();
}

void AngKFilesImport::SetFileDataInfo(FileDataInfo& dataInfo)
{
	ui->fileTagComboBox->setCurrentText(QString::fromStdString(dataInfo.fileTagStr));
	ui->bufferAddrEdit->setText(QString::fromStdString(dataInfo.fileBufferAddressStr));
	ui->formatComboBox->setCurrentText(QString::fromStdString(dataInfo.fileFormatStr));
	bool addrCheck = dataInfo.fileRelcationEnable == "TRUE" ? true : false;
	ui->addressGroup->setChecked(addrCheck);
	isDoubleClicked = true;
}

void AngKFilesImport::DetectFileFormat(QString strFile)
{
	bool bFind = false;
	QString parserPath = Utils::AngKPathResolve::localParsersPath();

	QDir parserDir(parserPath);
	// get all available parser dll filenames
	foreach(QFileInfo info, parserDir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot)) {
		QPluginLoader pluginloader(info.absoluteFilePath());
		QObject* plugin = pluginloader.instance();
		if (plugin) {
			AbstractParser* parser = qobject_cast<AbstractParser*>(plugin);
			if (parser && parser->ConfirmFormat(strFile)) {
				QString name = parser->getFormatName();
				ui->formatComboBox->setCurrentIndex(ui->formatComboBox->findText(name, Qt::MatchExactly));
				bFind = true;
				break;
			}
		}
	}

	if (!bFind) {
		ui->formatComboBox->setCurrentIndex(-1);
	}
}

//void AngKFilesImport::TransferFileFormat(QString srcFile, QFile& dstFile)
//{
//	bool bFind = false;
//	bool AutoDetect = ui->AutoFormatCheck->isChecked();
//	QString parserPath = Utils::AngKPathResolve::localParsersPath();
//	QString FormatName = "";
//
//	if (ui->formatComboBox->currentIndex() >= 0)
//		FormatName = ui->formatComboBox->currentText();
//	else
//		AutoDetect = true;
//
//	QDir parserDir(parserPath);
//	// get all available parser dll filenames
//	foreach(QFileInfo info, parserDir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot)) {
//		QPluginLoader pluginloader(info.absoluteFilePath());
//		QObject* plugin = pluginloader.instance();
//		if (plugin) {
//			AbstractParser* parser = qobject_cast<AbstractParser*>(plugin);
//			QString foematName = parser->getFormatName();
//			if (parser && ((AutoDetect && parser->ConfirmFormat(srcFile)) || (!AutoDetect) && (FormatName.compare(parser->getFormatName()) == 0))) {
//				bFind = true;
//				//QFile dst(Utils::AngKPathResolve::localTempFolderPath() + "IOHeap.bin");
//				dstFile.open(QIODevice::ReadWrite | QIODevice::Truncate);
//				if (dstFile.isOpen())
//				{
//					parser->TransferFile(srcFile, &dstFile);
//					dstFile.flush();
//					//QByteArray dstdata = dst.readAll();
//					//int32_t dstLength = dst.size();
//					//dst.close();
//				}
//				break;
//			}
//		}
//	}
//
//	if (!bFind) {
//		ACMessageBox::showWarning(this, tr("FileTransfer"), tr("Format Parser not found!"));
//	}
//}

void AngKFilesImport::TransferFileFormat(QString srcFile, QFile& dstFile) {
	bool bFind = false;
	bool AutoDetect = ui->AutoFormatCheck->isChecked();
	QString FormatName = "";

	if (ui->formatComboBox->currentIndex() >= 0) {
		FormatName = ui->formatComboBox->currentText();
	}
	else {
		AutoDetect = true;
	}

	QString parserPath = Utils::AngKPathResolve::localParsersPath();
	QDir parserDir(parserPath);

	// 缓存解析器列表（建议在类初始化时预加载）
	static QList<AbstractParser*> cachedParsers;
	if (cachedParsers.isEmpty()) {
		foreach(QFileInfo info, parserDir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot)) {
			QPluginLoader pluginLoader(info.absoluteFilePath());
			QObject* plugin = pluginLoader.instance();
			if (plugin) {
				AbstractParser* parser = qobject_cast<AbstractParser*>(plugin);
				if (parser) {
					cachedParsers.append(parser);
				}
			}
			pluginLoader.unload(); // 立即卸载，避免长期占用
		}
	}

	foreach(AbstractParser * parser, cachedParsers) {
		bool isAutoDetectMatch = AutoDetect && parser->ConfirmFormat(srcFile);
		bool isManualMatch = !AutoDetect && (FormatName == parser->getFormatName());

		if (isAutoDetectMatch || isManualMatch) {
			bFind = true;
			if (!dstFile.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
				ACMessageBox::showWarning(this, tr("FileTransfer"), tr("Failed to open destination file!"));
				return;
			}
			parser->TransferFile(srcFile, &dstFile);
			dstFile.flush();
			break;
		}
	}

	if (!bFind) {
		ACMessageBox::showWarning(this, tr("FileTransfer"), tr("Format Parser not found!"));
	}
}

void AngKFilesImport::onSlotAddFile()
{
	FileDataInfo fileInfo;

	fileInfo.fileNameStr = ui->nameLineEdit->text().toStdString();

	if (fileInfo.fileNameStr.empty()) {
		ACMessageBox::showWarning(this, tr("Add Warn"), tr("No file selected, adding file failed"));
		return;
	}

	fileInfo.fileTagStr = ui->fileTagComboBox->currentText().toStdString();

	QFileInfo info(ui->fileLineEdit->text());
	fileInfo.fileSizeStr = info.exists() ? QString::number(info.size()).toStdString() : QString::number(0).toStdString();
	fileInfo.filePathStr = ui->fileLineEdit->text().toStdString();
	fileInfo.fileFormatStr = ui->formatComboBox->currentText().toStdString();
	fileInfo.fileCheckStr = ui->fileCheckTextEdit->text().toStdString();
	fileInfo.fileAutoDetecteFormatStr = ui->AutoFormatCheck->isChecked() ? std::string("TRUE") : std::string("FALSE");
	fileInfo.fileWordAddressEnable = ui->wordAddressCheck->isChecked() ? std::string("TRUE") : std::string("FALSE");
	fileInfo.fileRelcationEnable = ui->addressGroup->isChecked() ? std::string("TRUE") : std::string("FALSE");
	fileInfo.fileBufferAddressStr = QString("0x" + QString::number(ui->bufferAddrEdit->text().toInt())).toStdString();
	fileInfo.fileAddressStr = QString("0x" + QString::number(ui->fileAddrTextEdit->text().toInt())).toStdString();
	fileInfo.fileLoadLengthStr = QString("0x" + QString::number(ui->lengthEdit->text().toInt())).toStdString();
	fileInfo.fileSwapType = m_swapGroup->checkedId();
	fileInfo.fileSwapTypeStr = AngKSwapSetting::GetSwapname((AngKSwapSetting::SwapMode)m_swapGroup->checkedId()).toStdString();
	//fileInfo.fileChecksumType = static_cast<QCryptographicHash::Algorithm>(ui->HashAlgoComboBox->currentIndex());
	fileInfo.fileChecksumType = ui->HashAlgoComboBox->currentText().toStdString();
	fileInfo.fileChecksumValue = ui->fileCheckTextEdit->text().toStdString();

	emit sgnSelectFileInfo(fileInfo, isDoubleClicked);

	isDoubleClicked = false;
	close();
}

void AngKFilesImport::onSlotSelectFile()
{
	QString filePath = QFileDialog::getOpenFileName(this, "Select File...", QCoreApplication::applicationDirPath(), tr("All Files(*.*) ;; bin Files(*.bin);; hex Files(*.hex);; nand Files(*.nand);; out Files(*.out)"));
	ui->fileLineEdit->setText(filePath);

	if (ui->AutoFormatCheck->isChecked())
	{
		//通过Parser_global插件管理进行档案解析，返回文件数据，使用QFile返回
		DetectFileFormat(filePath);
	}

	ui->nameLineEdit->setText(filePath.mid(filePath.lastIndexOf("/") + 1));
}