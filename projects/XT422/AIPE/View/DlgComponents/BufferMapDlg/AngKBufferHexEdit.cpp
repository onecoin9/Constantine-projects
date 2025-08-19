#include "AngKBufferHexEdit.h"
#include "ui_AngKBufferHexEdit.h"
#include "../qHexedit/achexviewedit.h"
#include "../qHexedit/document/qhexdocument.h"
#include "../qHexedit/document/buffer/qmemorybuffer.h"
#include "AngKCustomTab.h"
#include "StyleInit.h"
#include "XmlDefine.h"
#include "AngKCommonTools.h"
#include "AngKDataBuffer.h"
#include "AngKFillSetting.h"
#include "AngKGotoSetting.h"
#include "AngKHexViewEdit.h"
#include "AngKSwapSetting.h"
#include "ACMessageBox.h"
#include <QFontDatabase>
#include <QMessageBox>
#include <QTabBar>

#define SAFEDEL_ARRAY(_aptr)  do {if(_aptr)delete[] _aptr; _aptr=NULL;} while(0);

AngKBufferHexEdit::AngKBufferHexEdit(AngKDataBuffer* pDataBuf, QWidget *parent)
	: AngKDialog(parent)
	, m_bEdit(false)
{
	this->setObjectName("AngKBufferHexEdit");
	QT_SET_STYLE_SHEET(objectName());

	ui = new Ui::AngKBufferHexEdit();
	ui->setupUi(setCentralWidget());

	this->setFixedSize(775, 775);
	this->SetTitle(tr("BufferHexEdit"));
	InitText();
	InitButton();
	InitHexEdit(pDataBuf);

	m_pDataBuf = pDataBuf;
	//LoadXmlFile("D:\\ProgramSpace\\AP9900PCSoftware\\trunk\\AIPE\\AG06Client\\Depend\\pugixml\\ChipData_N25Qxx-V1.2.xml");
}

AngKBufferHexEdit::~AngKBufferHexEdit()
{
	delete ui;
}

void AngKBufferHexEdit::InitText()
{
	ui->bufferOperateGroup->setTitle(tr("Buffer operation(Read only)"));
	ui->gotoButton->setText(tr("Goto"));
	ui->fillButton->setText(tr("Fill"));
	ui->clearButton->setText(tr("Clear"));
	ui->EditButton->setText(tr("Edit Enable"));
	ui->findButton->setText(tr("Find"));
	ui->swapButton->setText(tr("Swap"));
	ui->bitsButton->setText(tr("8Bits/16Bits"));
	ui->okButton->setText(tr("Ok"));
	ui->checksumButton->setText(tr("Checksum"));
	ui->checksumShowButton->setText(tr("Fix Checksum Show"));
}

void AngKBufferHexEdit::InitButton()
{
	connect(this, &AngKBufferHexEdit::sgnClose, this, &AngKBufferHexEdit::close);
	//PartitionTab重绘初始化
	ui->partitionTabWidget->tabBar()->setStyle(new AngKCustomTab);
	//移动光标
	connect(ui->gotoButton, &QPushButton::clicked, this, &AngKBufferHexEdit::onSlotGoto);

	//可编辑控制
	connect(ui->EditButton, &QPushButton::clicked, this, &AngKBufferHexEdit::onSlotEditEnable);

	//清空
	connect(ui->clearButton, &QPushButton::clicked, this, &AngKBufferHexEdit::onSlotClearBuffer);

	//填充
	connect(ui->fillButton, &QPushButton::clicked, this, &AngKBufferHexEdit::onSlotFill);

	//查询
	connect(ui->findButton, &QPushButton::clicked, this, &AngKBufferHexEdit::onSlotFind);

	//Swap
	connect(ui->swapButton, &QPushButton::clicked, this, &AngKBufferHexEdit::onSlotSwap);
}

int AngKBufferHexEdit::LoadXmlFile(QString strFile)
{
	ui->partitionTabWidget->clear();

	pugi::xml_document doc;
	const wchar_t* encodedName = reinterpret_cast<const wchar_t*>(strFile.utf16());
	pugi::xml_parse_result result = doc.load_file(encodedName);

	if (!result)
		return XMLMESSAGE_LOAD_FAILED;

	pugi::xml_node root_node = doc.child(XML_NODE_CHIPDATA);
	pugi::xml_node bufMapSet = root_node.child(XML_NODE_CHIPDATA_BUFFERMAPSET);
	pugi::xml_node bufMapTable = bufMapSet.child(XML_NODE_CHIPDATA_BUFFERMAPTABLE);

	pugi::xml_node partition = bufMapTable.child(XML_NODE_CHIPDATA_BUFFERPARTITION);

	for (pugi::xml_node partNode = partition; partNode; partNode = partNode.next_sibling())
	{
		QTabWidget* subTabWidget = new QTabWidget();

		if (partNode.attribute("Visible").as_bool())
		{
			subTabWidget->tabBar()->setStyle(new AngKCustomTab);
			ui->partitionTabWidget->addTab(subTabWidget, partNode.attribute("Name").as_string());
		}

		for (pugi::xml_node vNode = partNode.first_child(); vNode; vNode = vNode.next_sibling())
		{
			AngKHexViewEdit* hexView = new AngKHexViewEdit();
			if (vNode.attribute("Visible").as_bool())
			{
				QString addr = QString(vNode.attribute("AddrRange").as_string());
				QStringList list = addr.split("-");
				hexView->setAddressRange(list[0].toUtf8().toULongLong(nullptr, 16));
				hexView->setAddressText(addr);
				subTabWidget->addTab(hexView, vNode.attribute("Name").as_string());
			}
		}
	}

	return XMLMESSAGE_SUCCESS;
}

void AngKBufferHexEdit::InitHexEdit(AngKDataBuffer* pDataBuf)
{
	int partCnt = pDataBuf->GetPartitionCount();
	for (int partIdx = 0; partIdx < partCnt; ++partIdx)
	{
		const tPartitionInfo* partInfo = pDataBuf->GetPartitionInfo(partIdx);

		QTabWidget* subTabWidget = new QTabWidget(ui->partitionTabWidget);
		if (partInfo->PartitionShow) {
			subTabWidget->tabBar()->setStyle(new AngKCustomTab);
			ui->partitionTabWidget->addTab(subTabWidget, QString::fromStdString(partInfo->partitionName));
		}

		int bufCnt = partInfo->vecSubView.size();
		for (int bufIdx = 0; bufIdx < bufCnt; ++bufIdx)
		{
			tBufInfo* bufferInfo = pDataBuf->GetBufferInfo(bufIdx, partIdx);
			if (bufferInfo->m_bBufferShow)
			{
				AngKHexViewEdit* hexView = new AngKHexViewEdit();
				hexView->setBufInfo(bufferInfo);
				//计算填入HexView的数据
				QByteArray chunk(reinterpret_cast<char*>(bufferInfo->m_pBuffMapped), bufferInfo->llBufEnd - bufferInfo->llBufStart);

				QByteArray ICBaseByte;
				ICBaseByte.resize(bufferInfo->m_adrSize);
				ICBaseByte.fill(bufferInfo->uBufOrgValue);
				ICBaseByte.replace(0, bufferInfo->m_adrSize, chunk);
				hexView->OpenMemoryData(ICBaseByte);

				//设置address起始大小
				QString addrStr = QString("%1-%2").arg(bufferInfo->llBufStart, 0, 16).arg(bufferInfo->llBufEnd, 0, 16);
				hexView->setAddressRange(bufferInfo->llBufStart, bufferInfo->llBufEnd);
				hexView->setAddressText(addrStr);

				subTabWidget->addTab(hexView, QString::fromStdString(bufferInfo->strBufName));
			}
		}
	}
	
}

AngKHexViewEdit* AngKBufferHexEdit::GetCurrentShowViewEdit()
{
	for (int i = 0; i < ui->partitionTabWidget->count(); ++i)
	{
		if(ui->partitionTabWidget->widget(i)->isHidden())
			continue;

		QTabWidget* subTabWidget = qobject_cast<QTabWidget*>(ui->partitionTabWidget->widget(i));
		for (int subIdx = 0; subIdx < subTabWidget->count(); ++subIdx)
		{
			if(subTabWidget->widget(subIdx)->isHidden())
				continue;

			AngKHexViewEdit* hexView = qobject_cast<AngKHexViewEdit*>(subTabWidget->widget(subIdx));
			return hexView;
		}
	}
	return nullptr;
}

void AngKBufferHexEdit::onSlotEditEnable()
{
	m_bEdit = !m_bEdit;
	AngKHexViewEdit* viewEdit = GetCurrentShowViewEdit();
	viewEdit->setEditEnable(m_bEdit);
	//TODO 判断是否可编辑
	if (!m_bEdit)
		ui->bufferOperateGroup->setTitle(tr("Buffer operation(Read only)"));
	else
		ui->bufferOperateGroup->setTitle(tr("Buffer operation(Write Enable)"));
}

void AngKBufferHexEdit::onSlotClearBuffer()
{
	AngKHexViewEdit* viewEdit = GetCurrentShowViewEdit();
	m_pDataBuf->ClearBuffer(viewEdit->GetBufInfo());

	tBufInfo* bufferInfo = viewEdit->GetBufInfo();
	QByteArray chunk(reinterpret_cast<char*>(bufferInfo->m_pBuffMapped), bufferInfo->llBufEnd - bufferInfo->llBufStart);

	QByteArray ICBaseByte;
	ICBaseByte.resize(bufferInfo->m_adrSize);
	ICBaseByte.fill(bufferInfo->uBufOrgValue);
	ICBaseByte.replace(0, bufferInfo->m_adrSize, chunk);
	viewEdit->OpenMemoryData(ICBaseByte);
}

void AngKBufferHexEdit::onSlotGoto()
{
	AngKGotoSetting gotoDlg(this);
	connect(&gotoDlg, &AngKGotoSetting::sgnGoToAddress, this, &AngKBufferHexEdit::onSlotGoToAddress);
	gotoDlg.exec();
	disconnect(&gotoDlg, &AngKGotoSetting::sgnGoToAddress, this, &AngKBufferHexEdit::onSlotGoToAddress);
}

void AngKBufferHexEdit::onSlotFill()
{
	AngKFillSetting fillDlg(tr("Fill Setting"), fillType, this);
	connect(&fillDlg, &AngKFillSetting::sgnFillBuffer, this, &AngKBufferHexEdit::onSlotFillBuffer);
	fillDlg.exec();
	disconnect(&fillDlg, &AngKFillSetting::sgnFillBuffer, this, &AngKBufferHexEdit::onSlotFillBuffer);
}

void AngKBufferHexEdit::onSlotFind()
{
	AngKFillSetting findDlg(tr("Find Setting"), findType, this);
	connect(&findDlg, &AngKFillSetting::sgnFindBuffer, this, &AngKBufferHexEdit::onSlotFindBuffer);
	connect(&findDlg, &AngKFillSetting::sgnFindNextBuffer, this, &AngKBufferHexEdit::onSlotFindNextBuffer);
	findDlg.exec();
	disconnect(&findDlg, &AngKFillSetting::sgnFindBuffer, this, &AngKBufferHexEdit::onSlotFindBuffer);
	disconnect(&findDlg, &AngKFillSetting::sgnFindNextBuffer, this, &AngKBufferHexEdit::onSlotFindNextBuffer);
}

void AngKBufferHexEdit::onSlotSwap()
{
	AngKSwapSetting swapDlg(this);
	swapDlg.exec();

	int getInt = swapDlg.GetSwapMode();

	if (getInt == AngKSwapSetting::NoMode)
		return;

	ADR	   adrBuffOff;
	//uchar  byBuf[BUFFER_RW_SIZE];
	uchar* byBuf = new uchar[BUFFER_RW_SIZE];

	for (adrBuffOff = 0; adrBuffOff < m_pDataBuf->GetSize(); adrBuffOff += BUFFER_RW_SIZE)
	{
		ADR iReadLen = m_pDataBuf->BufferRead(adrBuffOff, byBuf, BUFFER_RW_SIZE);

		if(iReadLen == 0)
			continue;

		if (getInt == AngKSwapSetting::ByteMode || getInt == AngKSwapSetting::WordMode) {
			for (int i = 0; i < iReadLen - 1; i += 2)
			{
				uchar byTemp = byBuf[i];
				byBuf[i] = byBuf[i + 1];
				byBuf[i + 1] = byTemp;
			}
		}

		if (getInt == AngKSwapSetting::BWMode || getInt == AngKSwapSetting::WordMode) {
			for (int i = 0; i < iReadLen - 3; i += 4)
			{
				uchar byTemp = byBuf[i];
				byBuf[i] = byBuf[i + 3];
				byBuf[i + 3] = byTemp;
				byTemp = byBuf[i + 1];
				byBuf[i + 1] = byBuf[i + 2];
				byBuf[i + 2] = byTemp;
			}
		}
		m_pDataBuf->BufferWrite(adrBuffOff, byBuf, iReadLen);
	}

	delete[] byBuf;

	ALOG_INFO("Buffer : Data are swapped, Swap type = %s", "CU", "--", swapDlg.GetSwapname((AngKSwapSetting::SwapMode)getInt).toStdString().c_str());

	//HexView更新
	AngKHexViewEdit* viewEdit = GetCurrentShowViewEdit();
	viewEdit->update();

	tBufInfo* bufferInfo = viewEdit->GetBufInfo();
	QByteArray chunk(reinterpret_cast<char*>(bufferInfo->m_pBuffMapped), bufferInfo->llBufEnd - bufferInfo->llBufStart);

	QByteArray ICBaseByte;
	ICBaseByte.resize(bufferInfo->m_adrSize);
	ICBaseByte.replace(0, bufferInfo->m_adrSize, chunk);
	viewEdit->OpenMemoryData(ICBaseByte);
}

void AngKBufferHexEdit::onSlotGoToAddress(QString address)
{
	AngKHexViewEdit* viewEdit = GetCurrentShowViewEdit();
	qint64 res = 0;
	bool ok = false;
	res = address.toLongLong(&ok, 16);
	viewEdit->gotoBar(res, false);
}

void AngKBufferHexEdit::onSlotFillBuffer(QString startAddr, QString endAddr, QString dataBuf, bool bAscii, bool bRandom)
{
	//检查非Ascii下，传入是否是16进制，不是直接返回
	if (!bAscii && !bRandom) {
		QRegExp hexRegex("[0-9A-Fa-f]+");
		// 检查输入字符串是否符合十六进制格式
		if (!hexRegex.exactMatch(dataBuf)) {
			ACMessageBox::showError(this, tr("BufMap Warning"), tr("Input string is not in hexadecimal format."));
			return;
		}
	}

	AngKHexViewEdit* viewEdit = GetCurrentShowViewEdit();
	tBufInfo* bufferInfo = viewEdit->GetBufInfo();

	bool ok;
	quint64 startAddress = startAddr.toULongLong(&ok, 16) + bufferInfo->llBufStart;//不同bufView的偏移量位置不同
	quint64 endAddress = endAddr.toULongLong(&ok, 16) + bufferInfo->llBufStart;

	if (endAddress > bufferInfo->llBufEnd) {
		ACMessageBox::showError(this, tr("BufMap Warning"), tr("Input error: End Address > Buffer End Address"));
		endAddress = bufferInfo->llBufEnd;
	}
	else if (startAddress > bufferInfo->llBufEnd){
		ACMessageBox::showError(this, tr("BufMap Warning"), tr("Input error: Start Address > Buffer End Address"));
		return;
	}

	int DataBufSize = 32 * 1024;
	uchar* pData = NULL;
	pData = new uchar[DataBufSize];
	if (!pData) {
		ALOG_FATAL("FillBuffer: memory alloc failed.", "CU", "--");
		SAFEDEL_ARRAY(pData);
		return;
	}

	uint64_t TotalWrite = endAddress - startAddress + 1;
	uint64_t Byteswrite, Offset = 0;

	if (bRandom) {
		for (uint64_t adrBuffOff = bufferInfo->m_BufFill.AddrStart; adrBuffOff < bufferInfo->m_BufFill.AddrEnd + 1; adrBuffOff += DataBufSize) {
			memcpy(pData, reinterpret_cast<uchar*>(Utils::AngKCommonTools::GenerateRandomString(DataBufSize).toUtf8().data()), DataBufSize);
			m_pDataBuf->BufferWrite(adrBuffOff, pData, qMin((uint64_t)DataBufSize, bufferInfo->m_BufFill.AddrEnd + 1 - adrBuffOff));
		}
	}
	else {
		// 计算需要拷贝的次数
		int dataSize = dataBuf.size();
		int copyTimes = DataBufSize / dataSize;

		// 使用memcpy和取余运算符循环拷贝"xxx"字符串到数组中
		for (int i = 0; i < copyTimes; ++i) {
			memcpy(pData + i * dataSize, dataBuf.toStdString().c_str(), dataSize);
		}

		while (TotalWrite > 0) {
			Byteswrite = TotalWrite > DataBufSize ? DataBufSize : TotalWrite;
			if (m_pDataBuf->BufferWrite(startAddress + Offset, pData, Byteswrite) == 0) {
				ALOG_FATAL("FillBuffer: WriteBuf failed.", "CU", "--");
				SAFEDEL_ARRAY(pData);
				return;
			}
			TotalWrite -= Byteswrite;
			Offset += Byteswrite;
		}
	}


	QByteArray chunk(reinterpret_cast<char*>(bufferInfo->m_pBuffMapped), bufferInfo->llBufEnd - bufferInfo->llBufStart);

	QByteArray ICBaseByte;
	ICBaseByte.resize(bufferInfo->m_adrSize);
	ICBaseByte.fill(bufferInfo->uBufOrgValue);
	ICBaseByte.replace(0, bufferInfo->m_adrSize, chunk);
	viewEdit->OpenMemoryData(ICBaseByte);
}

void AngKBufferHexEdit::onSlotFindBuffer(QString startAddr, QString endAddr, QString dataBuf, bool bAscii)
{
	AngKHexViewEdit* viewEdit = GetCurrentShowViewEdit();
	tBufInfo* bufferInfo = viewEdit->GetBufInfo();

	uint64_t adrBuffOff;
	uint64_t DataBufSize = 32 * 1024;
	uchar* pData = NULL;
	pData = new uchar[DataBufSize];

	for (adrBuffOff = bufferInfo->m_BufFind.AddrStart;
		adrBuffOff < bufferInfo->m_BufFind.AddrEnd + 1 - dataBuf.size();
		adrBuffOff += DataBufSize - dataBuf.size())
	{
		uint64_t adrReadLen = qMin(DataBufSize, bufferInfo->m_BufFind.AddrEnd + 1 - adrBuffOff);
		if (adrReadLen > DataBufSize) {
			return;
		}
		m_pDataBuf->BufferRead(adrBuffOff, pData, adrReadLen);
		for (int i = 0; i + dataBuf.size() <= adrReadLen; i++) {
			if (memcmp(dataBuf.toStdString().c_str(), pData + i, dataBuf.size()) == 0)
			{
				viewEdit->gotoBar(adrBuffOff + i, false);
				bufferInfo->m_BufFind.AddrFind = adrBuffOff + i;
				return;
			}
		}
	}
}

void AngKBufferHexEdit::onSlotFindNextBuffer(QString startAddr, QString endAddr, QString dataBuf, bool bAscii)
{
	AngKHexViewEdit* viewEdit = GetCurrentShowViewEdit();
	tBufInfo* bufferInfo = viewEdit->GetBufInfo();

	bool ok = false;
	uint64_t res = endAddr.toULongLong(&ok, 16);

	bufferInfo->m_BufFind.AddrStart = bufferInfo->m_BufFind.AddrFind + dataBuf.size();
	bufferInfo->m_BufFind.AddrEnd = res;

	onSlotFindBuffer(startAddr, endAddr, dataBuf, bAscii);
}
