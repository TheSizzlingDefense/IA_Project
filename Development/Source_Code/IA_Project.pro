QT       += core gui
QT       += network
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
    aicreatewindow.cpp \
    decklistpanel.cpp \
    modeselectorpanel.cpp \
    studypanel.cpp \
    sqlite3.c

HEADERS += \
    addcardwindow.h \
    addlistwindow.h \
    database.h \
    mainwindow.h \
    spacedrepetitioncalculator.h \
    aicreatewindow.h \
    decklistpanel.h \
    modeselectorpanel.h \
    studypanel.h \
    sqlite3.h

FORMS += \
    addcardwindow.ui \
    addlistwindow.ui \
    mainwindow.ui \
    aicreatewindow.ui \
    decklistpanel.ui \
    modeselectorpanel.ui \
    studypanel.ui
# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
