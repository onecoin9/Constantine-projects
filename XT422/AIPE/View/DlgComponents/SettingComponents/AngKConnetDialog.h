#pragma once

#include <QtWidgets/QDialog>
#include "ui_AngKConnetDialog.h"
#include "AppModeType.h"



class AngKDeviceModel;
class AngKConnetDialog : public QDialog
{
	Q_OBJECT

public:
	AngKConnetDialog(QWidget *parent = Q_NULLPTR);
	~AngKConnetDialog();
protected:
	virtual bool eventFilter(QObject* watched, QEvent* event);
	virtual void resizeEvent(QResizeEvent* re);
	virtual void closeEvent(QCloseEvent* event) override; // 重写关闭事件
	virtual void mousePressEvent(QMouseEvent* event);
	virtual void mouseMoveEvent(QMouseEvent* event);
	virtual void mouseReleaseEvent(QMouseEvent* event);

	/// <summary>
	/// 初始化窗口类
	/// </summary>
	void InitConnectDlg();

	/// <summary>
	/// 初始化弹框按钮
	/// </summary>
	void InitButton();

	/// <summary>
	/// 切换连接方式类型
	/// </summary>
	/// <param name="nIndexType">连接模式类型</param>
	void SwitchConnectTypeWgt(ConnectType nIndexType);
signals:
	void sgnCloseWindow();

public slots:

	void onSlotConnectFinish();
	void onSlotEnterDemoMode();
	void onSlotScanManagerFinished();
private:
	Ui::AngKConnetDialog *ui;
	ConnectType		m_curConnectType;
	int				m_nOldHeight;
	QSize			m_nOldSize;
	QSize			m_InitSize;
	QDesktopWidget* m_desktop;
	QPoint			m_clickPos;
	int				m_nDraggableHeight;   // 可拖动区域的高度
	bool			m_bDragging; // 拖动状态标志
	QButtonGroup*	m_pConTypeGroup;
};
