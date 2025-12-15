QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17
# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    addcardwindow.cpp \
    addlistwindow.cpp \
    database.cpp \
    main.cpp \
    mainwindow.cpp \
    spacedrepetitioncalculator.cpp \
    sqlite3.c \
    studywindow.cpp

HEADERS += \
    addcardwindow.h \
    addlistwindow.h \
    database.h \
    mainwindow.h \
    spacedrepetitioncalculator.h \
    sqlite3.h \
    studywindow.h

FORMS += \
    addcardwindow.ui \
    addlistwindow.ui \
    mainwindow.ui \
    studywindow.ui
# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
