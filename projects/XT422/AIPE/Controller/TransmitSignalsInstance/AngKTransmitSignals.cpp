#include "AngKTransmitSignals.h"


AngKTransmitSignals& AngKTransmitSignals::GetInstance()
{
	static AngKTransmitSignals m_pControl;
	return m_pControl;
}


AngKTransmitSignals::AngKTransmitSignals()
{
}

AngKTransmitSignals::~AngKTransmitSignals()
{
}
