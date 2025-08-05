#ifndef ANGK_WINDOW_TITLE_BAR_
#define ANGK_WINDOW_TITLE_BAR_
#pragma once

#include <QWidget>

class QPushButton;
class QLabel;
class QComboBox;
class AngKTitleImg;

class AngKWindowTitleBar : public QWidget
{
	Q_OBJECT

public:
	AngKWindowTitleBar(QWidget* parent, QWidget* window, QWidget* shadowContainerWidget, bool canResize);
	~AngKWindowTitleBar();

private:
	QWidget* m_window;
	QWidget* m_shadowContainerWidget;
	QMargins m_oldContentsMargin;

signals:
	void sgnIntroPropetry(QString);

public slots:
	void showSmall();
	void showMaxRestore();
	
public:
	void InitButton();
	void InitComboBox();
	void PersonImgChange(QString sPath);
	void PersonNameChange(QString sName);

	QPushButton* maxButton() const { return m_maximize; }
	QPushButton* minButton() const { return m_minimize; }
	QPushButton* closeButton() const { return m_close; }
	QLabel* titleLabel() const { return m_titleLabel; }
	
	void setContentsMargins(QMargins margins);
	
	void setText(QString title);
	QString text() const;

	void setShowTitleComponent(bool bShow);
protected:
	virtual void mousePressEvent(QMouseEvent* me);
	virtual void mouseMoveEvent(QMouseEvent* me);
	virtual void mouseDoubleClickEvent(QMouseEvent* event);
public:
	virtual void layoutTitleBar();
	virtual void windowStateChanged();
private:
	QPushButton* m_minimize;
	QPushButton* m_maximize;
	QPushButton* m_close;
	QLabel* m_titleLabel;
	QLabel* m_titleTextLabel;
	QLabel* m_versionTextLabel;
	QComboBox* m_comboBox;
	AngKTitleImg* m_personImg;
	QString m_restoreStyleSheet, m_maxSheet;
	QPoint m_startPos;
	QPoint m_clickPos;
	bool m_canResize;
};


#endif // !ANGK_WINDOW_TITLE_BAR_