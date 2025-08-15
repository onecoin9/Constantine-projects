#include "stdafx.h"
#include "SNStepCfg.h"

CSNStepCfg::CSNStepCfg() : addr_(0), bytes_(0) {

}

CSNStepCfg::~CSNStepCfg() {

}



BOOL CSNStepCfg::SerialInCfgData( CSerial& lSerial ) {
	try {
		lSerial >> addr_ >> bytes_;
	}
	catch (CACException){
		return FALSE;
	}
	return TRUE;
}



BOOL CSNStepCfg::SerialOutCfgData( CSerial& lSerial )
{
	try {
		lSerial << addr_ << bytes_;
	}
	catch (CACException){
		return FALSE;
	}
	return TRUE;
}

