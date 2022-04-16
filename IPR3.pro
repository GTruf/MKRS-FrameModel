QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

DEFINES += PROJECT_PATH=\"\\\"$${_PRO_FILE_PWD_}/\\\"\"

SOURCES += \
    src/frame.cpp \
    src/framecomboboxmodel.cpp \
    src/framemodelwidget.cpp \
    src/main.cpp \
    src/mainwindow.cpp

HEADERS += \
    src/frame.h \
    src/framecomboboxmodel.h \
    src/framemodelwidget.h \
    src/mainwindow.h

FORMS += \
    src/mainwindow.ui

INCLUDEPATH += \
    src

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
