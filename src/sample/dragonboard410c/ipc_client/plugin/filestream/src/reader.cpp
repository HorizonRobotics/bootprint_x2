#include "reader.h"
#include <stdio.h>
#include <cstdio>
#include <chrono> 
#include <assert.h>
#include <iostream>

Reader::Reader(const char*srcfile):
    m_srcfile(srcfile)
{
    m_runningflag.store(false);
    m_producingflag.store(false);
}

void Reader::setReceiver(InterfaceMsgContainer *container)
{
    m_container = container;
}

bool Reader::login(const char * ip, const char * user, const char * passwd)
{
    stop();
    //to do in future
    return true;
}

bool Reader::logout(const char * ip)
{
    (void*)(ip);
    stop();
    return true;
}

DeviceConfig Reader::config()
{
    return DeviceConfig();
}

bool Reader::setConfig(const DeviceConfig* cfg)
{
    return true;
}

void Reader::procReadFromFile()
{
    FILE *file = fopen(m_srcfile.c_str(), "rb");
	FILE *smartfile = fopen("data/smart.data", "rb");
    if(!file || !smartfile)
    {
        std::cout<<"open file failed..."<<std::endl;
        return;
    }
    PureVideoBlob video;
    video.height = 1080;
    video.width = 1920;
    video.size = 1920*540*3;
    video.pixels = (int8_t*)malloc(video.size);

    SmartDataBlob data;
    data.size = 0;
    data.data = (int8_t*)malloc(video.size);
    data.type = 0;
    
    int dattype = 7;
    int datalen = 0;
    while(m_producingflag==true)
    {
        int cnt = fread(&dattype, 1, 4, file);
        if(cnt != 4)
        {
            fseek(file, 0, SEEK_SET);
            continue;
        }

        printf("[%d]\n", dattype);

        //smart video
        if(dattype == 0)
        {
            cnt = fread(&datalen, 1, 4, file);
            assert(datalen == video.size);
            cnt = fread(video.pixels, 1, datalen, file);

        }
            
        fread(&datalen, 1, 4, file);
        data.size = datalen;
        if(datalen>0)
        {
            cnt = fread(data.data, 1, datalen, file);
        }

//
if(dattype == 0)
{
int smartsize=0;
cnt = fread(&smartsize, 1, sizeof(int), smartfile);
if(cnt != 4)
{
printf("smart file end.");
    fseek(smartfile, 0, SEEK_SET);
    fseek(file, 0, SEEK_SET);
    continue;
}
data.size = smartsize;
fread(data.data, 1, smartsize, smartfile);
}
//
        assert(m_container);
        if(dattype == 0)
            m_container->recvVideoBlob(&video, &data);
        else
            m_container->recvCommnBlob(&data);
        std::this_thread::sleep_for(std::chrono::milliseconds(40));
    }

    fclose(file);
    printf("thread finished...");
    m_runningflag.store(false);
}

void Reader::start()
{
    if(!m_runningflag==true)
    {
        m_runningflag.store(true);
        m_producingflag.store(true);
        m_reader = std::thread([this]{this->procReadFromFile();});
        m_reader.detach();
    }
}

void Reader::stop()
{
    if(m_runningflag==true)
    {
        m_producingflag.store(false);
        //wait for finish...
        std::this_thread::sleep_for(std::chrono::milliseconds(80));
    }
}

//creator and destroyer
InterfaceDeviceStream* CreateDeviceStream()
{
    return new Reader("data/data.dat");
}

void DestroyDeviceStream(InterfaceDeviceStream *stream)
{
    Reader *reader = dynamic_cast<Reader*>(stream);
    delete reader;
}
