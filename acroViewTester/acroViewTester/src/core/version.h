// version.h
#ifndef VERSION_H
#define VERSION_H

#define APP_VERSION_MAJOR 0
#define APP_VERSION_MINOR 1
#define APP_VERSION_PATCH 0

#define APP_VERSION_STRING "v" \
    QString::number(APP_VERSION_MAJOR) + "." + \
    QString::number(APP_VERSION_MINOR) + "." + \
    QString::number(APP_VERSION_PATCH)

#endif // VERSION_H#pragma once
