######################################################################
# Automatically generated by qmake (2.01a) Fri Aug 10 20:31:37 2012
######################################################################

TEMPLATE = app
TARGET = OpenPuush
DEPENDPATH += .
INCLUDEPATH += .

CONFIG += qxt
QT += core gui network
QXT += core gui

# Input
HEADERS += dropbox.hpp screenshot_overlay.hpp \
    openpuush.hpp \
    configwindow.hpp \
    audio_mgr.hpp \
    config.hpp
SOURCES += dropbox.cpp main.cpp screenshot_overlay.cpp \
    openpuush.cpp \
    configwindow.cpp \
    audio_mgr.cpp \
    config.cpp

FORMS += \
    configwindow.ui

RESOURCES += \
    resources.qrc

BUILDNO = $$system(ruby build_increment.rb)
DEFINES += BUILD_NUMBER=$${BUILDNO}

mac: LIBS += -framework QxtCore -framework QxtWidgets
else:win32: LIBS += -lQxtCore -lQxtWidgets
