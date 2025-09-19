#include "AngKSwapSetting.h"
#include "ui_AngKSwapSetting.h"
#include "StyleInit.h"
#include <QButtonGroup>

AngKSwapSetting::AngKSwapSetting(QWidget *parent)
	: AngKDialog(parent)
	, m_swapGroup(nullptr)
	, m_nSwapMode(NoMode)
{
	this->setObjectName("AngKSwapSetting");
	QT_SET_STYLE_SHEET(objectName());

	ui = new Ui::AngKSwapSetting();
	ui->setupUi(setCentralWidget());

	this->setFixedSize(450, 200);
	this->SetTitle(tr("Swap Setting"));
	setAttribute(Qt::WA_TranslucentBackground, true);

	InitText();

	connect(ui->okButton, &QPushButton::clicked, this, [=]() {
		accept();
		});
	connect(this, &AngKSwapSetting::sgnClose, this, &AngKSwapSetting::close);
}

AngKSwapSetting::~AngKSwapSetting()
{
	delete ui;
}

void AngKSwapSetting::InitText()
{
	ui->noButton->setText("No Swap");
	ui->byteButton->setText("Byte Swap(0x01 0x02 0x03 0x04->0x02 0x01 0x4 0x03)");
	ui->wordButton->setText("Word swap(0x01 0x02 0x03 0x04->0x03 0x04 0x01 0x02)");
	ui->BWButton->setText("B+W Swap(0x01 0x02 0x03 0x04->0x04 0x03 0x02 0x01)");

	m_swapGroup = new QButtonGroup(this);

	m_swapGroup->addButton(ui->noButton, NoMode);
	m_swapGroup->addButton(ui->byteButton, ByteMode);
	m_swapGroup->addButton(ui->wordButton, WordMode);
	m_swapGroup->addButton(ui->BWButton, BWMode);

	connect(m_swapGroup, static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked), [&](int index) {
		m_nSwapMode = (SwapMode)index;
		});
}

QString AngKSwapSetting::GetSwapname(SwapMode modeName)
{
	QString strSwapName;
	switch (modeName)
	{
	case AngKSwapSetting::NoMode:
		strSwapName = "No Swap";
		break;
	case AngKSwapSetting::ByteMode:
		strSwapName = "Byte Swap";
		break;
	case AngKSwapSetting::WordMode:
		strSwapName = "Word Swap";
		break;
	case AngKSwapSetting::BWMode:
		strSwapName = "B+W Swap";
		break;
	default:
		strSwapName = "Unknown Swap";
		break;
	}

	return strSwapName;
}
