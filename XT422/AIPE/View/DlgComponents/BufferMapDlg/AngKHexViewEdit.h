#pragma once

#include <QtWidgets/QWidget>
#include "DataBufferDefine.h"
namespace Ui { class AngKHexViewEdit; };

class AngKHexViewEdit : public QWidget
{
	Q_OBJECT

public:
	AngKHexViewEdit(QWidget *parent = Q_NULLPTR);
	~AngKHexViewEdit();

	void InitText();

	void OpenFile(QString fileName);

	void OpenIODevice(QIODevice* ioDev);

	void OpenMemoryData(QByteArray byteData);

	void setAddressRange(quint64 beginAddr, quint64 endAddr = 0);

	void setAddressText(QString addr);

	void setBufInfo(tBufInfo* _bufInfo);

	void setEditEnable(bool bEdit);

	tBufInfo* GetBufInfo();

	void ClearEdit();

	void gotoBar(int pos, bool isLine);
private:
	Ui::AngKHexViewEdit *ui;
	tBufInfo* m_pBufInfo;
};
