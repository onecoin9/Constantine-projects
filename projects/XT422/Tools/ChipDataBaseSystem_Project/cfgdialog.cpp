#include "cfgdialog.h"
#include "ui_cfgdialog.h"
#include "../nlohmann/json.hpp"
#include <iostream>
#include <QDebug>
cfgDialog::cfgDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::cfgDialog)
{
    ui->setupUi(this);
    InitText();
    InitButton();
}

cfgDialog::~cfgDialog()
{
    delete ui;
}

void cfgDialog::InitText()
{
    ui->readCheck->setText("Read");
    ui->progCheck->setText("Program");
    ui->blockProgCheck->setText("Block Program");
    ui->illegalBitCheck->setText("Illegal Bit Check");
    ui->functionCheck->setText("Functional Test");
    ui->verifyCheck->setText("Verify");
    ui->eraseCheck->setText("Erase");
    ui->blankCheck->setText("Blank Check");
    ui->secureCheck->setText("Secure");

    ui->insectionCheck->setText("Insection Check");
    ui->pinCheck->setText("Pin Check");
    ui->addressRelocateCheck->setText("Address Relocate");
    ui->IDCheck->setText("ID Check");
    ui->EEPROMCheck->setText("EEPROM Auto ID");
    ui->emptyBufferCheck->setText(QStringLiteral("将Buffer空白值设置为0x00"));
    ui->protectCheck->setText("Protect/Unprotect");
    ui->compareCheck->setText("Compare");
    ui->masterCopyCheck->setText("Master Copy");
    ui->vccVerifyCheck->setText("H/L-Vcc Verify");
    ui->loopFunCheck->setText("Loop Function");
    ui->onlineCheck->setText(QStringLiteral("是否支持在线"));
    ui->unTestCheck->setText(QStringLiteral("是否未测试"));
    ui->enableSNCheck->setText("Enable SN");

    ui->sumCheck->setText("Sum");
    ui->wordSumCheck->setText("Word Sum");
    ui->crc16Check->setText("CRC-16 Sum");
    ui->crc32Check->setText("CRC-32 Sum");

    ui->bit4Button->setText("4 Bits");
    ui->bit8Button->setText("8 Bits");
    ui->bit12Button->setText("12 Bits");
    ui->bit16Button->setText("16 Bits");

    ui->wordAddressCheck->setText("Word Address");
    ui->bigEndianCheck->setText("Bigendian");

    ui->okButton->setText(tr("Ok"));
    ui->cancelButton->setText(tr("Cancel"));

    ui->baseOperGroup->setTitle(QStringLiteral("基本操作"));
    ui->otherOperGroup->setTitle(QStringLiteral("其他操作"));
    ui->checkSumGroup->setTitle(QStringLiteral("校验和"));
    ui->bitsGroup->setTitle(QStringLiteral("字宽"));
    ui->fileLoadGroup->setTitle(QStringLiteral("文件加载属性"));
}

void cfgDialog::InitButton()
{
    connect(ui->okButton, &QPushButton::clicked, this, [=](){
        nlohmann::json jsonData;

        nlohmann::json baseJson;
        nlohmann::json otherJson;
        nlohmann::json checksumJson;
        nlohmann::json bitsJson;
        nlohmann::json fileLoadJson;

        //基础操作
        baseJson["read"] = ui->readCheck->isChecked();
        baseJson["prog"] = ui->progCheck->isChecked();
        baseJson["blockProg"] = ui->blockProgCheck->isChecked();
        baseJson["illegalBit"] = ui->illegalBitCheck->isChecked();
        baseJson["function"] = ui->functionCheck->isChecked();
        baseJson["verify"] = ui->verifyCheck->isChecked();
        baseJson["erase"] = ui->eraseCheck->isChecked();
        baseJson["blank"] = ui->blankCheck->isChecked();
        baseJson["secure"] = ui->secureCheck->isChecked();

        //其他操作
        otherJson["insection"] = ui->insectionCheck->isChecked();
        otherJson["pin"] = ui->pinCheck->isChecked();
        otherJson["addressRelocate"] = ui->addressRelocateCheck->isChecked();
        otherJson["IDCheck"] = ui->IDCheck->isChecked();
        otherJson["EEPROM"] = ui->EEPROMCheck->isChecked();
        otherJson["emptyBuffer"] = ui->emptyBufferCheck->isChecked();
        otherJson["protect"] = ui->protectCheck->isChecked();
        otherJson["compare"] = ui->compareCheck->isChecked();
        otherJson["masterCopy"] = ui->masterCopyCheck->isChecked();
        otherJson["vccVerify"] = ui->vccVerifyCheck->isChecked();
        otherJson["loopFun"] = ui->loopFunCheck->isChecked();
        otherJson["online"] = ui->onlineCheck->isChecked();
        otherJson["unTest"] = ui->unTestCheck->isChecked();
        otherJson["enableSN"] = ui->enableSNCheck->isChecked();

        //校验和
        checksumJson["sum"] = ui->sumCheck->isChecked();
        checksumJson["wordSum"] = ui->wordSumCheck->isChecked();
        checksumJson["crc16"] = ui->crc16Check->isChecked();
        checksumJson["crc32"] = ui->crc32Check->isChecked();

        //字宽
        bitsJson["bit4"] = ui->bit4Button->isChecked();
        bitsJson["bit8"] = ui->bit8Button->isChecked();
        bitsJson["bit12"] = ui->bit12Button->isChecked();
        bitsJson["bit16"] = ui->bit16Button->isChecked();

        //文件加载属性
        fileLoadJson["wordAddress"] = ui->wordAddressCheck->isChecked();
        fileLoadJson["bigEndian"] = ui->bigEndianCheck->isChecked();

        jsonData["baseOper"] = baseJson;
        jsonData["otherOper"] = otherJson;
        jsonData["checkSumOper"] = checksumJson;
        jsonData["bitsOper"] = bitsJson;
        jsonData["fileLoadOper"] = fileLoadJson;

        QString strJson = QString::fromStdString(jsonData.dump());

        emit sgnSaveOperAttri(strJson);

        close();
    });

    connect(ui->cancelButton, &QPushButton::clicked, this, &cfgDialog::close);
}

void cfgDialog::setOperAttr(QString attrJson)
{
    try {//nlohmann解析失败会报异常需要捕获一下

        auto config_json = nlohmann::json::parse(attrJson.toStdString());
        std::cout << config_json << std::endl;

        nlohmann::json baseJson = config_json["baseOper"];
        nlohmann::json otherJson = config_json["otherOper"];
        nlohmann::json checksumJson = config_json["checkSumOper"];
        nlohmann::json bitsJson = config_json["bitsOper"];
        nlohmann::json fileLoadJson = config_json["fileLoadOper"];

        //基础操作
        ui->readCheck->setChecked(baseJson["read"].get<bool>());
        ui->progCheck->setChecked(baseJson["prog"].get<bool>());
        ui->blockProgCheck->setChecked(baseJson["blockProg"].get<bool>());
        ui->illegalBitCheck->setChecked(baseJson["illegalBit"].get<bool>());
        ui->functionCheck->setChecked(baseJson["function"].get<bool>());
        ui->verifyCheck->setChecked(baseJson["verify"].get<bool>());
        ui->eraseCheck->setChecked(baseJson["erase"].get<bool>());
        ui->blankCheck->setChecked(baseJson["blank"].get<bool>());
        ui->secureCheck->setChecked(baseJson["secure"].get<bool>());

        //其他操作
        ui->insectionCheck->setChecked(otherJson["insection"].get<bool>());
        ui->pinCheck->setChecked(otherJson["pin"].get<bool>());
        ui->addressRelocateCheck->setChecked(otherJson["addressRelocate"].get<bool>());
        ui->IDCheck->setChecked(otherJson["IDCheck"].get<bool>());
        ui->EEPROMCheck->setChecked(otherJson["EEPROM"].get<bool>());
        ui->emptyBufferCheck->setChecked(otherJson["emptyBuffer"].get<bool>());
        ui->protectCheck->setChecked(otherJson["protect"].get<bool>());
        ui->compareCheck->setChecked(otherJson["compare"].get<bool>());
        ui->masterCopyCheck->setChecked(otherJson["masterCopy"].get<bool>());
        ui->vccVerifyCheck->setChecked(otherJson["vccVerify"].get<bool>());
        ui->loopFunCheck->setChecked(otherJson["loopFun"].get<bool>());
        ui->onlineCheck->setChecked(otherJson["online"].get<bool>());
        ui->unTestCheck->setChecked(otherJson["unTest"].get<bool>());
        ui->enableSNCheck->setChecked(otherJson["enableSN"].get<bool>());

        //校验和
        ui->sumCheck->setChecked(checksumJson["sum"].get<bool>());
        ui->wordSumCheck->setChecked(checksumJson["wordSum"].get<bool>());
        ui->crc16Check->setChecked(checksumJson["crc16"].get<bool>());
        ui->crc32Check->setChecked(checksumJson["crc32"].get<bool>());

        //字宽
        ui->bit4Button->setChecked(bitsJson["bit4"].get<bool>());
        ui->bit8Button->setChecked(bitsJson["bit8"].get<bool>());
        ui->bit12Button->setChecked(bitsJson["bit12"].get<bool>());
        ui->bit16Button->setChecked(bitsJson["bit16"].get<bool>());

        //文件加载属性
        ui->wordAddressCheck->setChecked(fileLoadJson["wordAddress"].get<bool>());
        ui->bigEndianCheck->setChecked(fileLoadJson["bigEndian"].get<bool>());
    }catch (const nlohmann::json::parse_error& e){
            qDebug() << "TAG_CHIPDATA Json parse failed or Json is Null";
    }
}
