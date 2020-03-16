#ifndef INTERFACE_DEVICE_STREAM_H
#define INTERFACE_DEVICE_STREAM_H

#include <stdint.h>
#include "interface.hpp"

struct PureVideoBlob
{
    int32_t width;
    int32_t height;
    int8_t *pixels;
    int32_t size;
};

#define NORMAL_SMART_FRAME_ID 1
//SmartDataBlob messages are processed in lua engine, which is written in script can be modified dynamic.
struct SmartDataBlob
{
    int32_t type;//set in device stream plugin, identified in lua engine.

    int8_t *data;
    int32_t size;
    int32_t frame_type;
};

struct DeviceConfig
{
    int32_t dtype;//ipc, matrix
    int32_t vtype;//yuv, jpeg

    int32_t width;
    int32_t height;
    int32_t fps;

    char ip[32];
};

class InterfaceMsgContainer
{
public:
    //these functions maybe execute in another thread, implemention must be thread safe
    virtual void recvVideoBlob(const PureVideoBlob *video, const SmartDataBlob *data) = 0;
    virtual void recvCommnBlob(const SmartDataBlob *data) = 0;
};

class InterfaceDeviceStream
{
public:
    virtual void setReceiver(InterfaceMsgContainer *container) = 0;

    virtual bool login(const char *ip, const char *user, const char *passwd) = 0;
    virtual bool logout(const char *ip) = 0;

    virtual DeviceConfig config() = 0;
    virtual bool setConfig(const DeviceConfig* cfg) = 0;

    virtual void start() = 0;
    virtual void stop() = 0;
};
PLUGIN_DEFINE(DeviceStream, InterfaceDeviceStream)


#endif
