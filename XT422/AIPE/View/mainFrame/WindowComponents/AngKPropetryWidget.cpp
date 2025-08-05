#include "AngKPropetryWidget.h"
#include "ui_AngKPropetryWidget.h"
#include "../View/GlobalInit/StyleInit.h"
#include "GlobalDefine.h"
#include <QTabBar>
AngKPropetryWidget::AngKPropetryWidget(QWidget *parent)
	: QWidget(parent)
{
	ui = new Ui::AngKPropetryWidget();
	ui->setupUi(this);

	ui->tabWidget->setTabText(0, tr("Task"));
	ui->tabWidget->setTabText(1, tr("Programmer"));
	ui->tabWidget->setTabText(2, tr("Site"));

	this->setObjectName("AngKPropetryWidget");
	QT_SET_STYLE_SHEET(objectName());
}

AngKPropetryWidget::~AngKPropetryWidget()
{
	delete ui;
}

void AngKPropetryWidget::showArea(int nIndex, bool bShow)
{
	this->setUpdatesEnabled(false);
	switch ((WinActionType)nIndex)
	{
	case WinActionType::Project:
	{
		if (bShow)
		{
			ui->tabProject->show();
			ui->tabWidget->tabBar()->setTabVisible(nIndex - 2, bShow);
			ui->tabWidget->setCurrentIndex(nIndex - 2);
		}
		else
		{
			ui->tabProject->hide();
			ui->tabWidget->tabBar()->setTabVisible(nIndex - 2, bShow);
		}
	}
		break;
	case WinActionType::Programmer:
	{
		if (bShow)
		{
			ui->tabProgram->show();
			ui->tabWidget->tabBar()->setTabVisible(nIndex - 2, bShow);
			ui->tabWidget->setCurrentIndex(nIndex - 2);
		}
		else
		{
			ui->tabProgram->hide();
			ui->tabWidget->tabBar()->setTabVisible(nIndex - 2, bShow);
		}
	}
		break;
	case WinActionType::Site:
	{
		if (bShow)
		{
			ui->tabSite->show();
			ui->tabWidget->tabBar()->setTabVisible(nIndex - 2, bShow);
			ui->tabWidget->setCurrentIndex(nIndex - 2);
		}
		else
		{
			ui->tabSite->hide();
			ui->tabWidget->tabBar()->setTabVisible(nIndex - 2, bShow);
		}
	}
		break;
	default:
		break;
	}

	if (ui->tabProgram->isHidden() && ui->tabProject->isHidden() && ui->tabSite->isHidden())
	{
		ui->bgWidget->hide();
		this->hide();
	}
	else
	{
		this->show();
		ui->bgWidget->show();
	}
	this->setUpdatesEnabled(true);
	
}

void AngKPropetryWidget::SetProjManagerInfo(QMap<QString, QPair<QString, ACProjManager*>> _mapProj)
{
	ui->tabProject->SetProjPariInfo(_mapProj);
}

void AngKPropetryWidget::UpdatePropertyUI()
{
	ui->tabProject->UpdateProjUI();
	//ui->tabProgram->UpdateProgUI();
	//ui->tabSite->UpdateSiteUI();
}
