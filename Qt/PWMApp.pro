QT += core gui serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

CONFIG += c++11 -Os

INCLUDEPATH += \
            forms/headers \
            forms/src \
            forms/ui

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
RC_ICONS = images/icon.ico

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    forms/src/about.cpp \
    forms/src/mainwindow.cpp \
    forms/src/tutorialdialog.cpp \
    include/qcustomplot.cpp \
    include/serialportwriter.cpp \
    main.cpp

HEADERS += \
    forms/headers/about.h \
    forms/headers/mainwindow.h \
    forms/headers/tutorialdialog.h \
    include/qcustomplot.h \
    include/serialportwriter.h \


FORMS += \
    forms/ui/about.ui \
    forms/ui/mainwindow.ui \
    forms/ui/tutorialdialog.ui \

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc

DISTFILES +=
