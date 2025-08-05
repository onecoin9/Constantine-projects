#pragma once

#include <QWidget>
namespace Ui { class AngKProjectDrvParaSetting; };

class QStandardItemModel;
class AngKProjectDrvParaSetting : public QWidget
{
	Q_OBJECT

public:
	AngKProjectDrvParaSetting(QWidget *parent = Q_NULLPTR);
	~AngKProjectDrvParaSetting();
	
	void InitTable();

	void InsertCommonDrvPara(QString commonJson);

	void InsertSelfDrvPara(QString selfJson);

	void GetDrvParaJson(QString& commonJson, QString& selfJson);
private:
	Ui::AngKProjectDrvParaSetting *ui;
	QStandardItemModel* m_DriverCommonParaTableModel;
	QStandardItemModel* m_DriverSelfParaTableModel;
};
