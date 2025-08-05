#pragma once

#include "AngKDialog.h"
#include <QtWidgets/QWidget>
#include <QFile>
namespace Ui { class AngKBufferHexEdit; };

class AngKHexViewEdit;
class AngKDataBuffer;
class AngKBufferHexEdit : public AngKDialog
{
	Q_OBJECT

public:
	AngKBufferHexEdit(AngKDataBuffer* pDataBuf, QWidget* parent = Q_NULLPTR);
	~AngKBufferHexEdit();

	void InitText();

	void InitButton();

	int LoadXmlFile(QString strFile);

	void InitHexEdit(AngKDataBuffer* pDataBuf);

	AngKHexViewEdit* GetCurrentShowViewEdit();


public slots:

	void onSlotEditEnable();
	void onSlotClearBuffer();
	void onSlotGoto();
	void onSlotFill();
	void onSlotFind();
	void onSlotSwap();

	void onSlotGoToAddress(QString);
	void onSlotFillBuffer(QString, QString, QString, bool, bool);
	void onSlotFindBuffer(QString, QString, QString, bool);
	void onSlotFindNextBuffer(QString, QString, QString, bool);
private:
	Ui::AngKBufferHexEdit *ui;
	QFile	file;
	bool	m_bEdit;
	AngKDataBuffer* m_pDataBuf;
};
