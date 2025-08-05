#include "AngKProjectOperation.h"
#include "ui_AngKProjectOperation.h"
#include "../View/GlobalInit/StyleInit.h"
#include "AngKProjDataset.h"
#include "json.hpp"
#include <QButtonGroup>

//StackWidget
#define ERASE_PAGE	0
#define BLANK_PAGE	1
#define PROG_PAGE	2
#define VERIFY_PAGE 3
#define SECURE_PAGE 4
#define READ_PAGE	5
#define SELF_PAGE	6

AngKProjectOperation::AngKProjectOperation(QWidget *parent)
	: QWidget(parent)
	, m_pageGroup(nullptr)
{
	ui = new Ui::AngKProjectOperation();
	ui->setupUi(this);

	ui->plainTextEdit->appendPlainText(tr("Operating Flow:"));
	ui->plainTextEdit->setReadOnly(true);

	InitOperatePath();
	InitWidgetInfo();
	InitButtonGroup();

	this->setObjectName("AngKProjectOperation");
	QT_SET_STYLE_SHEET(objectName());
}

AngKProjectOperation::~AngKProjectOperation()
{
	if (m_pageGroup)
	{
		m_pageGroup = nullptr;
		delete m_pageGroup;
	}
	m_box2Widget.clear();
	m_widget2Box.clear();

	delete ui;
}

void AngKProjectOperation::InitOperatePath()
{
	m_mapOperatePath[OperationTagType::Erase] = Utils::AngKPathResolve::localRelativeSkinPath() + "/PushButton/eraseButtonHover.svg";
	m_mapOperatePath[OperationTagType::Blank] = Utils::AngKPathResolve::localRelativeSkinPath() + "/PushButton/blankButtonHover.svg";
	m_mapOperatePath[OperationTagType::Program] = Utils::AngKPathResolve::localRelativeSkinPath() + "/PushButton/programButtonHover.svg";
	m_mapOperatePath[OperationTagType::Verify] = Utils::AngKPathResolve::localRelativeSkinPath() + "/PushButton/verifyButtonHover.svg";
	m_mapOperatePath[OperationTagType::Secure] = Utils::AngKPathResolve::localRelativeSkinPath() + "/PushButton/secureButtonHover.svg";
	m_mapOperatePath[OperationTagType::Read] = Utils::AngKPathResolve::localRelativeSkinPath() + "/PushButton/readButtonHover.svg";
	m_mapOperatePath[OperationTagType::CheckSum] = Utils::AngKPathResolve::localRelativeSkinPath() + "/PushButton/compareButton.svg";
}

void AngKProjectOperation::InitErasePage()
{
	m_box2Widget[ui->erasePage_eraseTagComboBox] = ui->erasePage_eraseTagWgt;
	m_box2Widget[ui->erasePage_blankTagComboBox] = ui->erasePage_blankTagWgt;

	ui->erasePage_eraseTagWgt->InitWidget(tr("Erase"), m_mapOperatePath[OperationTagType::Erase], OperationTagType::Erase);
	ui->erasePage_blankTagWgt->InitWidget(tr("Blank"), m_mapOperatePath[OperationTagType::Blank], OperationTagType::Blank);

	ui->erasePage_eraseTagComboBox->addItem(tr("Erase"), (int)ChipOperCfgSubCmdID::SubErase);
	ui->erasePage_eraseTagComboBox->setEnabled(false);
	ui->erasePage_blankTagComboBox->addItem(tr("Disabled"), (int)ChipOperCfgSubCmdID::UnEnable);
	ui->erasePage_blankTagComboBox->addItem(tr("Blank Check"), (int)ChipOperCfgSubCmdID::SubBlankCheck);
	ui->erasePage_blankTagComboBox->setCurrentIndex(1); //默认展示BlankCheck
	connect(ui->erasePage_blankTagComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AngKProjectOperation::onSlotChangeWidget);

	ui->erasePage_blankTagWgt->setFlowLabelText(tr("Blank Check"));

	GetErasePageFlow();
}

void AngKProjectOperation::InitBlankPage()
{
	m_box2Widget[ui->blankPage_blankTagComboBox] = ui->blankPage_blankTagWgt;

	ui->blankPage_blankTagWgt->InitWidget(tr("Blank"), m_mapOperatePath[OperationTagType::Blank], OperationTagType::Blank);
	ui->blankPage_blankTagComboBox->addItem(tr("Blank Check"), (int)ChipOperCfgSubCmdID::SubBlankCheck);
	ui->blankPage_blankTagComboBox->setEnabled(false);
	ui->blankPage_blankTagWgt->setFlowLabelText(tr("Blank Check"));
}

void AngKProjectOperation::InitProgramPage()
{
	m_box2Widget[ui->progPage_eraseTagComboBox] = ui->progPage_eraseTagWgt;
	m_box2Widget[ui->progPage_blankTagComboBox] = ui->progPage_blankTagWgt;
	m_box2Widget[ui->progPage_progComboBox] = ui->progPage_progTagWgt;
	m_box2Widget[ui->progPage_verifyTagComboBox] = ui->progPage_verifyTagWgt;
	m_box2Widget[ui->progPage_marginVerifyTagComboBox] = ui->progPage_marginVerifyTagWgt;
	m_box2Widget[ui->progPage_CompareTagComboBox] = ui->progPage_CompareTagWgt;
	m_box2Widget[ui->progPage_SecureTagComboBox] = ui->progPage_SecureTagWgt;

	ui->progPage_eraseTagWgt->InitWidget(tr("Erase"), m_mapOperatePath[OperationTagType::Erase], OperationTagType::Erase);
	ui->progPage_eraseTagComboBox->addItem(tr("Disabled"), (int)ChipOperCfgSubCmdID::UnEnable);
	ui->progPage_eraseTagComboBox->addItem(tr("Erase"), (int)ChipOperCfgSubCmdID::SubErase);
	ui->progPage_eraseTagComboBox->addItem(tr("Erase if CheckBlank Failed"), (int)ChipOperCfgSubCmdID::EraseIfBlankCheckFailed);
	ui->progPage_eraseTagComboBox->setCurrentIndex(2); //默认展示Erase if CheckBlank Failed
	ui->progPage_eraseTagWgt->setFlowLabelText(tr("Erase if CheckBlank Failed"));
	connect(ui->progPage_eraseTagComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AngKProjectOperation::onSlotChangeWidget);

	ui->progPage_blankTagWgt->InitWidget(tr("Blank"), m_mapOperatePath[OperationTagType::Blank], OperationTagType::Blank);
	ui->progPage_blankTagComboBox->addItem(tr("Disabled"), (int)ChipOperCfgSubCmdID::UnEnable);
	ui->progPage_blankTagComboBox->addItem(tr("Blank Check"), (int)ChipOperCfgSubCmdID::SubBlankCheck);
	ui->progPage_blankTagComboBox->setCurrentIndex(1); //默认展示Blank Check
	ui->progPage_blankTagWgt->setFlowLabelText(tr("Blank Check"));
	connect(ui->progPage_blankTagComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AngKProjectOperation::onSlotChangeWidget);

	ui->progPage_progTagWgt->InitWidget(tr("Program"), m_mapOperatePath[OperationTagType::Program], OperationTagType::Program);
	ui->progPage_progComboBox->addItem(tr("Enabled"), (int)ChipOperCfgSubCmdID::SubProgram);
	ui->progPage_progComboBox->setEnabled(false);
	ui->progPage_progTagWgt->setFlowLabelText(tr("Program"));

	ui->progPage_verifyTagWgt->InitWidget(tr("Verify"), m_mapOperatePath[OperationTagType::Verify], OperationTagType::Verify);
	ui->progPage_verifyTagComboBox->addItem(tr("Disabled"), (int)ChipOperCfgSubCmdID::UnEnable);
	ui->progPage_verifyTagComboBox->addItem(tr("Enabled"), (int)ChipOperCfgSubCmdID::SubVerify);
	ui->progPage_verifyTagWgt->setFlowLabelText(tr("Verify"));
	ui->progPage_verifyTagComboBox->setCurrentIndex(1); //默认展示Verify Enable
	connect(ui->progPage_verifyTagComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AngKProjectOperation::onSlotChangeWidget);

	ui->progPage_marginVerifyTagWgt->setBgWidgetProperty("drag", true);
	ui->progPage_marginVerifyTagWgt->InitWidget(tr("Margin Verify"), m_mapOperatePath[OperationTagType::Verify], OperationTagType::Verify);
	ui->progPage_marginVerifyTagComboBox->addItem(tr("Disabled"), (int)ChipOperCfgSubCmdID::UnEnable);
	ui->progPage_marginVerifyTagComboBox->addItem(tr("Hi-Vcc Verify"), (int)ChipOperCfgSubCmdID::HighVerify);
	ui->progPage_marginVerifyTagComboBox->addItem(tr("Lo-Vcc Verify"), (int)ChipOperCfgSubCmdID::LowVerify);
	ui->progPage_marginVerifyTagComboBox->addItem(tr("Hi/Lo-Vcc Verify"), (int)ChipOperCfgSubCmdID::High_Low_Verify);
	ui->progPage_marginVerifyTagComboBox->setCurrentIndex(0);//默认展示Margin Verify Disabled
	//FIXME, 这里还不支持，暂时关闭
	ui->progPage_marginVerifyTagComboBox->setEnabled(false);
	connect(ui->progPage_marginVerifyTagComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AngKProjectOperation::onSlotChangeWidget);

	ui->progPage_CompareTagWgt->InitWidget(tr("CheckSum Compare"), m_mapOperatePath[OperationTagType::CheckSum], OperationTagType::CheckSum);
	ui->progPage_CompareTagComboBox->addItem(tr("Disabled"), (int)ChipOperCfgSubCmdID::UnEnable);
	ui->progPage_CompareTagComboBox->addItem(tr("Enabled"), (int)ChipOperCfgSubCmdID::ChecksumCompare);
	ui->progPage_CompareTagComboBox->setCurrentIndex(1); //默认展示ChecksumCompare Disabled
	ui->progPage_CompareTagWgt->setFlowLabelText(tr("CheckSum Compare"));
	connect(ui->progPage_CompareTagComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AngKProjectOperation::onSlotChangeWidget);

	ui->progPage_SecureTagWgt->setBgWidgetProperty("drag", true);
	ui->progPage_SecureTagWgt->InitWidget(tr("Secure"), m_mapOperatePath[OperationTagType::Secure], OperationTagType::Secure);
	ui->progPage_SecureTagComboBox->addItem(tr("Disabled"), (int)ChipOperCfgSubCmdID::UnEnable);
	ui->progPage_SecureTagComboBox->addItem(tr("Enabled"), (int)ChipOperCfgSubCmdID::SubSecure);
	ui->progPage_SecureTagComboBox->setCurrentIndex(0);//默认展示SubSecure Enable
	ui->progPage_SecureTagWgt->setFlowLabelText(tr("Secure"));
	connect(ui->progPage_SecureTagComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AngKProjectOperation::onSlotChangeWidget);
}

void AngKProjectOperation::InitVerifyPage()
{
	m_box2Widget[ui->verifyPage_verifyTagComboBox] = ui->verifyPage_verifyTagWgt;
	m_box2Widget[ui->verifyPage_marginVerifyTagComboBox] = ui->verifyPage_marginVerifyTagWgt;

	ui->verifyPage_verifyTagWgt->InitWidget(tr("Verify"), m_mapOperatePath[OperationTagType::Verify], OperationTagType::Verify);
	ui->verifyPage_verifyTagComboBox->addItem(tr("Disabled"), (int)ChipOperCfgSubCmdID::UnEnable);
	ui->verifyPage_verifyTagComboBox->addItem(tr("Enabled"), (int)ChipOperCfgSubCmdID::SubVerify);
	ui->verifyPage_verifyTagComboBox->setEnabled(false);
	ui->verifyPage_verifyTagComboBox->setCurrentIndex(1);//默认展示Verify Enable
	connect(ui->verifyPage_verifyTagComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AngKProjectOperation::onSlotChangeWidget);

	ui->verifyPage_marginVerifyTagWgt->setBgWidgetProperty("drag", true);
	ui->verifyPage_marginVerifyTagWgt->InitWidget(tr("Margin Verify"), m_mapOperatePath[OperationTagType::Verify], OperationTagType::Verify);
	ui->verifyPage_marginVerifyTagComboBox->addItem(tr("Disabled"), (int)ChipOperCfgSubCmdID::UnEnable);
	ui->verifyPage_marginVerifyTagComboBox->addItem(tr("Hi-Vcc Verify"), (int)ChipOperCfgSubCmdID::HighVerify);
	ui->verifyPage_marginVerifyTagComboBox->addItem(tr("Lo-Vcc Verify"), (int)ChipOperCfgSubCmdID::LowVerify);
	ui->verifyPage_marginVerifyTagComboBox->addItem(tr("Hi/Lo-Vcc Verify"), (int)ChipOperCfgSubCmdID::High_Low_Verify);
	ui->verifyPage_marginVerifyTagComboBox->setCurrentIndex(0);//默认展示Margin Verify UnEnable
	connect(ui->verifyPage_marginVerifyTagComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AngKProjectOperation::onSlotChangeWidget);
}

void AngKProjectOperation::InitSecurePage()
{
	m_box2Widget[ui->securePage_VerifyTagComboBox] = ui->securePage_VerifyTagWgt;
	m_box2Widget[ui->securePage_marginVerifyTagComboBox] = ui->securePage_marginVerifyTagWgt;
	m_box2Widget[ui->securePage_secureTagComboBox] = ui->securePage_secureTagWgt;

	ui->securePage_VerifyTagWgt->InitWidget(tr("Verify"), m_mapOperatePath[OperationTagType::Verify], OperationTagType::Verify);
	ui->securePage_VerifyTagComboBox->addItem(tr("Disabled"), (int)ChipOperCfgSubCmdID::UnEnable);
	ui->securePage_VerifyTagComboBox->addItem(tr("Enabled"), (int)ChipOperCfgSubCmdID::SubVerify);
	ui->securePage_VerifyTagComboBox->setCurrentIndex(1);//默认展示Verify Enabled
	ui->securePage_VerifyTagWgt->setFlowLabelText(tr("Verify"));
	connect(ui->securePage_VerifyTagComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AngKProjectOperation::onSlotChangeWidget);

	ui->securePage_marginVerifyTagWgt->setBgWidgetProperty("drag", true);
	ui->securePage_marginVerifyTagWgt->InitWidget(tr("Margin Verify"), m_mapOperatePath[OperationTagType::Verify], OperationTagType::Verify);
	ui->securePage_marginVerifyTagComboBox->addItem(tr("Disabled"), (int)ChipOperCfgSubCmdID::UnEnable);
	ui->securePage_marginVerifyTagComboBox->addItem(tr("Hi-Vcc Verify"), (int)ChipOperCfgSubCmdID::HighVerify);
	ui->securePage_marginVerifyTagComboBox->addItem(tr("Lo-Vcc Verify"), (int)ChipOperCfgSubCmdID::LowVerify);
	ui->securePage_marginVerifyTagComboBox->addItem(tr("Hi/Lo-Vcc Verify"), (int)ChipOperCfgSubCmdID::High_Low_Verify);
	ui->securePage_marginVerifyTagComboBox->setCurrentIndex(0);//默认展示Margin Verify UnEnable
	connect(ui->securePage_marginVerifyTagComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AngKProjectOperation::onSlotChangeWidget);

	ui->securePage_secureTagWgt->InitWidget(tr("Secure"), m_mapOperatePath[OperationTagType::Secure], OperationTagType::Secure);
	ui->securePage_secureTagComboBox->addItem(tr("Disabled"), (int)ChipOperCfgSubCmdID::UnEnable);
	ui->securePage_secureTagComboBox->addItem(tr("Enabled"), (int)ChipOperCfgSubCmdID::SubSecure);
	ui->securePage_secureTagComboBox->setEnabled(false);
	ui->securePage_secureTagComboBox->setCurrentIndex(1);//默认展示Secure Enabled
	ui->securePage_secureTagWgt->setFlowLabelText(tr("Secure"));
	connect(ui->securePage_secureTagComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AngKProjectOperation::onSlotChangeWidget);
}

void AngKProjectOperation::InitReadPage()
{
	m_box2Widget[ui->readPage_readTagComboBox] = ui->readPage_readTagWgt;

	ui->readPage_readTagWgt->InitWidget(tr("Read"), m_mapOperatePath[OperationTagType::Read], OperationTagType::Read);
	ui->readPage_readTagComboBox->addItem(tr("Enabled"), (int)ChipOperCfgSubCmdID::SubRead);
	ui->readPage_readTagComboBox->setEnabled(false);
	ui->readPage_readTagWgt->setFlowLabelText(tr("Read"));
}

void AngKProjectOperation::InitSelfPage()
{
	ui->eraseTag->setDrag(true);
	ui->eraseTag->InitWidget(tr("Erase"), m_mapOperatePath[OperationTagType::Erase], OperationTagType::Erase);

	ui->blankTag->setDrag(true);
	ui->blankTag->InitWidget(tr("Blank"), m_mapOperatePath[OperationTagType::Blank], OperationTagType::Blank);

	ui->programTag->setDrag(true);
	ui->programTag->InitWidget(tr("Program"), m_mapOperatePath[OperationTagType::Program], OperationTagType::Program);

	ui->verifyTag->setDrag(true);
	ui->verifyTag->InitWidget(tr("Verify"), m_mapOperatePath[OperationTagType::Verify], OperationTagType::Verify);

	ui->secureTag->setDrag(true);
	ui->secureTag->InitWidget(tr("Secure"), m_mapOperatePath[OperationTagType::Secure], OperationTagType::Secure);

	ui->readTag->setDrag(true);
	ui->readTag->InitWidget(tr("Read"), m_mapOperatePath[OperationTagType::Read], OperationTagType::Read);

	ui->eraseTag->setBgWidgetProperty("drag");
	ui->blankTag->setBgWidgetProperty("drag");
	ui->programTag->setBgWidgetProperty("drag");
	ui->verifyTag->setBgWidgetProperty("drag");
	ui->secureTag->setBgWidgetProperty("drag");
	ui->readTag->setBgWidgetProperty("drag");

	m_box2Widget[ui->seqComboBox_1] = ui->seqWidget_1;
	m_box2Widget[ui->seqComboBox_2] = ui->seqWidget_2;
	m_box2Widget[ui->seqComboBox_3] = ui->seqWidget_3;
	m_box2Widget[ui->seqComboBox_4] = ui->seqWidget_4;
	m_box2Widget[ui->seqComboBox_5] = ui->seqWidget_5;
	m_box2Widget[ui->seqComboBox_6] = ui->seqWidget_6;
	m_box2Widget[ui->seqComboBox_7] = ui->seqWidget_7;

	connect(ui->seqWidget_1, &AngKOperationTag::sgnDropSymbolType, this, &AngKProjectOperation::onSlotDropSymbolType);
	connect(ui->seqWidget_2, &AngKOperationTag::sgnDropSymbolType, this, &AngKProjectOperation::onSlotDropSymbolType);
	connect(ui->seqWidget_3, &AngKOperationTag::sgnDropSymbolType, this, &AngKProjectOperation::onSlotDropSymbolType);
	connect(ui->seqWidget_4, &AngKOperationTag::sgnDropSymbolType, this, &AngKProjectOperation::onSlotDropSymbolType);
	connect(ui->seqWidget_5, &AngKOperationTag::sgnDropSymbolType, this, &AngKProjectOperation::onSlotDropSymbolType);
	connect(ui->seqWidget_6, &AngKOperationTag::sgnDropSymbolType, this, &AngKProjectOperation::onSlotDropSymbolType);
	connect(ui->seqWidget_7, &AngKOperationTag::sgnDropSymbolType, this, &AngKProjectOperation::onSlotDropSymbolType);

	m_widget2Box[ui->seqWidget_1] = ui->seqComboBox_1;
	m_widget2Box[ui->seqWidget_2] = ui->seqComboBox_2;
	m_widget2Box[ui->seqWidget_3] = ui->seqComboBox_3;
	m_widget2Box[ui->seqWidget_4] = ui->seqComboBox_4;
	m_widget2Box[ui->seqWidget_5] = ui->seqComboBox_5;
	m_widget2Box[ui->seqWidget_6] = ui->seqComboBox_6;
	m_widget2Box[ui->seqWidget_7] = ui->seqComboBox_7;

	ui->seqWidget_1->setRightClick(true);
	ui->seqWidget_2->setRightClick(true);
	ui->seqWidget_3->setRightClick(true);
	ui->seqWidget_4->setRightClick(true);
	ui->seqWidget_5->setRightClick(true);
	ui->seqWidget_6->setRightClick(true);
	ui->seqWidget_7->setRightClick(true);
}

void AngKProjectOperation::InitWidgetInfo()
{
	InitErasePage();
	InitBlankPage();
	InitProgramPage();
	InitVerifyPage();
	InitSecurePage();
	InitReadPage();
	InitSelfPage();
}

void AngKProjectOperation::InitButtonGroup()
{
	m_pageGroup = new QButtonGroup(ui->buttonWidget);

	connect(m_pageGroup, static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked), [&](int index) 
	{
		ui->stackedWidget->setCurrentIndex(index - 1);
		ui->plainTextEdit->clear();
		ui->plainTextEdit->appendPlainText(tr("Operating Flow:"));
		GetPageFlow(index - 1);
	});

	m_pageGroup->addButton(ui->eraseButton, (int)OperationTagType::Erase);
	m_pageGroup->addButton(ui->blankButton, (int)OperationTagType::Blank);
	m_pageGroup->addButton(ui->programButton, (int)OperationTagType::Program);
	m_pageGroup->addButton(ui->verifyButton, (int)OperationTagType::Verify);
	m_pageGroup->addButton(ui->secureButton, (int)OperationTagType::Secure);
	m_pageGroup->addButton(ui->readButton, (int)OperationTagType::Read);
	m_pageGroup->addButton(ui->selfButton, (int)OperationTagType::Self);

	ui->eraseButton->setChecked(true);
}

void AngKProjectOperation::GetPageFlow(int nPage)
{
	switch (nPage)
	{
	case ERASE_PAGE:
		GetErasePageFlow();
	break;
	case BLANK_PAGE:
		GetBlankPageFlow();
		break;
	case PROG_PAGE:
		GetProgramPageFlow();
		break;
	case VERIFY_PAGE:
		GetVerifyPageFlow();
		break;
	case SECURE_PAGE:
		GetSecurePageFlow();
		break;
	case READ_PAGE:
		GetReadPageFlow();
		break;
	case SELF_PAGE:
		GetSelfPageFlow();
		break;
	default:
		break;
	}
}

void AngKProjectOperation::GetErasePageFlow()
{
	ui->erasePage_eraseTagComboBox->currentData().toInt() > 0 ? ui->plainTextEdit->appendPlainText(m_box2Widget[ui->erasePage_eraseTagComboBox]->getFlowLabelText()) : false;
	ui->erasePage_blankTagComboBox->currentData().toInt() > 0 ? ui->plainTextEdit->appendPlainText(m_box2Widget[ui->erasePage_blankTagComboBox]->getFlowLabelText()) : false;
}

void AngKProjectOperation::GetBlankPageFlow()
{
	ui->blankPage_blankTagComboBox->currentData().toInt() > 0 ? ui->plainTextEdit->appendPlainText(m_box2Widget[ui->blankPage_blankTagComboBox]->getFlowLabelText()) : false;
}

void AngKProjectOperation::GetProgramPageFlow()
{
	ui->progPage_eraseTagComboBox->currentData().toInt() > 0 ? ui->plainTextEdit->appendPlainText(m_box2Widget[ui->progPage_eraseTagComboBox]->getFlowLabelText()) : false;
	ui->progPage_blankTagComboBox->currentData().toInt() > 0 ? ui->plainTextEdit->appendPlainText(m_box2Widget[ui->progPage_blankTagComboBox]->getFlowLabelText()) : false;
	ui->progPage_progComboBox->currentData().toInt() > 0 ? ui->plainTextEdit->appendPlainText(m_box2Widget[ui->progPage_progComboBox]->getFlowLabelText()) : false;
	ui->progPage_verifyTagComboBox->currentData().toInt() > 0 ? ui->plainTextEdit->appendPlainText(m_box2Widget[ui->progPage_verifyTagComboBox]->getFlowLabelText()) : false;
	ui->progPage_marginVerifyTagComboBox->currentData().toInt() > 0 ? ui->plainTextEdit->appendPlainText(m_box2Widget[ui->progPage_marginVerifyTagComboBox]->getFlowLabelText()) : false;
	ui->progPage_CompareTagComboBox->currentData().toInt() > 0 ? ui->plainTextEdit->appendPlainText(m_box2Widget[ui->progPage_CompareTagComboBox]->getFlowLabelText()) : false;
	ui->progPage_SecureTagComboBox->currentData().toInt() > 0 ? ui->plainTextEdit->appendPlainText(m_box2Widget[ui->progPage_SecureTagComboBox]->getFlowLabelText()) : false;
}

void AngKProjectOperation::GetVerifyPageFlow()
{
	ui->verifyPage_verifyTagComboBox->currentData().toInt() > 0 ? ui->plainTextEdit->appendPlainText(m_box2Widget[ui->verifyPage_verifyTagComboBox]->getFlowLabelText()) : false;
	ui->verifyPage_marginVerifyTagComboBox->currentData().toInt() > 0 ? ui->plainTextEdit->appendPlainText(m_box2Widget[ui->verifyPage_marginVerifyTagComboBox]->getFlowLabelText()) : false;
}

void AngKProjectOperation::GetSecurePageFlow()
{
	ui->securePage_VerifyTagComboBox->currentData().toInt() > 0 ? ui->plainTextEdit->appendPlainText(m_box2Widget[ui->securePage_VerifyTagComboBox]->getFlowLabelText()) : false;
	ui->securePage_marginVerifyTagComboBox->currentData().toInt() > 0 ? ui->plainTextEdit->appendPlainText(m_box2Widget[ui->securePage_marginVerifyTagComboBox]->getFlowLabelText()) : false;
	ui->securePage_secureTagComboBox->currentData().toInt() > 0 ? ui->plainTextEdit->appendPlainText(m_box2Widget[ui->securePage_secureTagComboBox]->getFlowLabelText()) : false;
}

void AngKProjectOperation::GetReadPageFlow()
{
	ui->readPage_readTagComboBox->currentData().toInt() > 0 ? ui->plainTextEdit->appendPlainText(m_box2Widget[ui->readPage_readTagComboBox]->getFlowLabelText()) : false;
}

void AngKProjectOperation::GetSelfPageFlow()
{
	ui->plainTextEdit->clear();
	ui->plainTextEdit->appendPlainText(tr("Operating Flow:"));

	ui->seqComboBox_1->currentData().toInt() > 0 ? ui->plainTextEdit->appendPlainText(m_box2Widget[ui->seqComboBox_1]->getFlowLabelText()) : false;
	ui->seqComboBox_2->currentData().toInt() > 0 ? ui->plainTextEdit->appendPlainText(m_box2Widget[ui->seqComboBox_2]->getFlowLabelText()) : false;
	ui->seqComboBox_3->currentData().toInt() > 0 ? ui->plainTextEdit->appendPlainText(m_box2Widget[ui->seqComboBox_3]->getFlowLabelText()) : false;
	ui->seqComboBox_4->currentData().toInt() > 0 ? ui->plainTextEdit->appendPlainText(m_box2Widget[ui->seqComboBox_4]->getFlowLabelText()) : false;
	ui->seqComboBox_5->currentData().toInt() > 0 ? ui->plainTextEdit->appendPlainText(m_box2Widget[ui->seqComboBox_5]->getFlowLabelText()) : false;
	ui->seqComboBox_6->currentData().toInt() > 0 ? ui->plainTextEdit->appendPlainText(m_box2Widget[ui->seqComboBox_6]->getFlowLabelText()) : false;
	ui->seqComboBox_7->currentData().toInt() > 0 ? ui->plainTextEdit->appendPlainText(m_box2Widget[ui->seqComboBox_7]->getFlowLabelText()) : false;
}

void AngKProjectOperation::SaveOperationCfg(OpInfoList& operList)
{
	operList.clear();

	bool bEraseRight = m_stuOperJson.baseInfo.bErase;
	bool bBlankRight = m_stuOperJson.baseInfo.bBlank;
	bool bProgRight = m_stuOperJson.baseInfo.bProg;
	bool bVerifyRight = m_stuOperJson.baseInfo.bVerify;
	bool bSecureRight = m_stuOperJson.baseInfo.bSecure;
	bool bReadRight = m_stuOperJson.baseInfo.bRead;
	bool bMarginVerifyRight = m_stuOperJson.otherInfo.bH_L_VccVerify;
	bool bChecksumCompareRight = m_stuOperJson.otherInfo.bCompare;

	//erasePage
	if (bEraseRight)
	{
		OperatorInfo eraseInfo;
		eraseInfo.strOpName = TranslateProgOperType(OperationTagType::Erase);
		eraseInfo.iOpId = ChipOperCfgCmdID::Erase;
		int eraseData = ui->erasePage_eraseTagComboBox->currentData().toInt();
		int blankData = ui->erasePage_blankTagComboBox->currentData().toInt();

		if (eraseData > 0)
			eraseInfo.vecOpList.push_back(eraseData);
		if(bBlankRight && blankData > 0)
			eraseInfo.vecOpList.push_back(blankData);

		operList.push_back(eraseInfo);
	}

	//blankPage
	if (bBlankRight)
	{
		OperatorInfo blankInfo;
		blankInfo.strOpName = TranslateProgOperType(OperationTagType::Blank);
		blankInfo.iOpId = ChipOperCfgCmdID::BlankCheck;
		int blankData = ui->blankPage_blankTagComboBox->currentData().toInt();
		if(blankData > 0)
			blankInfo.vecOpList.push_back(blankData);

		operList.push_back(blankInfo);
	}

	//programPage
	if (bProgRight)
	{
		OperatorInfo programInfo;
		programInfo.strOpName = TranslateProgOperType(OperationTagType::Program);
		programInfo.iOpId = ChipOperCfgCmdID::Program;
		int eraseData = ui->progPage_eraseTagComboBox->currentData().toInt();
		int blankData = ui->progPage_blankTagComboBox->currentData().toInt();
		int progData = ui->progPage_progComboBox->currentData().toInt();
		int verifyData = ui->progPage_verifyTagComboBox->currentData().toInt();
		int marginVerifyData = ui->progPage_marginVerifyTagComboBox->currentData().toInt();
		int compareData = ui->progPage_CompareTagComboBox->currentData().toInt();
		int secureData = ui->progPage_SecureTagComboBox->currentData().toInt();

		if (bEraseRight && eraseData > 0)
			programInfo.vecOpList.push_back(eraseData);
		if (bBlankRight && blankData > 0)
			programInfo.vecOpList.push_back(blankData);
		if (progData > 0)
			programInfo.vecOpList.push_back(progData);
		if (bVerifyRight && verifyData > 0)
			programInfo.vecOpList.push_back(verifyData);
		if (bMarginVerifyRight && marginVerifyData > 0)
			programInfo.vecOpList.push_back(marginVerifyData);
		if (bChecksumCompareRight && compareData > 0)
			programInfo.vecOpList.push_back(compareData);
		if (bSecureRight && secureData > 0)
			programInfo.vecOpList.push_back(secureData);

		operList.push_back(programInfo);
	}
	//verifyPage
	if (bVerifyRight)
	{
		OperatorInfo verifyInfo;
		verifyInfo.strOpName = TranslateProgOperType(OperationTagType::Verify);
		verifyInfo.iOpId = ChipOperCfgCmdID::Verify;
		int verifyData = ui->verifyPage_verifyTagComboBox->currentData().toInt();
		int marginVerifyData = ui->verifyPage_marginVerifyTagComboBox->currentData().toInt();

		if(verifyData > 0)
			verifyInfo.vecOpList.push_back(verifyData);
		if (bMarginVerifyRight && marginVerifyData > 0)
			verifyInfo.vecOpList.push_back(marginVerifyData);

		operList.push_back(verifyInfo);
	}

	//securePage
	if (bSecureRight)
	{
		OperatorInfo secureInfo;
		secureInfo.strOpName = TranslateProgOperType(OperationTagType::Secure);
		secureInfo.iOpId = ChipOperCfgCmdID::Secure;
		int verifyData = ui->securePage_VerifyTagComboBox->currentData().toInt();
		int marginVerifyData = ui->securePage_marginVerifyTagComboBox->currentData().toInt();
		int secureData = ui->securePage_secureTagComboBox->currentData().toInt();

		if (bVerifyRight && verifyData > 0)
			secureInfo.vecOpList.push_back(verifyData);
		if (bMarginVerifyRight && marginVerifyData > 0)
			secureInfo.vecOpList.push_back(marginVerifyData);
		if (secureData > 0)
			secureInfo.vecOpList.push_back(secureData);

		operList.push_back(secureInfo);
	}

	//readPage
	if (bReadRight)
	{
		OperatorInfo readInfo;
		readInfo.strOpName = TranslateProgOperType(OperationTagType::Read);
		readInfo.iOpId = ChipOperCfgCmdID::Read;
		int readData = ui->readPage_readTagComboBox->currentData().toInt();
		if (readData > 0)
			readInfo.vecOpList.push_back(readData);

		operList.push_back(readInfo);
	}

	//selfPage
	OperatorInfo selfInfo;
	selfInfo.strOpName = TranslateProgOperType(OperationTagType::Self);
	selfInfo.iOpId = ChipOperCfgCmdID::Custom;
	int seqComData_1 = ui->seqComboBox_1->currentData().toInt();
	int seqComData_2 = ui->seqComboBox_2->currentData().toInt();
	int seqComData_3 = ui->seqComboBox_3->currentData().toInt();
	int seqComData_4 = ui->seqComboBox_4->currentData().toInt();
	int seqComData_5 = ui->seqComboBox_5->currentData().toInt();
	int seqComData_6 = ui->seqComboBox_6->currentData().toInt();
	int seqComData_7 = ui->seqComboBox_7->currentData().toInt();

	seqComData_1 > 0 ? selfInfo.vecOpList.push_back(seqComData_1) : false;
	seqComData_2 > 0 ? selfInfo.vecOpList.push_back(seqComData_2) : false;
	seqComData_3 > 0 ? selfInfo.vecOpList.push_back(seqComData_3) : false;
	seqComData_4 > 0 ? selfInfo.vecOpList.push_back(seqComData_4) : false;
	seqComData_5 > 0 ? selfInfo.vecOpList.push_back(seqComData_5) : false;
	seqComData_6 > 0 ? selfInfo.vecOpList.push_back(seqComData_6) : false;
	seqComData_7 > 0 ? selfInfo.vecOpList.push_back(seqComData_7) : false;
	operList.push_back(selfInfo);

}

void AngKProjectOperation::CreateOperToolWgt(std::string cfgJson)
{
	try {//nlohmann解析失败会报异常需要捕获一下
		if (cfgJson.empty())
			return;

		nlohmann::json _CfgJson = nlohmann::json::parse(cfgJson);

		//baseOper
		nlohmann::json baseOperJson = _CfgJson["baseOper"];
		m_stuOperJson.baseInfo.bBlank = baseOperJson["blank"];
		m_stuOperJson.baseInfo.bBlockProg = baseOperJson["blockProg"];
		m_stuOperJson.baseInfo.bErase = baseOperJson["erase"];
		m_stuOperJson.baseInfo.bFunction = baseOperJson["function"];
		m_stuOperJson.baseInfo.bIllegalBit = baseOperJson["illegalBit"];
		m_stuOperJson.baseInfo.bProg = baseOperJson["prog"];
		m_stuOperJson.baseInfo.bRead = baseOperJson["read"];
		m_stuOperJson.baseInfo.bSecure = baseOperJson["secure"];
		m_stuOperJson.baseInfo.bVerify = baseOperJson["verify"];

		//bitsOper
		nlohmann::json bitsOperJson = _CfgJson["bitsOper"];
		m_stuOperJson.bitsInfo.bBit4 = bitsOperJson["bit4"];
		m_stuOperJson.bitsInfo.bBit8 = bitsOperJson["bit8"];
		m_stuOperJson.bitsInfo.bBit12 = bitsOperJson["bit12"];
		m_stuOperJson.bitsInfo.bBit16 = bitsOperJson["bit16"];

		//checkSumOper
		nlohmann::json checkSumOperJson = _CfgJson["checkSumOper"];
		m_stuOperJson.checksumInfo.bBytesum = checkSumOperJson["sum"];
		m_stuOperJson.checksumInfo.bWordsum = checkSumOperJson["wordSum"];
		m_stuOperJson.checksumInfo.bCRC16 = checkSumOperJson["crc16"];
		m_stuOperJson.checksumInfo.bCRC32 = checkSumOperJson["crc32"];

		//fileLoadOper
		nlohmann::json fileLoadOperJson = _CfgJson["fileLoadOper"];
		m_stuOperJson.fileLoadInfo.bBigEndian = fileLoadOperJson["bigEndian"];
		m_stuOperJson.fileLoadInfo.bWordAddress = fileLoadOperJson["wordAddress"];

		//otherOper
		nlohmann::json otherOperJson = _CfgJson["otherOper"];
		m_stuOperJson.otherInfo.bEEPROM = otherOperJson["EEPROM"];
		m_stuOperJson.otherInfo.bIDCheck = otherOperJson["IDCheck"];
		m_stuOperJson.otherInfo.bAddressRelocate = otherOperJson["addressRelocate"];
		m_stuOperJson.otherInfo.bCompare = otherOperJson["compare"];
		m_stuOperJson.otherInfo.bEmptyBuffer = otherOperJson["emptyBuffer"];
		m_stuOperJson.otherInfo.bEnableSN = otherOperJson["enableSN"];
		m_stuOperJson.otherInfo.bInsection = otherOperJson["insection"];
		m_stuOperJson.otherInfo.bLoopFun = otherOperJson["loopFun"];
		m_stuOperJson.otherInfo.bMasterCopy = otherOperJson["masterCopy"];
		m_stuOperJson.otherInfo.bOnline = otherOperJson["online"];
		m_stuOperJson.otherInfo.bPin = otherOperJson["pin"];
		m_stuOperJson.otherInfo.bProtect = otherOperJson["protect"];
		m_stuOperJson.otherInfo.bunTest = otherOperJson["unTest"];
		m_stuOperJson.otherInfo.bH_L_VccVerify = otherOperJson["vccVerify"];

	}
	catch (const nlohmann::json::exception& e) {
		m_stuOperJson.clear();
		ALOG_FATAL("CreateOperToolWgt Json parse failed : %s.", "CU", "--", e.what());
	}

	//根据操作命令动态改变Tag显示
	DynamicChangeOperPage();
}

void AngKProjectOperation::ChipOper2UI_ProjJson(std::string& chipOperJson)
{
	nlohmann::json chipOperUIJson;

	//Erase
	nlohmann::json eraseJson;
	if (!ui->eraseButton->isHidden()) {
		eraseJson["Enable"] = true;
		eraseJson["BlankCheck"] = ui->erasePage_blankTagComboBox->currentIndex();
		eraseJson["EraseData"] = ui->erasePage_eraseTagComboBox->currentData().toInt();
		eraseJson["BlankCheckData"] = ui->erasePage_blankTagComboBox->currentData().toInt();
	}
	else {
		eraseJson["Enable"] = false;
	}
	chipOperUIJson["Erase"] = eraseJson;

	//Blank
	nlohmann::json blankJson;
	if (!ui->blankButton->isHidden()) {
		blankJson["Enable"] = true;
		blankJson["BlankCheckData"] = ui->blankPage_blankTagComboBox->currentData().toInt();
	}
	else {
		blankJson["Enable"] = false;
	}
	chipOperUIJson["Blank"] = blankJson;

	//Program
	nlohmann::json programJson;
	if (!ui->programButton->isHidden()) {
		programJson["Enable"] = true;
		programJson["Erase"] = ui->progPage_eraseTagComboBox->currentIndex();
		programJson["BlankCheck"] = ui->progPage_blankTagComboBox->currentIndex();
		programJson["Verify"] = ui->progPage_verifyTagComboBox->currentIndex();
		programJson["MarginVerify"] = ui->progPage_marginVerifyTagComboBox->currentIndex();
		programJson["Secure"] = ui->progPage_SecureTagComboBox->currentIndex();

		programJson["EraseData"] = ui->progPage_eraseTagComboBox->currentData().toInt();
		programJson["BlankCheckData"] = ui->progPage_blankTagComboBox->currentData().toInt();
		programJson["ProgramData"] = ui->progPage_progComboBox->currentData().toInt();
		programJson["VerifyData"] = ui->progPage_verifyTagComboBox->currentData().toInt();
		programJson["MarginVerifyData"] = ui->progPage_marginVerifyTagComboBox->currentData().toInt();
		programJson["SecureData"] = ui->progPage_SecureTagComboBox->currentData().toInt();
	}
	else {
		programJson["Enable"] = false;
	}
	chipOperUIJson["Program"] = programJson;

	//Verify
	nlohmann::json verifyJson;
	if (!ui->verifyButton->isHidden()) {
		verifyJson["Enable"] = true;
		verifyJson["MarginVerify"] = ui->verifyPage_marginVerifyTagComboBox->currentIndex();
		verifyJson["VerifyData"] = ui->verifyPage_verifyTagComboBox->currentData().toInt();
		verifyJson["MarginVerifyData"] = ui->verifyPage_marginVerifyTagComboBox->currentData().toInt();
	}
	else {
		verifyJson["Enable"] = false;
	}
	chipOperUIJson["Verify"] = verifyJson;

	//Secure
	nlohmann::json secureJson;
	if (!ui->secureButton->isHidden()) {
		secureJson["Enable"] = true;
		secureJson["Verify"] = ui->securePage_VerifyTagComboBox->currentIndex();
		secureJson["MarginVerify"] = ui->securePage_marginVerifyTagComboBox->currentIndex();

		secureJson["SecureData"] = ui->securePage_secureTagComboBox->currentData().toInt();
		secureJson["VerifyData"] = ui->securePage_marginVerifyTagComboBox->currentData().toInt();
		secureJson["MarginVerifyData"] = ui->securePage_marginVerifyTagComboBox->currentData().toInt();
	}
	else {
		secureJson["Enable"] = false;
	}
	chipOperUIJson["Secure"] = secureJson;

	//Read
	nlohmann::json readJson;
	if (!ui->readButton->isHidden()) {
		readJson["Enable"] = true;
		readJson["ReadData"] = ui->readPage_readTagComboBox->currentData().toInt();
	}
	else {
		readJson["Enable"] = false;
	}
	chipOperUIJson["Read"] = readJson;

	//Self
	nlohmann::json selfJson;
	if (!ui->selfButton->isHidden()) {
		selfJson["Enable"] = true;
		selfJson["selfCmdType1"] = ui->seqWidget_1->GetOperationTagType();
		selfJson["selfCmd1"] = ui->seqComboBox_1->currentIndex();
		selfJson["selfCmd1Data"] = ui->seqComboBox_1->currentData().toInt();

		selfJson["selfCmdType2"] = ui->seqWidget_2->GetOperationTagType();
		selfJson["selfCmd2"] = ui->seqComboBox_2->currentIndex();
		selfJson["selfCmd2Data"] = ui->seqComboBox_2->currentData().toInt();


		selfJson["selfCmdType3"] = ui->seqWidget_3->GetOperationTagType();
		selfJson["selfCmd3"] = ui->seqComboBox_3->currentIndex();
		selfJson["selfCmd3Data"] = ui->seqComboBox_3->currentData().toInt();


		selfJson["selfCmdType4"] = ui->seqWidget_4->GetOperationTagType();
		selfJson["selfCmd4"] = ui->seqComboBox_4->currentIndex();
		selfJson["selfCmd4Data"] = ui->seqComboBox_4->currentData().toInt();


		selfJson["selfCmdType5"] = ui->seqWidget_5->GetOperationTagType();
		selfJson["selfCmd5"] = ui->seqComboBox_5->currentIndex();
		selfJson["selfCmd5Data"] = ui->seqComboBox_5->currentData().toInt();


		selfJson["selfCmdType6"] = ui->seqWidget_6->GetOperationTagType();
		selfJson["selfCmd6"] = ui->seqComboBox_6->currentIndex();
		selfJson["selfCmd6Data"] = ui->seqComboBox_6->currentData().toInt();


		selfJson["selfCmdType7"] = ui->seqWidget_7->GetOperationTagType();
		selfJson["selfCmd7"] = ui->seqComboBox_7->currentIndex();
		selfJson["selfCmd7Data"] = ui->seqComboBox_7->currentData().toInt();
	}
	else {
		selfJson["Enable"] = false;
	}
	chipOperUIJson["Self"] = selfJson;

	chipOperJson = chipOperUIJson.dump();
}

void AngKProjectOperation::UI2ChipOper_ProjJson(std::string chipOperJson)
{
	try {//nlohmann解析失败会报异常需要捕获一下
		auto operJson = nlohmann::json::parse(chipOperJson);
		//Erase
		{
			nlohmann::json eraseJson = operJson["Erase"];
			ui->eraseButton->setHidden(!eraseJson["Enable"].get<bool>());
			if (eraseJson["Enable"].get<bool>()) {
				//设置其他子命令
				ui->erasePage_blankTagComboBox->setCurrentIndex(eraseJson["BlankCheck"].get<int>());
			}
		}

		//Blank
		{
			nlohmann::json blankJson = operJson["Blank"];
			ui->blankButton->setHidden(!blankJson["Enable"].get<bool>());
		}

		//Program
		{
			nlohmann::json programJson = operJson["Program"];
			ui->programButton->setHidden(!programJson["Enable"].get<bool>());
			if (programJson["Enable"].get<bool>()) {
				ui->progPage_eraseTagComboBox->setCurrentIndex(programJson["Erase"].get<int>());
				ui->progPage_blankTagComboBox->setCurrentIndex(programJson["BlankCheck"].get<int>());
				ui->progPage_verifyTagComboBox->setCurrentIndex(programJson["Verify"].get<int>());
				ui->progPage_marginVerifyTagComboBox->setCurrentIndex(programJson["MarginVerify"].get<int>());
				ui->progPage_SecureTagComboBox->setCurrentIndex(programJson["Secure"].get<int>());
			}
		}

		//Verify
		{
			nlohmann::json verifyJson = operJson["Verify"];
			ui->verifyButton->setHidden(!verifyJson["Enable"].get<bool>());
			if (verifyJson["Enable"].get<bool>()) {
				//设置其他子命令
				ui->verifyPage_marginVerifyTagComboBox->setCurrentIndex(verifyJson["MarginVerify"].get<int>());
			}
		}

		//Secure
		{
			nlohmann::json secureJson = operJson["Secure"];
			ui->secureButton->setHidden(!secureJson["Enable"].get<bool>());
			if (secureJson["Enable"].get<bool>()) {
				//设置其他子命令
				ui->securePage_VerifyTagComboBox->setCurrentIndex(secureJson["Verify"].get<int>());
				ui->securePage_marginVerifyTagComboBox->setCurrentIndex(secureJson["MarginVerify"].get<int>());
			}
		}

		//Read
		{
			nlohmann::json readJson = operJson["Read"];
			ui->readButton->setHidden(!readJson["Enable"].get<bool>());
		}

		//Self
		{
			nlohmann::json selfJson = operJson["Self"];
			ui->selfButton->setHidden(!selfJson["Enable"].get<bool>());
			if (selfJson["Enable"].get<bool>()) {
				//设置其他子命令
				OperationTagType selfOperType = (OperationTagType)selfJson["selfCmdType1"].get<int>();
				std::string selfOperStr = TranslateProgOperType(selfOperType);
				if (selfOperStr != ""){
					onSlotDropSymbolType(ui->seqWidget_1, (int)selfOperType);
					ui->seqWidget_1->setCurOperateType(selfOperType);
					ui->seqWidget_1->PaintSymbolLabel(QString::fromStdString(selfOperStr));
					ui->seqComboBox_1->setCurrentIndex(selfJson["selfCmd1"].get<int>());
				}
				selfOperType = (OperationTagType)selfJson["selfCmdType2"].get<int>();
				selfOperStr = TranslateProgOperType(selfOperType);
				if (selfOperStr != "") {
					onSlotDropSymbolType(ui->seqWidget_2, (int)selfOperType);
					ui->seqWidget_2->setCurOperateType(selfOperType);
					ui->seqWidget_2->PaintSymbolLabel(QString::fromStdString(selfOperStr));
					ui->seqComboBox_2->setCurrentIndex(selfJson["selfCmd2"].get<int>());
				}
				selfOperType = (OperationTagType)selfJson["selfCmdType3"].get<int>();
				selfOperStr = TranslateProgOperType(selfOperType);
				if (selfOperStr != "") {
					onSlotDropSymbolType(ui->seqWidget_3, (int)selfOperType);
					ui->seqWidget_3->setCurOperateType(selfOperType);
					ui->seqWidget_3->PaintSymbolLabel(QString::fromStdString(selfOperStr));
					ui->seqComboBox_3->setCurrentIndex(selfJson["selfCmd3"].get<int>());
				}
				selfOperType = (OperationTagType)selfJson["selfCmdType4"].get<int>();
				selfOperStr = TranslateProgOperType(selfOperType);
				if (selfOperStr != "") {
					onSlotDropSymbolType(ui->seqWidget_4, (int)selfOperType);
					ui->seqWidget_4->setCurOperateType(selfOperType);
					ui->seqWidget_4->PaintSymbolLabel(QString::fromStdString(selfOperStr));
					ui->seqComboBox_4->setCurrentIndex(selfJson["selfCmd4"].get<int>());
				}
				selfOperType = (OperationTagType)selfJson["selfCmdType5"].get<int>();
				selfOperStr = TranslateProgOperType(selfOperType);
				if (selfOperStr != "") {
					onSlotDropSymbolType(ui->seqWidget_5, (int)selfOperType);
					ui->seqWidget_5->setCurOperateType(selfOperType);
					ui->seqWidget_5->PaintSymbolLabel(QString::fromStdString(selfOperStr));
					ui->seqComboBox_5->setCurrentIndex(selfJson["selfCmd5"].get<int>());
				}
				selfOperType = (OperationTagType)selfJson["selfCmdType6"].get<int>();
				selfOperStr = TranslateProgOperType(selfOperType);
				if (selfOperStr != "") {
					onSlotDropSymbolType(ui->seqWidget_6, (int)selfOperType);
					ui->seqWidget_6->setCurOperateType(selfOperType);
					ui->seqWidget_6->PaintSymbolLabel(QString::fromStdString(selfOperStr));
					ui->seqComboBox_6->setCurrentIndex(selfJson["selfCmd6"].get<int>());
				}
				selfOperType = (OperationTagType)selfJson["selfCmdType7"].get<int>();
				selfOperStr = TranslateProgOperType(selfOperType);
				if (selfOperStr != "") {
					onSlotDropSymbolType(ui->seqWidget_7, (int)selfOperType);
					ui->seqWidget_7->setCurOperateType(selfOperType);
					ui->seqWidget_7->PaintSymbolLabel(QString::fromStdString(selfOperStr));
					ui->seqComboBox_7->setCurrentIndex(selfJson["selfCmd7"].get<int>());
				}
			}
		}
	}
	catch (const nlohmann::json::exception& e) {
		ALOG_FATAL("chipOper Json parse failed : %s.", "CU", "--", e.what());
	}
}

std::string AngKProjectOperation::TranslateProgOperType(OperationTagType operTagType)
{
	QString operStr;

	switch (operTagType)
	{
	case OperationTagType::Erase:
		operStr = "Erase";
		break;
	case OperationTagType::Blank:
		operStr = "Blank";
		break;
	case OperationTagType::Program:
		operStr = "Program";
		break;
	case OperationTagType::Verify:
		operStr = "Verify";
		break;
	case OperationTagType::Secure:
		operStr = "Secure";
		break;
	case OperationTagType::Read:
		operStr = "Read";
		break;
	case OperationTagType::CheckSum:
		operStr = "CheckSum";
		break;
	case OperationTagType::Self:
		operStr = "Self";
		break;
	case OperationTagType::None:
	default:
		break;
	}

	return operStr.toStdString();
}

void AngKProjectOperation::DynamicChangeOperPage()
{
	bool bEraseRight = m_stuOperJson.baseInfo.bErase;
	bool bBlankRight = m_stuOperJson.baseInfo.bBlank;
	bool bProgRight = m_stuOperJson.baseInfo.bProg;
	bool bVerifyRight = m_stuOperJson.baseInfo.bVerify;
	bool bSecureRight = m_stuOperJson.baseInfo.bSecure;
	bool bReadRight = m_stuOperJson.baseInfo.bRead;
	bool bMarginVerifyRight = m_stuOperJson.otherInfo.bH_L_VccVerify;
	bool bChecksumCompareRight = m_stuOperJson.otherInfo.bCompare;

	bool defaultShow = false;
	OperationTagType defaultTag = OperationTagType::None;

	//erasePage
	ui->eraseButton->setHidden(!bEraseRight);
	if (!bEraseRight && !defaultShow){
		ui->erasePage->setHidden(!bEraseRight);
	}

	if (bEraseRight){
		if (!bBlankRight) {
			ui->erase_BlankSelectWgt->setHidden(!bBlankRight);
		}
		if (!defaultShow) {
			defaultShow = true;
			defaultTag = OperationTagType::Erase;
		}
	}

	//blankPage
	ui->blankButton->setHidden(!bBlankRight);
	if (!bBlankRight && !defaultShow) {
		ui->blankPage->setHidden(!bBlankRight);
	}
	if (bBlankRight) {
		if (!defaultShow) {
			defaultShow = true;
			defaultTag = OperationTagType::Blank;
		}
	}

	//ProgramPage
	ui->programButton->setHidden(!bProgRight);
	if (!bProgRight && !defaultShow) {
		ui->programPage->setHidden(!bProgRight);
	}
	if (bProgRight){
		if (!bEraseRight)
			ui->prog_eraseSelectWgt->setHidden(!bEraseRight);
		if (!bBlankRight)
			ui->prog_blankSelectWgt->setHidden(!bBlankRight);
		if (!bVerifyRight)
			ui->prog_verifySelectWgt->setHidden(!bVerifyRight);
		if (!bMarginVerifyRight)
			ui->prog_marginSelectWgt->setHidden(!bMarginVerifyRight);
		if (!bChecksumCompareRight)
			ui->prog_compareSelectWgt->setHidden(!bChecksumCompareRight);
		if (!bSecureRight)
			ui->prog_secureSelectWgt->setHidden(!bSecureRight);

		if (!defaultShow) {
			defaultShow = true;
			defaultTag = OperationTagType::Program;
		}
	}

	//verifyPage
	ui->verifyButton->setHidden(!bVerifyRight);
	if (!bVerifyRight && !defaultShow) {
		ui->verifyPage->setHidden(!bVerifyRight);
	}
	if (bVerifyRight) {
		if (!bMarginVerifyRight)
			ui->verify_marginVerifySelectWgt->setHidden(!bMarginVerifyRight);

		if (!defaultShow) {
			defaultShow = true;
			defaultTag = OperationTagType::Verify;
		}
	}

	//securePage
	ui->secureButton->setHidden(!bSecureRight);
	if (!bSecureRight && !defaultShow) {
		ui->securePage->setHidden(!bSecureRight);
	}
	if (bSecureRight) {
		if (!bVerifyRight)
			ui->secure_verifySelectWgt->setHidden(!bVerifyRight);
		if (!bMarginVerifyRight)
			ui->secure_marginVerifySelectWgt->setHidden(!bMarginVerifyRight);

		if (!defaultShow) {
			defaultShow = true;
			defaultTag = OperationTagType::Secure;
		}
	}

	//readPage
	ui->readButton->setHidden(!bReadRight);
	if (!bReadRight && !defaultShow) {
		ui->readPage->setHidden(!bReadRight);
	}

	if (bReadRight){
		if (!defaultShow) {
			defaultShow = true;
			defaultTag = OperationTagType::Read;
		}
	}

	if(defaultTag != OperationTagType::None)
		m_pageGroup->button((int)defaultTag)->click();
	else
		m_pageGroup->button((int)OperationTagType::Self)->click();
}

AngKOperationTag* AngKProjectOperation::GetOperationTagByType(OperationTagType operTagType)
{
	if (operTagType == OperationTagType::Erase) {
		return ui->eraseTag;
	}
	else if (operTagType == OperationTagType::Blank) {
		return ui->blankTag;
	}
	else if (operTagType == OperationTagType::Program) {
		return ui->programTag;
	}
	else if (operTagType == OperationTagType::Verify) {
		return ui->verifyTag;
	}
	else if (operTagType == OperationTagType::Secure) {
		return ui->secureTag;
	}
	else if (operTagType == OperationTagType::Read) {
		return ui->readTag;
	}

	return nullptr;
}

void AngKProjectOperation::onSlotDropSymbolType(QObject* obj, int symbolType)
{	
	AngKOperationTag* tagOper = qobject_cast<AngKOperationTag*>(obj);

	disconnect(m_widget2Box[tagOper], QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AngKProjectOperation::onSlotChangeWidget);
	m_widget2Box[tagOper]->clear();
	m_widget2Box[tagOper]->addItem(tr("Disabled"), (int)ChipOperCfgSubCmdID::UnEnable);
	switch ((OperationTagType)symbolType)
	{
	case OperationTagType::None:
		ui->plainTextEdit->clear();
		ui->plainTextEdit->appendPlainText(tr("Operating Flow:"));
		m_widget2Box[tagOper]->clear();
		break;
	case OperationTagType::Erase:
	{
		m_widget2Box[tagOper]->addItem(tr("Erase"), (int)ChipOperCfgSubCmdID::SubErase);
		m_widget2Box[tagOper]->addItem(tr("Erase if CheckBlank Failed"), (int)ChipOperCfgSubCmdID::EraseIfBlankCheckFailed);
		m_widget2Box[tagOper]->setCurrentIndex(2);// 默认Erase if CheckBlank Failed
		tagOper->setFlowLabelText(tr("Erase if CheckBlank Failed"));
	}
	break;
	case OperationTagType::Blank:
	{
		m_widget2Box[tagOper]->addItem(tr("Blank Check"), (int)ChipOperCfgSubCmdID::SubBlankCheck);
		m_widget2Box[tagOper]->setCurrentIndex(1);// 默认Blank Check
		tagOper->setFlowLabelText(tr("Blank_Check"));
	}
	break;
	case OperationTagType::Program:
	{
		m_widget2Box[tagOper]->addItem(tr("Enabled"), (int)ChipOperCfgSubCmdID::SubProgram);
		m_widget2Box[tagOper]->setCurrentIndex(1);// 默认Program
		tagOper->setFlowLabelText(tr("Program"));
	}
		break;
	case OperationTagType::Verify:
	{
		m_widget2Box[tagOper]->addItem(tr("Enabled"), (int)ChipOperCfgSubCmdID::SubVerify);
		m_widget2Box[tagOper]->addItem(tr("Hi-Vcc Verify"), (int)ChipOperCfgSubCmdID::HighVerify);
		m_widget2Box[tagOper]->addItem(tr("Lo-Vcc Verify"), (int)ChipOperCfgSubCmdID::LowVerify);
		m_widget2Box[tagOper]->addItem(tr("Hi/Lo-Vcc Verify"), (int)ChipOperCfgSubCmdID::High_Low_Verify);
		m_widget2Box[tagOper]->setCurrentIndex(1);// 默认Verify
		tagOper->setFlowLabelText(tr("Verify"));
	}
		break;
	case OperationTagType::Secure:
	{
		m_widget2Box[tagOper]->addItem(tr("Enabled"), (int)ChipOperCfgSubCmdID::SubSecure);
		m_widget2Box[tagOper]->setCurrentIndex(1);// 默认Secure
		tagOper->setFlowLabelText(tr("Secure"));
	}
		break;
	case OperationTagType::Read:
	{
		m_widget2Box[tagOper]->addItem(tr("Enabled"), (int)ChipOperCfgSubCmdID::SubRead);
		m_widget2Box[tagOper]->setCurrentIndex(1);// 默认Read
		tagOper->setFlowLabelText(tr("Read"));
	}
		break;
	case OperationTagType::CheckSum:
		m_widget2Box[tagOper]->addItem(tr("Enabled"), (int)ChipOperCfgSubCmdID::ChecksumCompare);
		m_widget2Box[tagOper]->setCurrentIndex(1);// 默认ChecksumCompare
		tagOper->setFlowLabelText(tr("Checksum Compare"));
		break;
	default:
		break;
	}

	connect(m_widget2Box[tagOper], QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AngKProjectOperation::onSlotChangeWidget);
	GetSelfPageFlow();
}

void AngKProjectOperation::onSlotChangeWidget(int idx)
{
	qDebug() << idx;
	QComboBox* clickBox = qobject_cast<QComboBox*>(sender());
	m_box2Widget[clickBox]->setBgWidgetProperty("fix");

	switch ((ChipOperCfgSubCmdID)clickBox->currentData().toInt())
	{
	case UnEnable:
		m_box2Widget[clickBox]->setBgWidgetProperty("drag", true);
		m_box2Widget[clickBox]->setFlowLabelText("");
		break;
	case CheckID:
		break;
	case PinCheck:
		break;
	case InsertionCheck:
		break;
	case DevicePowerOn:
		break;
	case DevicePowerOff:
		break;
	case PowerOn:
		break;
	case PowerOff:
		break;
	case SubProgram:
		break;
	case SubErase:
		m_box2Widget[clickBox]->setFlowLabelText(tr("Erase"));
		break;
	case SubVerify:
		break;
	case SubBlankCheck:
		m_box2Widget[clickBox]->setFlowLabelText(tr("Blank Check"));
		break;
	case SubSecure:
		m_box2Widget[clickBox]->setFlowLabelText(tr("Secure"));
		break;
	case SubIllegalCheck:
		break;
	case SubRead:
		break;
	case EraseIfBlankCheckFailed:
		m_box2Widget[clickBox]->setFlowLabelText(tr("Erase if CheckBlank Failed"));
		break;
	case LowVerify:
		m_box2Widget[clickBox]->setFlowLabelText(tr("Lo-Vcc Verify"));
		break;
	case HighVerify:
		m_box2Widget[clickBox]->setFlowLabelText(tr("Hi-Vcc Verify"));
		break;
	case ChecksumCompare:
		m_box2Widget[clickBox]->setFlowLabelText(tr("Checksum Compare"));
		break;
	case High_Low_Verify:
		m_box2Widget[clickBox]->setFlowLabelText(tr("Hi/Lo-Vcc Verify"));
		break;
	default:
		break;
	}

	ui->plainTextEdit->clear();
	ui->plainTextEdit->appendPlainText(tr("Operating Flow:"));

	GetPageFlow(ui->stackedWidget->currentIndex());
}