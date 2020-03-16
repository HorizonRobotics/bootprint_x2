TEMPLATE = subdirs
CONFIG += c++11
#CONFIG += ordered

SUBDIRS = \
    SmartVideo \
	test

#CONFIG += ordered

CONFIG(debug, debug|release) {
    BUILD_SUFFIX = d
    CONFIG += console
    BUILD_DIR = debug
} else {
    BUILD_DIR = release
    BUILD_SUFFIX =
}


#MY_OUT_PATH = $$PWD/bin/$${OSDIR}
#DESTDIR = MY_OUT_PATH
#OBJECTS_DIR = $${BUILD_DIR}/.obj
#MOC_DIR = $${BUILD_DIR}/.moc
#RCC_DIR = $${BUILD_DIR}/.rcc
#UI_DIR = $${BUILD_DIR}/.ui
#

