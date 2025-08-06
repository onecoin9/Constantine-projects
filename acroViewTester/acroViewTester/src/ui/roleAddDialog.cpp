#include "roleAddDialog.h"


RoleAddDialog::RoleAddDialog(QWidget* parent) :
    QDialog(parent),
    ui(new Ui::RoleAddDialog)
{
    ui->setupUi(this);
   // this->setWindowFlags(Qt::WindowCloseButtonHint); 
    qDebug() << "RoleAddDialog created";
}

RoleAddDialog::~RoleAddDialog()
{
    delete ui;
}