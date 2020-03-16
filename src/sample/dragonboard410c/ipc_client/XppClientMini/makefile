.PHONY : mingw xppclient macos linux undefined

CFLAGS := -std=c++11 -Wall -I../3rdparty/sglite/include -I../3rdparty/protobuf/include -I../3rdparty/glfw/include
LDFLAGS := -L../3rdparty/sglite/linux -lsgcore -L../3rdparty/protobuf/linux -lprotobuf -L../3rdparty/glfw/linux -lglfw

SRC := \
    scene.cpp \
    pb/x2.pb.cc \
    view.cpp \
    main.cpp


UNAME=$(shell uname)
SYS=$(if $(filter Linux%,$(UNAME)),linux,\
	    $(if $(filter MINGW%,$(UNAME)),mingw,\
	        undefined\
))

all: $(SYS)

undefined:
	@echo "I can't guess your platform, please do 'make PLATFORM' where PLATFORM is one of these:"
	@echo "      linux mingw"


mingw : TARGET := xppclientmini.exe
mingw : CFLAGS += -I/usr/include -I/usr/local/include -I.
mingw : LDFLAGS += -L/usr/bin -lglfw3 -L/usr/local/bin
linux : TARGET := xppclientmini.out
linux : CFLAGS += -I/usr/include -I/usr/local/include -I.
linux : LDFLAGS += -L/usr/bin -lglfw -ldl -pthread -L/usr/local/bin


mingw : $(SRC) xppclient
linux : $(SRC) xppclient

xppclient :
	g++ $(CFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

.PHONY: install clean
install:
	cp ../3rdparty/glfw/linux/libglfw.so.3 .
	cp ../3rdparty/hbipc/linux/libhbipc_ap.so .
	cp ../3rdparty/protobuf/linux/libprotobuf.so.17 .
	cp ../3rdparty/sglite/linux/libsgcore.so .
	#copy a plugin, xppclientmini will load it
	cp ../plugin/96boardstream/src/lib96boardstream.so .
	
clean:
	-rm -f $(TARGET)
