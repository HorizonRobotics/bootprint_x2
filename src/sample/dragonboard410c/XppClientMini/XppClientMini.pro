
include(../3rdparty/myqt.pri)

TEMPLATE = app
CONFIG += c++11 console
CONFIG -= app_bundle qt


DESTDIR = $$PWD/../bin/$${PLATFORM}


INCLUDEPATH += ../3rdparty/sglite/include
LIBS += -L../3rdparty/sglite/$${OSDIR} -lsgcore$${BUILD_SUFFIX}

INCLUDEPATH += ../3rdparty/glfw/include
LIBS += -L../3rdparty/glfw/$${OSDIR} -lglfw3

INCLUDEPATH += ../3rdparty/protobuf/include
LIBS += -L../3rdparty/protobuf/$${OSDIR} -lglfw3 -llibprotobuf-lite$${BUILD_SUFFIX}


HEADERS += view.h scene.h pb/x2.pb.h

SOURCES += view.cpp scene.cpp pb/x2.pb.cc main.cpp
