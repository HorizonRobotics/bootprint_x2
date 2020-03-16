.PHONY : mingw macos linux 96board undefined

CXXFLAGS := -shared -std=c++11 -Wall -fPIC -DBUILDING_PLUGIN -I../../include -I../../../3rdparty/hbipc/include -I../../../3rdparty/protobuf/include
LDFLAGS := -L../../../3rdparty/hbipc/linux -lhbipc_ap -L../../../3rdparty/protobuf/linux -lprotobuf

SRC := \
    common.cpp \
    pack.pb.cc \
    recv_device.cpp


UNAME=$(shell uname)
SYS=$(if $(filter Linux%,$(UNAME)),linux,\
	    $(if $(filter MINGW%,$(UNAME)),mingw,\
	        undefined\
))

all: $(SYS)

undefined:
	@echo "I can't guess your platform, please do 'make PLATFORM' where PLATFORM is one of these:"
	@echo "      linux mingw"


mingw : TARGET := 96boardstream.dll
mingw : CFLAGS += -I/usr/include -I/usr/local/include -I.
mingw : LDFLAGS += -L/usr/bin -lglfw3 -L/usr/local/bin
linux : TARGET := lib96boardstream.so
linux : CXXFLAGS += -I/usr/include -I/usr/local/include -I.
linux : CFLAGS += -I/usr/include -I/usr/local/include -I.
linux : LDFLAGS += -L/usr/bin -L/usr/local/bin

mingw : $(SRC) 96board
linux : $(SRC) 96board

read_frame.o : read_frame.c
	gcc $(CFLAGS) -c read_frame.c
96board : read_frame.o
	g++ $(CXXFLAGS) -o $(TARGET) read_frame.o $(SRC) $(LDFLAGS)

.PHONY: clean
clean:
	-rm -f $(TARGET)
