//
// Created by yulingong on 2019/12/25.
//

#ifndef XPLUGINFLOW_SMART_VIDEO_H
#define XPLUGINFLOW_SMART_VIDEO_H

#include <unistd.h>
#include <signal.h>
#include <sstream>
#include <atomic>
#include <mutex>
#include <queue>
#include <iostream>
#include "xpluginflow/plugin/xpluginasync.h"
#include "xpluginflow/message/pluginflow/flowmsg.h"
#include "xpluginflow/message/pluginflow/msg_registry.h"
#include "xpluginflow/manager/msg_manager.h"
#include "hobotlog/hobotlog.hpp"

#include "interfacedevicestream.hpp"
extern "C" {
#include "read_frame.h"
}
/*
 * 添加自定义plugin分为三步：
 * 1.自定义消息类型，名字全局唯一；
 * 2.在生成消息的Plugin部分定义新的Message，新的Message需要继承XPluginFlowMessage,
 *   实现序列化和反序列化函数;
 * 3.继承XPluginAsync实现自定义Plugin：
 *   a.实现消息处理函数；
 *   b.覆盖Init函数，在其中完成监听消息注册及其他初始化工作，并在函数返回前调用父plugin的Init方法;
 * 4.在main函数中向总线注册新的消息类型及其反序列化函数。
*/

using horizon::vision::xpluginflow::XPluginAsync;
using horizon::vision::xpluginflow::XPluginFlowMessage;
using horizon::vision::xpluginflow::XPluginFlowMessagePtr;
using horizon::vision::xpluginflow::XMsgQueue;

using std::chrono::milliseconds;

#define TYPE_SMART_MESSAGE "XPLUGIN_SMART_MESSAGE"

struct SMART_FRAME
{
    char *data;
    int size;
};

struct SmartMessage : XPluginFlowMessage {
    std::string m_str;
    explicit SmartMessage(const std::string &str) : m_str(str) {
        type_ = TYPE_SMART_MESSAGE;
    }
    std::string Serialize() override {
        return "";
    }

    static XPluginFlowMessagePtr Deserialize(const std::string &str) {
        auto np_msg = std::make_shared<SmartMessage>(str);
        return np_msg;
    }
};

class SmartConsumerPlugin : public XPluginAsync, public InterfaceDeviceStream {
public:
    SmartConsumerPlugin() {}
    ~SmartConsumerPlugin() {
        Uinit();
    };
    int Init() override {
        int ret = init(); // init video data source
        if (ret != 0)
        {
            printf("init : error, %d", ret);
            return ret;
        }
        else {
            printf("====Init video data source success!====\n");
        }

        RegisterMsg(TYPE_SMART_MESSAGE, std::bind(&SmartConsumerPlugin::GetSmartMessage,
                                                  this, std::placeholders::_1));
        m_bPlaying = true;
        m_videoThread = std::make_shared<std::thread>(&SmartConsumerPlugin::RecvVideo, this);

        return XPluginAsync::Init();
    }
    int Uinit()
    {
        printf("Enter RecvDevice::Uinit()\n");
        m_bPlaying = false;

        uinit(); // deinit video source

        while(!m_smartQueue.empty()) {
            SMART_FRAME *smartFrame = nullptr;
            smartFrame = m_smartQueue.front();
            m_smartQueue.pop();
            delete[] smartFrame->data;
            smartFrame->data = nullptr;
        }

        if(m_videoThread && m_videoThread->joinable()) {
            m_videoThread->join();
            m_videoThread = nullptr;
        }

        printf("Exit RecvDevice::Uinit()\n");
        return 0;
    }

    void RecvVideo() {
        printf("====Enter RecvDevice::RecvVideo()====\n");
        uint32_t length = 0;
        void *buf = nullptr;
        int buf_index = -1, size = -1;

        try
        {
            while (m_bPlaying)
            {
                printf("====start get a video====\n");
                size = dequeue_buf(&buf, &length, &buf_index);
                if (size < 0)
                {
                    printf("dequeue_buf failed! ret = %d\n", size);
                    break;
                }
                printf("====get a video success====\n");

                int frame_id = ((((unsigned int)((char*)buf)[0]) & 0x000000FF) << 8) + ((char*)buf)[1];

                PureVideoBlob video;
                video.height = 1080;
                video.width = 1920;
                video.size = 1920 * 1080 * 3 / 2;
                video.pixels = (int8_t *)buf;

                if(!m_smartQueue.empty()) {
                    SMART_FRAME *smartFrame = nullptr;
                    std::unique_lock<std::mutex> lock(m_smartMutex);
                    smartFrame = m_smartQueue.front();
                    m_smartQueue.pop();
                    lock.unlock();
                    SmartDataBlob smart;
                    smart.frame_type = NORMAL_SMART_FRAME_ID;
                    smart.size = smartFrame->size;
                    smart.data = (int8_t *) smartFrame->data;

                    m_container->recvVideoBlob(&video, &smart);

                    delete[] smartFrame->data;
                    smartFrame->data = nullptr;
                    smartFrame = nullptr;
                    printf("====smartQueue size: %d====\n", m_smartQueue.size());
                }
                queue_buf(buf_index); //释放视频系统内存
            }
        }
        catch (std::exception &e)
        {
            printf("%s, exception: %s", __FUNCTION__, e.what());
        }
        printf("====Exit RecvDevice::RecvVideo()====\n");
        return;
    }

    int GetSmartMessage(XPluginFlowMessagePtr msg) {
        auto np_msg = std::static_pointer_cast<SmartMessage>(msg);
        SMART_FRAME *smartFrame = new SMART_FRAME;
        smartFrame->size = np_msg->m_str.size();
        smartFrame->data = new char[smartFrame->size];
        memcpy(smartFrame->data, np_msg->m_str.c_str(), smartFrame->size);
        std::lock_guard<std::mutex> lck(m_smartMutex);
        m_smartQueue.push(smartFrame);
    }

    std::string desc() const {
        return "SmartConsumerPlugin";
    }
public:
    virtual void setReceiver(InterfaceMsgContainer *container) override {
        m_container = container;
    }

    virtual bool login(const char *ip, const char *user, const char *passwd) override {return false;}
    virtual bool logout(const char *ip) override {return false;}

    virtual DeviceConfig config() override {return DeviceConfig();}
    virtual bool setConfig(const DeviceConfig* cfg) override {return false;}

    virtual void start() override {}
    virtual void stop() override{}

private:
    InterfaceMsgContainer *m_container;
    std::mutex m_smartMutex;
    std::queue<SMART_FRAME*> m_smartQueue;
    std::atomic_bool m_bPlaying;
    std::shared_ptr<std::thread> m_videoThread;
};

#endif //XPLUGINFLOW_SMART_VIDEO_H
