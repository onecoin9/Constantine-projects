#pragma once

#include <QMetaType>
#include <stdint.h>
#include "Platform.h"


//定义属性
#define DefineProperty(_Type,_PropName) \
public:\
	Q_PROPERTY(_Type _PropName READ _PropName WRITE set##_PropName);\
	_Type _PropName(){return m_##_PropName;}\
	void set##_PropName(_Type _PropName){m_##_PropName = _PropName;}\
private:\
	_Type m_##_PropName;

//定义属性，并在属性变化的时候发送对应的信号，注意信号不能在宏里面实现，需要单独实现
#define DefineProperty_Notify(_Type,_PropName) \
public:\
	Q_PROPERTY(_Type _PropName READ _PropName WRITE set##_PropName NOTIFY _PropName##Changed);\
	_Type _PropName(){return m_##_PropName;}\
	void set##_PropName(_Type _PropName){\
		m_##_PropName = _PropName;\
		emit _PropName##Changed(_PropName);\
	}\
private:\
	_Type m_##_PropName;