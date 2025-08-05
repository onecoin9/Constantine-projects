#include "AngKHexViewEdit.h"
#include "ui_AngKHexViewEdit.h"
#include "../View/GlobalInit/StyleInit.h"
#include "../qHexedit/achexviewedit.h"
#include "../qHexedit/document/qhexdocument.h"
#include "../qHexedit/document/buffer/qmemorybuffer.h"
#include "AngKTransmitSignals.h"

AngKHexViewEdit::AngKHexViewEdit(QWidget *parent)
	: QWidget(parent)
{
	ui = new Ui::AngKHexViewEdit();
	ui->setupUi(this);

	//OpenFile("D:\\ProgramSpace\\AG06Client\\AG06Client\\Skin\\Dark\\Qt5WebEngineCored.pdb");
	ui->hexWidget->setViewFont("SimSun", 11);

	InitText();
	connect(ui->hexWidget, &AcHexViewEdit::sgnHexEditInput, &AngKTransmitSignals::GetInstance(), &AngKTransmitSignals::sgnHexEditInput);

	this->setObjectName("AngKHexViewEdit");
	QT_SET_STYLE_SHEET(objectName());
}

AngKHexViewEdit::~AngKHexViewEdit()
{
	disconnect(ui->hexWidget, &AcHexViewEdit::sgnHexEditInput, &AngKTransmitSignals::GetInstance(), &AngKTransmitSignals::sgnHexEditInput);
	delete ui;
}

void AngKHexViewEdit::InitText()
{
	ui->bufferRangeText->setText(tr("BufferRange:"));
}

void AngKHexViewEdit::OpenFile(QString fileName)
{
	/* ...or from File */
	QHexDocument* document = QHexDocument::fromFile<QMemoryBuffer>(fileName);

	ui->hexWidget->setDocument(document);
}

void AngKHexViewEdit::OpenIODevice(QIODevice* ioDev)
{
	// ...from a generic I/O device...
	QHexDocument* document = QHexDocument::fromDevice<QMemoryBuffer>(ioDev);

	ui->hexWidget->setDocument(document);
}

void AngKHexViewEdit::OpenMemoryData(QByteArray byteData)
{
	// Load data from In-Memory Buffer...
	QHexDocument* document = QHexDocument::fromMemory<QMemoryBuffer>(byteData);

	ui->hexWidget->setDocument(document);
}

void AngKHexViewEdit::setAddressRange(quint64 beginAddr, quint64 endAddr)
{
	ui->hexWidget->setAddressBase(beginAddr);
}

void AngKHexViewEdit::setAddressText(QString addr)
{
	ui->bufferRangeEdit->setText(addr);
}

void AngKHexViewEdit::setBufInfo(tBufInfo* _bufInfo)
{
	m_pBufInfo = _bufInfo;
}

void AngKHexViewEdit::setEditEnable(bool bEdit)
{
	ui->hexWidget->setEditEnable(bEdit);
}

tBufInfo* AngKHexViewEdit::GetBufInfo()
{
	return m_pBufInfo;
}

void AngKHexViewEdit::ClearEdit()
{
	ui->hexWidget->document()->metadata()->clear();
}

void AngKHexViewEdit::gotoBar(int pos, bool isLine)
{
	ui->hexWidget->renderer()->enableCursor(true);
	ui->hexWidget->document()->cursor()->selectOffset(0, 1);
	auto cur = ui->hexWidget->document()->cursor();
	isLine ? cur->moveTo(quint64(pos), 0) : cur->moveTo(pos);
}
