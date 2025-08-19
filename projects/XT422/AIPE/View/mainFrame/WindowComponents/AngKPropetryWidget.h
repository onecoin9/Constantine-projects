#pragma once

#include <QtWidgets/QWidget>
#include "AngKProjDataset.h"
#include "ACProjManager.h"
namespace Ui { class AngKPropetryWidget; };

class AngKPropetryWidget : public QWidget
{
	Q_OBJECT

public:
	AngKPropetryWidget(QWidget *parent = Q_NULLPTR);
	~AngKPropetryWidget();

	/// <summary>
	/// 是否展示属性窗口
	/// </summary>
	/// <param name="nIndex">属性窗口索引</param>
	/// <param name="bShow">是否show</param>
	void showArea(int nIndex, bool bShow);

	void SetProjManagerInfo(QMap<QString, QPair<QString, ACProjManager*>> _mapProj);
	/// <summary>
	/// 刷新属性UI
	/// </summary>
	void UpdatePropertyUI();
signals:
	void sgnAddProject();
	void sgnSaveProject();

private:
	Ui::AngKPropetryWidget* ui;
};
