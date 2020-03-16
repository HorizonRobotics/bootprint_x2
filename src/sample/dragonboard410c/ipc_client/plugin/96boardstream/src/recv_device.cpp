#include <unistd.h>
#include "recv_device.h"
#include "pack.pb.h"
extern "C" {
#include "read_frame.h"
}
UUID_DEFINE(server_id, 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xe);

RecvDevice::RecvDevice()
{
    //Init();
}

RecvDevice::~RecvDevice()
{
    Uinit();
}

void RecvDevice::setReceiver(InterfaceMsgContainer * container)
{
    m_container = container;
}

bool RecvDevice::login(const char * ip, const char * user, const char * passwd)
{
    return true;
}

bool RecvDevice::logout(const char * ip)
{
    return true;
}

DeviceConfig RecvDevice::config()
{
    return DeviceConfig();
}

bool RecvDevice::setConfig(const DeviceConfig * cfg)
{
    return false;
}

void RecvDevice::start()
{
}

void RecvDevice::stop()
{

}

int RecvDevice::Init()
{
    printf("Enter RecvDevice::Init()\n");

    // init video data source
    int ret = init();
    if (ret != 0)
    {
        printf("init : error, %d", ret);
        return EC_HBIPC_ERROR;
    }
    else {
        printf("====Init video data source success!====\n");
    }

    // init smart data source
    domain_configuration domain_config_;
    domain_config_.domain_number = 1;
    domain_config_.domain_array[0].domain_name = (char*)"X2BIF001";
    domain_config_.domain_array[0].domain_id = 0;
    domain_config_.domain_array[0].device_name = (char*)"/dev/x2_bif";
    ret = hbipc_ap_init("X2BIF001", TF_BLOCK, 0, &domain_config_);
    if (ret < 0)
    {
        printf("hbipc_ap_init failed! ret = %d, name: %s, path: %s\n", ret,
            domain_config_.domain_array[0].domain_name,
            domain_config_.domain_array[0].device_name);
        return EC_HBIPC_ERROR;
    }
    else {
        printf("====hbipc_ap_init success!====\n");
    }
    domain_id_ = ret;

    printf("====Begin hbipc_ap_startserver...====\n");
    while(true) {
        printf("====Try hbipc_ap_startserver...\n");
        ret = hbipc_ap_startserver(domain_id_, server_id, NULL, TF_NONBLOCK);
        if (ret < 0) {
            sleep(1);
        } else {
            break;
        }
    }
#if 0
    if (ret < 0)
    {
        printf("hbipc_ap_startserver failed! ret = %d\n", ret);
        return EC_HBIPC_ERROR;
    }
    else {
        printf("====hbipc_ap_startserver success!====\n");
    }
#else
    printf("====hbipc_ap_startserver success!====\n");
#endif
    provider_id_ = ret;

    ret = hbipc_ap_connect(domain_id_, provider_id_, &session_connect_);
    if (ret < 0)
    {
        printf("hbipc_ap_connect failed! ret = %d\n", ret);
        return EC_HBIPC_ERROR;
    }
    else {
        printf("====hbipc_ap_connect success!====\n");
    }

    m_bPlaying = true;
    m_videoThread = std::make_shared<std::thread>(&RecvDevice::RecvVideo, this);
    m_smartThread = std::make_shared<std::thread>(&RecvDevice::RecvSmart, this);
    //m_syncThread = std::make_shared<std::thread>(&RecvDevice::getVideoSmart, this);
    
    printf("Exit RecvDevice::Init()\n");
    return 0;
}

int RecvDevice::Uinit()
{
    printf("Enter RecvDevice::Uinit()\n");
    m_bPlaying = false;

    // deinit video source
    uinit();

    // deinit smart source
    int ret = hbipc_ap_disconnect(&session_connect_);
    if (ret < 0) {
        printf("hbipc_ap_disconnect failed! ret = %d\n", ret);
    }
    else {
        printf("====hbipc_ap_disconnect success!====\n");
    }

    ret = hbipc_ap_stopserver(domain_id_, provider_id_);
    if (ret < 0) {
        printf("hbipc_ap_stopserver failed! ret = %d\n", ret);
    }
    else {
        printf("====hbipc_ap_stopserver success!====\n");
    }
    ret = hbipc_ap_deinit(domain_id_);
    if (ret < 0) {
        printf("hbipc_ap_deinit failed! ret = %d\n", ret);
    }
    else {
        printf("====hbipc_ap_deinit success!====\n");
    }

    while(!m_smartQueue.empty()) {
        SMART_FRAME *smartFrame = nullptr;
        smartFrame = m_smartQueue.front();
        m_smartQueue.pop();
        delete[] smartFrame->data;
        smartFrame->data = nullptr;
    }
    while(!m_videoQueue.empty()) {
        VIDEO_FRAME *videoFrame = nullptr;
        videoFrame = m_videoQueue.front();
        m_videoQueue.pop();
        delete[] videoFrame->data;
        videoFrame->data = nullptr;
    }

    if(m_videoThread && m_videoThread->joinable()) {
        m_videoThread->join();
        m_videoThread = nullptr;
    }
    if(m_smartThread && m_smartThread->joinable()) {
        m_smartThread->join();
        m_smartThread = nullptr;
    }
    if(m_initThread && m_initThread->joinable()) {
	m_initThread->join();
	m_initThread = nullptr;
    }
//    if(m_syncThread && m_syncThread->joinable()) {
//        m_syncThread->join();
//        m_syncThread = nullptr;
//    }

    printf("Exit RecvDevice::Uinit()\n");
    return 0;
}

void RecvDevice::RecvVideo()
{
    printf("====Enter RecvDevice::RecvVideo()====\n");
    uint32_t length = 0;
    void *buf = nullptr;
    int buf_index = -1, size = -1;

    try
    {
        while (m_bPlaying)
        {
            //printf("====start get a video====\n");
            size = dequeue_buf(&buf, &length, &buf_index);
            if (size < 0)
            {
                printf("dequeue_buf failed! ret = %d\n", size);
                continue;
            }
            //printf("====get a video success====\n");

            //int frame_id = ((((unsigned int)((char*)buf)[0]) & 0x000000FF) << 8) + ((char*)buf)[1];
            PureVideoBlob video;
            video.height = 1080;
            video.width = 1920;
            video.size = 1920 * 1080 * 3 / 2;
            video.pixels = (int8_t *)buf;

            if(!m_smartQueue.empty()) {
                SMART_FRAME *smartFrame = nullptr;
                std::unique_lock<std::mutex> lock(m_smartMUtex);
                smartFrame = m_smartQueue.front();
                m_smartQueue.pop();
                lock.unlock();

                SmartDataBlob smart;
                smart.size = smartFrame->size;
                smart.data = (int8_t *) smartFrame->data;
                smart.frame_type = smartFrame->frame_type;

                m_container->recvVideoBlob(&video, &smart);

                deleteSmartFrame(smartFrame);
                printf("smart queue size = %d\n", m_smartQueue.size());
            }
            queue_buf(buf_index); //释放视频系统内存
        }
    }
    catch (std::exception &e)
    {
        printf("%s, exception: %s", __FUNCTION__, e.what());
    }
    printf("====Exit RecvDevice::RecvVideo()====\n");
}

void RecvDevice::RecvSmart()
{
    printf("====Enter RecvDevice::RecvSmart()====\n");
    int index = 0, size = -1, len = 409600;
    char *buf = new char[len];
    printf("====buf size: 409600====\n");
    try
    {
        while (m_bPlaying)
        {
            memset(buf, 0, len);
    	    printf("====recv smart frame====\n");
            size = hbipc_ap_recvframe(&session_connect_, buf, len);
            if (size < 0)
            {
                printf("hbipc_ap_recvframe failed! ret = %d\n", size);
                continue;
            }
	        printf("===recv success===%d\n",size);

            pack::MessagePack packMessage;
            std::string buf_str(buf, size);
            if(!packMessage.ParseFromString(buf_str)) {
                printf("packMessage.ParseFromString failed!\n");
            }
            int frame_type = packMessage.addition_().frame_().frame_type_();
	        std::string contents = packMessage.content_();

            SMART_FRAME *smartFrame = new SMART_FRAME;
            smartFrame->size = contents.size();
            smartFrame->index = index;
            smartFrame->data = new char[contents.size()];
            smartFrame->frame_type = frame_type;
            memcpy(smartFrame->data, contents.c_str(), contents.size());
            std::unique_lock<std::mutex> lck(m_smartMUtex);
            m_smartQueue.push(smartFrame);
            lck.unlock();

            index++;
        }
    }
    catch (std::exception &e)
    {
        printf("%s, exception: %s", __FUNCTION__, e.what());
    }
    delete[] buf;

    printf("====Exit RecvDevice::RecvSmart()====\n");
}

void RecvDevice::getVideoSmart() {
    printf("====Enter RecvDevice::getVideoSmart()====\n");
    while(m_bPlaying) {
        if(!m_smartQueue.empty() && !m_videoQueue.empty()) {
            VIDEO_FRAME *videoFrame = nullptr;
            std::unique_lock<std::mutex> lck(m_videoMutex);
            videoFrame = m_videoQueue.front();
            m_videoQueue.pop();
            lck.unlock();
            PureVideoBlob video;
            video.height = 1080;
            video.width = 1920;
            video.size = 1920 * 1080 * 3 / 2;
            video.pixels = (int8_t *) videoFrame->data;

            SMART_FRAME *smartFrame = nullptr;
            std::unique_lock<std::mutex> lock(m_smartMUtex);
            smartFrame = m_smartQueue.front();
            m_smartQueue.pop();
            lock.unlock();
            SmartDataBlob smart;
            smart.size = smartFrame->size;
            smart.data = (int8_t *) smartFrame->data;
            smart.frame_type = smartFrame->frame_type;

            m_container->recvVideoBlob(&video, &smart);
        }
    }
    printf("====Exit RecvDevice::getVideoSmart()====\n");
}

InterfaceDeviceStream* CreateDeviceStream()
{
    RecvDevice *recvDevice = new RecvDevice;
    recvDevice->m_initThread = std::make_shared<std::thread>(&RecvDevice::Init, recvDevice);
    return recvDevice;
}

void DestroyDeviceStream(InterfaceDeviceStream *stream) {
    RecvDevice *device = dynamic_cast<RecvDevice*>(stream);
    delete device;
}
