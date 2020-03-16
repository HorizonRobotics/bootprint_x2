#ifndef READER_H
#define READER_H

#include "../../include/interfacedevicestream.hpp"
#include <thread>
#include <atomic>
#include <string>


class Reader : public InterfaceDeviceStream
{
public:
    Reader(const char*srcfile);
    virtual void setReceiver(InterfaceMsgContainer *container) override;

    virtual bool login(const char *ip, const char *user, const char *passwd) override;
    virtual bool logout(const char *ip) override;

    virtual DeviceConfig config() override;
    virtual bool setConfig(const DeviceConfig* cfg) override;

    virtual void start() override;
    virtual void stop() override;

    void procReadFromFile();

private:
    InterfaceMsgContainer *m_container;
    std::thread m_reader;
    std::atomic_bool m_runningflag;
    std::atomic_bool m_producingflag;

    std::string m_srcfile;
};

#endif // READER_H
