#pragma once

#include <QtWidgets/QAction>
#include <QtCore/QDebug>
#include <QtGui/QDragEnterEvent>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QInputDialog>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMenu>
#include <QtCore/QMimeData>
#include <QtCore/QRect>
#include <QtWidgets/QWidget>
#include <vector>
#include "GlobalDefine.h"
#include "SetWinSize.h"

class AGroupBox;
class ALabel;
class AEditText;
class ACheckBox;
class AComboBox;
class ATabWidget;

class EditShowWidget : public QWidget
{
	Q_OBJECT
public:
	EditShowWidget(QWidget *parent = nullptr);
	~EditShowWidget();

	int CheckCurWgtIndex(QString strCtrl);

	void ClearAllVec();

	//拖动重新创建
	void DropCreateGroupBox(QPoint drop);
	void DropCreateLabel(QPoint drop);
	void DropCreateEditText(QPoint drop);
	void DropCreateCheckBox(QPoint drop);
	void DropCreateComboBox(QPoint drop);
	void DropCreateTabWidget(QPoint drop);

	void CheckCover(QPoint ePos);

	void SetCtrlNewGroupParent(int idx, int posX, int posY);
	void SetCtrlNewTabWidgetParent(int idx, int posX, int posY);

	void ClearChildList(QObjectList _objList);

	template<class T>
	void SetConnectSize(T* t, QAction* adjAct);

	template<class T>
	std::vector<T*>& GetSubWinVec(T* tt);

	//读取XML建立槽函数链接
	int XmlCreateGroupBox(AGroupBox* agb);
	int XmlCreateLabel(ALabel* aLabel);
	int XmlCreateEditText(AEditText* aEditText);
	int XmlCreateCheckBox(ACheckBox* ackBox);
	int XmlCreateComboBox(AComboBox* acbBox);
	int XmlCreateTabWidget(ATabWidget* tabWgt);

	ATabWidget* IsSubTabWgt(QString strName);

	int EditWgtChildren();

protected:
	virtual void dragEnterEvent(QDragEnterEvent* event);

	virtual void dropEvent(QDropEvent* event);

	virtual void mouseMoveEvent(QMouseEvent* event);

	virtual void mouseReleaseEvent(QMouseEvent* event);

private:
	void GroupRClick(AGroupBox* agb, QMenu* qmenu, QAction* adj, QAction* del);
	void LabelRClick(ALabel* alabel, QMenu* qmenu, QAction* adj, QAction* del);
	void EditTextRClick(AEditText* aEdit, QMenu* qmenu, QAction* adj, QAction* del);
	void CheckBoxRClick(ACheckBox* aChkBox, QMenu* qmenu, QAction* adj, QAction* del);
	void ComboBoxRClick(AComboBox* aCbBox, QMenu* qmenu, QAction* adj, QAction* del, QAction* addEle, QAction* delEle);
	void TabWidgetRClick(ATabWidget* agb, QMenu* qmenu, QAction* adj, QAction* del, QAction* InsertAf, QAction* InsertBe, QAction* modifyAct);

signals:
	void sgnShowPropetry(QObject*);
	void sgnClearPropetry(QObject*);
public slots:
	void OnSlotGroupBox(QObject*);
	void OnSlotLabel(QObject*);
	void OnSlotEditText(QObject*);
	void OnSlotCheckBox(QObject*);
	void OnSlotComboBox(QObject*);
	void OnSlotTabWidget(QObject*);

	void OnSlotRClick(QObject*);
private:
	QPoint							m_startPos;//鼠标位置
	QPoint							m_curPos;
	QObject*						m_curSelect;
	SetWinSize*						m_winSizeDlg;
	std::vector<AGroupBox*>			m_vecGroup;
	std::vector<ALabel*>			m_vecLabel;
	std::vector<AEditText*>			m_vecEditText;
	std::vector<ACheckBox*>			m_vecCheckBox;
	std::vector<AComboBox*>			m_vecComboBox;
	std::vector<ATabWidget*>		m_vecTabWidget;
};