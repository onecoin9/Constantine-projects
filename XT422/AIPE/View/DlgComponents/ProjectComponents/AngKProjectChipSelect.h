#pragma once

#include <QWidget>
#include "DataJsonSerial.hpp"
namespace Ui { class AngKProjectChipSelect; };

class AngKChipSelectWidget;
class AngKProjectChipSelect : public QWidget
{
	Q_OBJECT

public:
	AngKProjectChipSelect(QWidget *parent = Q_NULLPTR);
	~AngKProjectChipSelect();

	void InitText();

	ChipDataJsonSerial& getJsonSerial() { return m_chipJson; }

	void insertChipText(ChipDataJsonSerial serJson);

signals:
	void sgnChipDataReport(ChipDataJsonSerial);
public slots:
	void onSlotSelectChipDataJson(ChipDataJsonSerial chipJson);

	void onSlotSelectChip();
private:
	Ui::AngKProjectChipSelect *ui;
	ChipDataJsonSerial	m_chipJson;
};
