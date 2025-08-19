#include "AngKChainSelectDlg.h"
#include "ui_AngKChainSelectDlg.h"
#include "StyleInit.h"
#include <QStandardItemModel>
#include <QHeaderView>

Q_DECLARE_METATYPE(std::string)
Q_DECLARE_METATYPE(std::vector<int>)

AngKChainSelectDlg::AngKChainSelectDlg(QWidget *parent)
	: AngKDialog(parent)
	, m_pChainModel(nullptr)
	, m_strIP("")
{
	this->setObjectName("AngKChainSelectDlg");
	QT_SET_STYLE_SHEET(objectName());

	ui = new Ui::AngKChainSelectDlg();
	ui->setupUi(setCentralWidget());

	this->setFixedSize(550, 260);
	this->SetTitle(tr("Chain Select"));
	setAttribute(Qt::WA_TranslucentBackground, true);

	InitTable();
	InitButton();
}

AngKChainSelectDlg::~AngKChainSelectDlg()
{
	delete ui;
}

void AngKChainSelectDlg::InitTable()
{
	m_pChainModel = new QStandardItemModel(this);

	ui->chainTableView->verticalHeader()->setVisible(false);
	QStringList headList;
	headList << tr("HopNum") << tr("LinkNum") << tr("SiteSN") << tr("SiteAlias");

	m_pChainModel->setHorizontalHeaderLabels(headList);
	ui->chainTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

	ui->chainTableView->setModel(m_pChainModel);
	ui->chainTableView->setAlternatingRowColors(true);
	ui->chainTableView->horizontalHeader()->setHighlightSections(false);
	ui->chainTableView->horizontalHeader()->setStretchLastSection(true);
	ui->chainTableView->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);

	QHeaderView* netHead = ui->chainTableView->horizontalHeader();

	// 列宽度自适应
	netHead->setSectionResizeMode(QHeaderView::Fixed);

	//网络表格固定，根据UI保持固定
	ui->chainTableView->setColumnWidth(0, 75);
	ui->chainTableView->setColumnWidth(1, 75);
	ui->chainTableView->setColumnWidth(2, 175);
	ui->chainTableView->setColumnWidth(3, 95);
}

void AngKChainSelectDlg::InitButton()
{
	ui->okButton->setText(tr("OK"));
	connect(ui->okButton, &QPushButton::clicked, this, &AngKChainSelectDlg::onSlotSelectChain);
	connect(this, &AngKChainSelectDlg::sgnClose, this, &AngKChainSelectDlg::close);
}

void AngKChainSelectDlg::SetChainInfo(DeviceStu& devInfo)
{
	if (!devInfo.getLastHop)
		return;

	m_strIP = devInfo.strIP;
	int row = m_pChainModel->rowCount();
	m_pChainModel->insertRow(m_pChainModel->rowCount());
	for (int linkIdx = 0; linkIdx < devInfo.nLinkNum + 1; ++linkIdx) {
		m_pChainModel->setData(m_pChainModel->index(linkIdx, HopIndex), QString::number(devInfo.nHopNum));
		m_pChainModel->item(linkIdx, HopIndex)->setCheckable(true);

		m_pChainModel->setData(m_pChainModel->index(linkIdx, ChainID), QString::number(devInfo.nChainID));
		m_pChainModel->setData(m_pChainModel->index(linkIdx, SiteSN), QString::fromStdString(devInfo.tMainBoardInfo.strHardwareSN));
		m_pChainModel->setData(m_pChainModel->index(linkIdx, SiteAlias), QString::fromStdString(devInfo.strSiteAlias));
	}
}

void AngKChainSelectDlg::onSlotSelectChain() 
{
	std::vector<int> chainVec;
	for (int i = 0; i < m_pChainModel->rowCount(); ++i) {
		if (m_pChainModel->item(i, HopIndex)->checkState() == Qt::Checked){
			int linkIdx = m_pChainModel->data(m_pChainModel->index(i, ChainID)).toInt();
			chainVec.push_back(linkIdx);
		}
	}

	if (!chainVec.empty()) {
		emit sgnSelectChain(m_strIP, chainVec);
	}

	close();
}
