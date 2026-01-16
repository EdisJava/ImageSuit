QT+= core gui widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    downloadedwidget.cpp \
    downloadwidget.cpp \
    imageviewer.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    downloadedwidget.h \
    downloadwidget.h \
    imageviewer.h \
    mainwindow.h

FORMS += \
    downloadedwidget.ui \
    downloadwidget.ui \
    imageviewer.ui \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../SuiteCore/release/ -lSuiteCore
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../SuiteCore/debug/ -lSuiteCore
else:unix: LIBS += -L$$OUT_PWD/../SuiteCore/ -lSuiteCore

INCLUDEPATH += $$PWD/../SuiteCore
DEPENDPATH += $$PWD/../SuiteCore

RESOURCES += \
    resource.qrc
