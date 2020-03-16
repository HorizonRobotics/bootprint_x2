contains(QT_ARCH, i386) {
    PLATFORM = x86
    ARCH_SUFFIX = 
} else {
    PLATFORM = x64
    ARCH_SUFFIX = 64
}

CONFIG(debug, debug|release) {
    BUILD_SUFFIX = d
} else {
    BUILD_SUFFIX =
}

win32:OSDIR = win_$$PLATFORM
macx:OSDIR = macx
linux:OSDIR = linux_$$PLATFORM


win32: {
    QMAKE_CXXFLAGS += /MP /Zi
    QMAKE_LFLAGS += /DEBUG
    CONFIG -= flat
}