#pragma once

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
//#include "AngKPathResolve.h"
//#include "AngkLogger.h"

#ifndef QT_SET_STYLE_SHEET
#define QT_SET_STYLE_SHEET(qssFile)	{ QString _path = Utils::AngKPathResolve::localSkinFile(qssFile, true); QFile f(_path); ALOG_INFO(_path); f.open(QFile::ReadOnly); if (f.isOpen()){ setStyleSheet(QString(f.readAll())); f.close();} }
#endif QT_SET_STYLE_SHEET

#define ANGKACTION_MENU_PROJ	QString("Project")
#define ANGKACTION_MENU_FILE	QString("File")
#define ANGKACTION_MENU_UTIL	QString("Utilities")
#define ANGKACTION_MENU_WINDOW	QString("Window")
#define ANGKACTION_MENU_VIEW	QString("View")
#define ANGKACTION_MENU_SETTING	QString("Setting")
#define ANGKACTION_MENU_HELP	QString("Help")
