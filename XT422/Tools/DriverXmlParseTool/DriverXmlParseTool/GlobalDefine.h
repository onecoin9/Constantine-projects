#pragma once
#include "AGroupBox.h"

#define FRAMESHAPE 10

static QString strGroup = "Group";
static QString strLabel = "Label";
static QString strTextEdit = "TextEdit";
static QString strCheckBox = "CheckBox";
static QString strComboBox = "ComboBox";
static QString strTabWidget = "TabPage";

enum EditValue
{
	DEC = 0,
	HEX = 1,
	STR = 2
};

enum EditEndian
{
	Little = 0,
	Big = 1
};

enum SpacePos
{
	TOPLEFT = 11,
	TOP = 12,
	TOPRIGHT = 13,
	LEFT = 21,
	CENTER = 22,
	RIGHT = 23,
	BUTTOMLEFT = 31,
	BUTTOM = 32,
	BUTTOMRIGHT = 33
};