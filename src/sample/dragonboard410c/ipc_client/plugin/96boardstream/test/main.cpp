#include "stl_inc.h"
#include "pbprocess.h"
#include "testdevice.h"
#include <csignal>

bool stop_flag_ = true;

int TestLocalData()
{
    X2Processer worker;

    const char *path = "smart";
    int num = 100;
    int num_out = worker.GenTestSmartData(path, num);
    if (num_out == 0)
        return -1;
    std::string fname = path;
    fname.append("/0");
    int length;
    char *buf = worker.LoadSmartData(fname.c_str(), length);
    if (buf == nullptr)
        return -1;
    worker.ParseSmart(buf, length);
    delete[] buf;
    return 0;
}

int TestHbipcSmart()
{
    int ec = -1;
    ec = HbipcInit();
    if (ec < 0)
    {
        cout << "init error " << ec << endl;
        return ec;
    }

    ec = HbipcRecv();

    ec = HbipcClose();

    return 0;
}

void sig_handler(int sig)
{
    stop_flag_ = true;
    return;
}

int main(int argc, char *argv[])
{
    signal(SIGINT, sig_handler);

    //TestLocalData();
    TestHbipcSmart();

    return 0;
}
