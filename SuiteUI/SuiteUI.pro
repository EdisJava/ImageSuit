QT+= core gui widgets concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    cardcontrols.cpp \
    downloadedwidget.cpp \
    downloadwidget.cpp \
    imagecarddelegate.cpp \
    imageviewer.cpp \
    main.cpp \
    mainwindow.cpp \


HEADERS += \
    cardcontrols.h \
    downloadedwidget.h \
    downloadwidget.h \
    imagecarddelegate.h \
    imageviewer.h \
    mainwindow.h

FORMS += \
    downloadedwidget.ui \
    downloadwidget.ui \
    imagecarddelegate.ui \
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

TRANSLATIONS += \
        translations/app_es.ts

RESOURCES += \
    resource.qrc
