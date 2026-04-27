QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

SOURCE_DIR = ./sources
HEADER_DIR = ./headers
FORM_DIR = ./forms

INCLUDEPATH += $$HEADER_DIR

SOURCES += $$files($$SOURCE_DIR/*.cpp)

HEADERS += $$files($$HEADER_DIR/*.h) \
    headers/cmdenum.h \
    headers/modulations.h

FORMS += $$files($$FORM_DIR/*.ui)

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    views.qrc

win32: LIBS += -Wl,--whole-archive -lssp -Wl,--no-whole-archive
