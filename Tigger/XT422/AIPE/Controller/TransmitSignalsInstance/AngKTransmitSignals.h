#pragma once
#ifndef TRANSMITSIGNALS_H
#define TRANSMITSIGNALS_H

/*当前UI存在跨越多界面进行信号槽连接，代码查看方式不友好，对这样的问题引入中间类做信号中转传递*/

#include <QObject>
#include "DataJsonSerial.hpp"

class AngKTransmitSignals : public QObject
{
	Q_OBJECT

public:
	static AngKTransmitSignals& GetInstance();
private:
	AngKTransmitSignals();
	~AngKTransmitSignals();

	AngKTransmitSignals(const AngKTransmitSignals&) = delete;
	AngKTransmitSignals(const AngKTransmitSignals&&) = delete;
	AngKTransmitSignals& operator=(const AngKTransmitSignals&) = delete;

signals:
	void sigHandleEventTransmitChipIDFetched(std::string resultJson);
	void sigHandleEventTransmitExtCSDFetched(std::string resultJson);
	void sgnSelectChipDataJson(ChipDataJsonSerial);
	void sgnHexEditInput(qint64, uchar);
	void sgnProperty2Mainframe(QString, int);
	void sigLinkStatusChanged(quint32 HopNum, quint16 LinkStatus, quint32 IsLastHop);
	void sigOption2PropertyACXMLChipID(std::string acxmlChipID);
public slots:

};

#endif // TRANSMITSIGNALS_H