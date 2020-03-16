/*
* Brief: Receive data module
* Author: GYW
* LOG:
*   - Created 20190926
*/
#pragma once

#include "common.h"
#include "hbipc/hbipc_ap.h"
#include "hbipc/hbipc_errno.h"
#include "interfacedevicestream.hpp"


//////////////////////////////////////////////////////////////////////////
// receive data from device
class RecvDevice : public InterfaceDeviceStream
{
public:
    RecvDevice();
    ~RecvDevice();
public:
    virtual void setReceiver(InterfaceMsgContainer *container) override;

    virtual bool login(const char *ip, const char *user, const char *passwd) override;
    virtual bool logout(const char *ip) override;

    virtual DeviceConfig config() override;
    virtual bool setConfig(const DeviceConfig* cfg) override;

    virtual void start() override;
    virtual void stop() override;

private:
    InterfaceMsgContainer *m_container;
    void getVideoSmart();

public:
    int Init();
    int Uinit();
    void RecvVideo();
    void RecvSmart();

    bool video_source_ok_;
    bool smart_source_ok_;

    // for smart
    int domain_id_ = -1;
    int provider_id_ = -1;
    struct session session_connect_;

    std::shared_ptr<std::thread> m_videoThread;
    std::shared_ptr<std::thread> m_smartThread;
    std::shared_ptr<std::thread> m_syncThread;
    std::shared_ptr<std::thread> m_initThread;

    std::mutex m_videoMutex;     
    std::mutex m_smartMUtex;
    std::queue<VIDEO_FRAME*> m_videoQueue;
    std::queue<SMART_FRAME*> m_smartQueue;
    std::atomic_bool m_bPlaying;
};
