QT       += core gui
QT       += serialport
QT       += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    src/appconfig.cpp \
    src/customitem.cpp \
    src/main.cpp \
    src/mainwindow.cpp \
    src/networkmanager.cpp \
    src/sendfiledialog.cpp \
    src/serialmanager.cpp \
    src/serialsettingdialog.cpp \
    src/tabpage.cpp

HEADERS += \
    src/appconfig.h \
    src/customitem.h \
    src/dataStructure.h \
    src/mainwindow.h \
    src/networkmanager.h \
    src/sendfiledialog.h \
    src/serialmanager.h \
    src/serialsettingdialog.h \
    src/tabpage.h

FORMS += \
    src/customitem.ui \
    src/mainwindow.ui \
    src/sendfiledialog.ui \
    src/serialsettingdialog.ui \
    src/tabpage.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resource.qrc

RC_ICONS = PicRes\usb_blue.ico

VERSION = 1.2.1

DEFINES += APP_VERSION=\\\"$$VERSION\\\"

# DEFINES += QT_NO_DEBUG_OUTPUT
