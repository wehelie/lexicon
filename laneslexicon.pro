#-------------------------------------------------
#
#
#-------------------------------------------------
QT       += core gui printsupport
QT       += webkitwidgets network
QT       += xml sql

CONFIG   += debug
QMAKE_CXXFLAGS += -g

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = laneslexicon
TEMPLATE = app

INCLUDEPATH += /usr/include/xalanc/PlatformSupport
INCLUDEPATH += /usr/include/xalanc/XalanTransformer

#LIBS +=   -lboost_thread-mt -lboost_system -lboost_filesystem
LIBS += -lxalan-c -lxalanMsg -lxerces-c -lxerces-depdom
MOC_DIR = ./moc
OBJECTS_DIR = ./obj
QMAKE_CXXFLAGS += -Wunused-parameter
SOURCES += main.cpp\
        eventtype.cpp \
        laneslexicon.cpp \
        contentswidget.cpp \
        graphicsentry.cpp \
        xsltsupport.cpp \
        searchwidget.cpp \
        noteswidget.cpp


HEADERS  += laneslexicon.h \
            contentswidget.h \
            graphicsentry.h \
            xsltsupport.h \
            searchwidget.h \
            noteswidget.h
