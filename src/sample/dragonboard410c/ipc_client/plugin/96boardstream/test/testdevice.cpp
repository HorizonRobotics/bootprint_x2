#include "testdevice.h"
#include <hbipc/hbipc_ap.h>
#include <pb_ap_recv/pb_ap_recv.h>
#include <pb_ap_recv/pb_ap_recv_c.h>
#include <hbipc/hbipc_errno.h>

UUID_DEFINE(server_id, 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf);

domain_configuration domain_config_;
int domain_id_ = -1;
int provider_id_ = -1;
struct session session_connect_;
extern bool stop_flag_;

int HbipcInit()
{
    string domains[2] = { "X2BIF001", "X2SD001" };
    string devices[2] = { "/dev/x2_bif", "/dev/x2_sd" };
    domain_config_.domain_number = 2;
    int length = 32;
    for (int i = 0; i < domain_config_.domain_number; ++i)
    {
        domain_info &info = domain_config_.domain_array[i];
        info.domain_id = 0;
        info.domain_name = new char[length];
        memset(info.domain_name, 0, length);
        memcpy(info.domain_name, domains[i].c_str(), domains[i].size());

        info.device_name = new char[length];
        memset(info.device_name, 0, length);
        memcpy(info.device_name, devices[i].c_str(), devices[i].size());

        cout << "domain " << i << "- id: " << info.domain_id <<
            ", name: " << info.domain_name << ", device: " << info.device_name << endl;
    }

    int ret = -1;
    ret = hbipc_ap_init(domain_config_.domain_array[0].domain_name, TF_BLOCK, 0, &domain_config_);
    if (ret < 0)
        return ret;
    domain_id_ = ret;

    ret = hbipc_ap_startserver(domain_id_, server_id, NULL, TF_BLOCK);
    if (ret < 0)
        return -1;
    provider_id_ = ret;

    ret = hbipc_ap_connect(domain_id_, provider_id_, &session_connect_);
    if (ret < 0)
        return -1;

    cout << __FUNCTION__ << " ok" << endl;
    return 0;
}

int HbipcClose()
{
    for (int i = 0; i < domain_config_.domain_number; ++i)
    {
        domain_info &info = domain_config_.domain_array[i];
        if (info.domain_name)
        {
            delete info.domain_name;
            info.domain_name = nullptr;
        }
        if (info.device_name)
        {
            delete info.device_name;
            info.device_name = nullptr;
        }
        info.domain_id = 0;
    }

    int ret = -1;
    ret = hbipc_ap_disconnect(&session_connect_);
    ret = hbipc_ap_stopserver(domain_id_, provider_id_);
    ret = hbipc_ap_deinit(domain_id_);
    return 0;
}

int HbipcRecv()
{
    int index = 0, length, ret;
    int size = 4*1024;
    char *buf = new char[size];
    if (buf == nullptr)
        return -1;

    stop_flag_ = false;
    cout << __FUNCTION__ << " start" << endl;
    try
    {
        while (!stop_flag_)
        {
            memset(buf, 0, size);

            ret = hbipc_ap_recvframe(&session_connect_, buf, size);
            if (ret < 0)
            {
                //std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIME_RECV));
                continue;
            }

            length = ret;
            auto time_stamp = GetTimeStampFromPb(buf, length);
            cout << "recv: size " << length << ", time_stamp " << time_stamp << endl;
            index++;
            //std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIME_RECV));
        }
        cout << "Read smart data, sum " << index << endl;
    }
    catch (std::exception &e)
    {
        cout << __FUNCTION__ << " " << e.what() << endl;
    }
    delete buf;
    cout << __FUNCTION__ << " exit" << endl;
    return 0;
}
