#QT       += core gui opengl

PROJ_NAME = test
CONFIG += c++11
CONFIG(debug, debug|release) {
    BUILD_SUFFIX = d
	CONFIG += console
	BUILD_DIR = debug
} else {
	BUILD_SUFFIX =
	BUILD_DIR = release
}

TARGET = $${PROJ_NAME}$${BUILD_SUFFIX}
TEMPLATE = app
DEFINES += QT_DEPRECATED_WARNINGS

INCLUDEPATH += $$PWD/../depend/protobuf/include $$PWD/../depend/hbipc/include ./
win32:{
LIBS += $$PWD/../depend/protobuf/lib/$${OSDIR}/libprotobuf$${BUILD_SUFFIX}.lib
} else {
LIBS += $$PWD/../depend/protobuf/lib/$${OSDIR}/libprotobuf$${BUILD_SUFFIX}.a
LIBS += -L$$PWD/../depend/hbipc/lib/$${OSDIR} -lhbipc_ap -lpb_ap_recv
}

SOURCES += *.cpp *.cc
HEADERS += *.h
FORMS += *.ui

DESTDIR = $${MY_OUT_PATH}
OBJECTS_DIR = $${MY_BASE_PATH}/$${BUILD_DIR}/$${PROJ_NAME}/.obj
MOC_DIR = $${MY_BASE_PATH}/$${BUILD_DIR}/$${PROJ_NAME}/.moc
RCC_DIR = $${MY_BASE_PATH}/$${BUILD_DIR}/$${PROJ_NAME}/.rcc
UI_DIR = $${MY_BASE_PATH}/$${BUILD_DIR}/$${PROJ_NAME}/.ui
