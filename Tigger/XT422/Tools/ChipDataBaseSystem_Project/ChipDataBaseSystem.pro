QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

DEFINES -= UNICODE
DEFINES += UMBCS
QMAKE_CXXFLAGS -= -Zc:strictStrings

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    DataBaseOper/AngKDataBaseOper.cpp \
    DataBaseOper/ISQList.cpp \
    Model/AngKDBStandardItemModelEx.cpp \
    Model/AngKItemDelegate.cpp \
    Model/AngKSortFilterProxyModel.cpp \
    adddialog.cpp \
    cfgdialog.cpp \
    databaseoper.cpp \
    framelesswindow.cpp \
    main.cpp \
    mainwindow.cpp \
    titlebar.cpp

HEADERS += \
    DataBaseOper/AngKDataBaseOper.h \
    DataBaseOper/ChipDBModel.h \
    DataBaseOper/ChipModel.h \
    DataBaseOper/DBController.hpp \
    DataBaseOper/ISQList.h \
    DataBaseOper/SQLBuilder.hpp \
    DataBaseOper/SQLStatement.hpp \
    DataBaseOper/reflection.hpp \
    GlobalDefine.h \
    Model/AngKDBStandardItemModelEx.h \
    Model/AngKItemDelegate.h \
    Model/AngKSortFilterProxyModel.h \
    Sqlite3/sqlite3.h \
    StyleInit.h \
    adddialog.h \
    cfgdialog.h \
    databaseoper.h \
    framelesswindow.h \
    mainwindow.h \
    nlohmann/json.hpp \
    nlohmann/json_fwd.hpp \
    titlebar.h

FORMS += \
    adddialog.ui \
    cfgdialog.ui \
    databaseoper.ui \
    mainwindow.ui \
    titlebar.ui

TRANSLATIONS += \
    ChipDataBaseSystem_zh_CN.ts
CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/Sqlite3/ -lsqlite3
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/Sqlite3/ -lsqlite3
else:unix:!macx: LIBS += -L$$PWD/Sqlite3/ -lsqlite3

INCLUDEPATH += $$PWD/Sqlite3
DEPENDPATH += $$PWD/Sqlite3

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/Sqlite3/libsqlite3.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/Sqlite3/libsqlite3.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/Sqlite3/sqlite3.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/Sqlite3/sqlite3.lib
else:unix:!macx: PRE_TARGETDEPS += $$PWD/Sqlite3/libsqlite3.a
