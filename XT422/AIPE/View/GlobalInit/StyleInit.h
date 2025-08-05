#pragma once

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include "AngKPathResolve.h"
#include "AngkLogger.h"

#ifndef QT_SET_STYLE_SHEET
#define QT_SET_STYLE_SHEET(qssFile)	{ QString _path = Utils::AngKPathResolve::localSkinFile(qssFile, true); QFile f(_path); f.open(QFile::ReadOnly); if (f.isOpen()){ setStyleSheet(QString(f.readAll())); f.close();} }
#endif QT_SET_STYLE_SHEET

#define ANGKACTION_MENU_PROJ	QString(QObject::tr("Project"))
#define ANGKACTION_MENU_FILE	QString(QObject::tr("Task"))
#define ANGKACTION_MENU_UTIL	QString(QObject::tr("Utilities"))
#define ANGKACTION_MENU_WINDOW	QString(QObject::tr("Window"))
#define ANGKACTION_MENU_VIEW	QString(QObject::tr("View"))
#define ANGKACTION_MENU_SETTING	QString(QObject::tr("Setting"))
#define ANGKACTION_MENU_AUTOMATIC	QString(QObject::tr("Automatic"))
#define ANGKACTION_MENU_HELP	QString(QObject::tr("Help"))
#define ANGKACTION_MENU_DEBUG	QString(QObject::tr("Debug"))
