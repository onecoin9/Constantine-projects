#pragma once

#include <QtWidgets/QWidget>
#include "ChipOperatorCfgDefine.h"
#include "GlobalDefine.h"
namespace Ui { class AngKToolWidget; };

static int SP_BufferToolButton = 10;
static int SP_ChipSelectToolButton = 11;

class QButtonGroup;
class QToolButton;
class QMouseEvent;
class AngKToolWidget : public QWidget
{
	Q_OBJECT

public:
	AngKToolWidget(QWidget *parent = Q_NULLPTR);
	~AngKToolWidget();

	/// <summary>
	/// 初始化命令按钮
	/// </summary>
	void InitButton();

	/// <summary>
	/// 更新芯片命令操作权限
	/// </summary>
	/// <param name="baseInfo">操作Json保存</param>
	void ShowButtonFromJson(BaseOper baseInfo);

	int GetSelectOperType();

	void SetButtonEnable(OperationTagType nType);
protected:
	virtual bool eventFilter(QObject* watched, QEvent* event);

	void ButtonHoverEnterState(QToolButton* btn);
	void ButtonRecoverState(QToolButton* btn);
	void ButtonPressState(QToolButton* btn);

signals:
	void sgnOpenBufferTool();
	void sgnOpenSelectChipDlg();

public slots:
	/// <summary>
	/// 按钮执行操作槽函数
	/// </summary>
	void onSlotClick_defineToolButton(bool);
	void onSlotClick_eraseToolButton(bool);
	void onSlotClick_blankToolButton(bool);
	void onSlotClick_programToolButton(bool);
	void onSlotClick_verifyToolButton(bool);
	void onSlotClick_readToolButton(bool);
	void onSlotClick_secureToolButton(bool);
	void onSlotClick_illegalToolButton(bool);
	void onSlotClick_protectToolButton(bool);
	void onSlotClick_bufferToolButton(bool);

private:
	Ui::AngKToolWidget *ui;
	bool			m_bDark;
	QButtonGroup*	m_buttonGroup;
	int				m_nLastId;
};
