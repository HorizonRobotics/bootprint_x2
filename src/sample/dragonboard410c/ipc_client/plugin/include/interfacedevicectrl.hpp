#ifndef INTERFACE_DEVICE_CTRL_H
#define INTERFACE_DEVICE_CTRL_H

#include <string>
#include "interface.hpp"

//these interface use json string as all parameters or return values
/*
{
    type: 1(get)   2(set)
    url: ip
    module: 1(scan), 2(snap), 3(flow)
    desc: object
}
*/
class InterfaceDeviceCtrl
{
public:
    virtual int login(const char *ip, const char *user, const char *passwd, std::string *err_msg) = 0;
    virtual bool logout(const char *ip) = 0;

    virtual const char* get(const char* msg) = 0;
    virtual const char* post(const char* msg) = 0;
    virtual void free(const char* msg) = 0;//message gotten should be free with this

};
PLUGIN_DEFINE(DeviceCtrl, InterfaceDeviceCtrl)

#endif
