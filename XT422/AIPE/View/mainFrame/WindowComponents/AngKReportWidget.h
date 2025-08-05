#pragma once

#include <QtWidgets/QWidget>
namespace Ui { class AngKReportWidget; };

class AngKReportWidget : public QWidget
{
	Q_OBJECT

public:
	AngKReportWidget(QWidget *parent = Q_NULLPTR);
	~AngKReportWidget();

	/// <summary>
	/// 设置烧录进度
	/// </summary>
	/// <param name="nProgress">烧录进度</param>
	void SetCurrentProgress(int nProgress);

	void UpdateCurrentProgress();

	void SetoutputSlot(int nOutput);

	void SetExpectSlot(int nExpect);

	void ResetExpectAndOutput();

	void AddOutputNum();

	void AddFailedNum();

	void CalCurrentYield();
private:
	Ui::AngKReportWidget *ui;
	int m_nFailedNum;	//失败数需要使用烧录数-成功数获取。因为AP9900开始阶段，失败结果存在漏返回，上位机自动记录
};
