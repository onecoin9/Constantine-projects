#pragma once

#include <QWidget>
#include "GlobalDefine.h"
namespace Ui { class AngKOperationTag; };

class AngKOperationTag : public QWidget
{
	Q_OBJECT

public:
	AngKOperationTag(QWidget *parent = Q_NULLPTR);
	~AngKOperationTag();

	void ClearText();

	void setBgWidgetProperty(QString value, bool bgWidget = false);

	void setDrag(bool bDrag);

	void setRightClick(bool bRight);

	void InitWidget(QString text, QString symbol, OperationTagType operType);

	QString getLabelText();

	QString getFlowLabelText();

	void setFlowLabelText(QString flowText);

	OperationTagType GetOperationTagType();

	void PaintSymbolLabel(QString operStr);

	void setCurOperateType(OperationTagType operType);
protected:
	virtual void mousePressEvent(QMouseEvent* event);

	virtual void mouseMoveEvent(QMouseEvent* event);

	virtual void dragEnterEvent(QDragEnterEvent* event);

	virtual void dropEvent(QDropEvent* event);
private:
	void InitSymbol();

	void setLabelText(QString text);

	void setSymbolLabel(QString symbol);
signals:
	void sgnDropSymbolType(QObject*, int);
private:
	Ui::AngKOperationTag *ui;
	bool						m_bDrag;
	bool						m_bRightClick;
	QPoint						m_startPos;	// 拖放起点
	QString						m_operFlowText;
	OperationTagType			m_operType;
	std::map<QString, QString>	m_mapOperSymbolPath;
};
