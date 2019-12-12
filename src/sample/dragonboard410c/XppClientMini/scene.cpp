#include "scene.h"

#ifdef _WIN32
#include "windows.h"
#else
#include <dlfcn.h>
#endif

#include <math.h>
#include <stdio.h>
#include <locale>
#include <codecvt>


enum GlobalDefineColor
{
    RED = 0xff0000,
    GREEN = 0x00ff00,
    BLUE = 0x0000ff,
    CYAN = 0x00ffff,
    SKELETON_1 = 0xff5500,
    SKELETON_2 = 0xff0055,
    SKELETON_3 = 0x55ff00,
    SKELETON_4 = 0x00ff55,
    SKELETON_5 = 0x0055ff,
    SKELETON_6 = 0x5500ff,
    SKELETON_7 = 0xffaa00,
    SKELETON_8 = 0xff00aa,
    SKELETON_9 = 0xaaff00,
    SKELETON_10 = 0x00ffaa,
    SKELETON_11 = 0xaa00ff,
    SKELETON_12 = 0x00aaff,
    SKELETON_13 = 0xff00ff,
};

static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

SmartScene::SmartScene()
{
    //set scene size
    this->setSize(1920, 1080);

    //init video node
    m_video.setSize(1920, 1080);
    this->addItem(&m_video);

    //pre-allocate some items to avoid add&remove frequently
    //or you should keep synchronization with renderer
    m_rectPool.create(this, 19 * 64);
    m_textPool.create(this, 2 * 64);
}

SmartScene::~SmartScene()
{
    {
        std::lock_guard<std::mutex> locker(m_mtxStop);
        m_stopStream = true;
    }
    m_cvStop.notify_one();

    m_thdStream.join();
}

void SmartScene::drawVideo(const unsigned char* yuvptr, unsigned int len)
{
    m_video.setYuv(yuvptr, len);
}

//convert line to rectangle
void SmartScene::drawLine(const x2::Point & point0, const x2::Point & point1, int color)
{
    static int lineSize = 2;
    static float KPS_THRESH = 0.2f;
    static float KPS_THRESH_MAX = 3.0f;

    if (point0.score_() > KPS_THRESH
        && point0.score_() < KPS_THRESH_MAX
        &&  point1.score_() > KPS_THRESH
        && point1.score_() < KPS_THRESH_MAX)
    {
        float x0 = point0.x_();
        float y0 = point0.y_();
        float x1 = point1.x_();
        float y1 = point1.y_();

        float deltax = x1 - x0, deltay = y1 - y0;
        float width = sqrtf(deltax*deltax + deltay * deltay);
        float height = lineSize;

        SGRectangleNode *rect = m_rectPool.alloc();
        if (rect)
        {
            rect->setRotation(-atan2(deltay, deltax));//rotation with anti-clockwise direction
            rect->setSize(width, height);
            rect->setPos((x0 + x1 - width) / 2.f, (y0 + y1 - height) / 2.f);
            rect->setColor(color);
        }
    }
}

void SmartScene::drawSkeleton(const x2::Points & points)
{
    assert (17 == points.points__size());

    drawLine(points.points_(15), points.points_(13), GlobalDefineColor::RED);
    drawLine(points.points_(13), points.points_(11), GlobalDefineColor::SKELETON_1);
    drawLine(points.points_(16), points.points_(14), GlobalDefineColor::SKELETON_7);
    drawLine(points.points_(14), points.points_(12), GlobalDefineColor::SKELETON_7);

    drawLine(points.points_(11), points.points_(12), GlobalDefineColor::SKELETON_9);
    drawLine(points.points_(5), points.points_(11), GlobalDefineColor::SKELETON_3);
    drawLine(points.points_(6), points.points_(12), GlobalDefineColor::GREEN);

    drawLine(points.points_(5), points.points_(6), GlobalDefineColor::SKELETON_4);
    drawLine(points.points_(5), points.points_(7), GlobalDefineColor::SKELETON_10);
    drawLine(points.points_(6), points.points_(8), GlobalDefineColor::CYAN);

    drawLine(points.points_(7), points.points_(9), GlobalDefineColor::SKELETON_12);
    drawLine(points.points_(8), points.points_(10), GlobalDefineColor::SKELETON_5);
    drawLine(points.points_(1), points.points_(2), GlobalDefineColor::BLUE);
    drawLine(points.points_(0), points.points_(1), GlobalDefineColor::SKELETON_6);
    drawLine(points.points_(0), points.points_(2), GlobalDefineColor::SKELETON_11);

    drawLine(points.points_(1), points.points_(3), GlobalDefineColor::SKELETON_13);
    drawLine(points.points_(2), points.points_(4), GlobalDefineColor::SKELETON_8);
    drawLine(points.points_(3), points.points_(5), GlobalDefineColor::SKELETON_2);
    drawLine(points.points_(4), points.points_(6), GlobalDefineColor::BLUE);
}

void SmartScene::recvVideoBlob(const PureVideoBlob * video, const SmartDataBlob * data)
{
    this->drawVideo((const unsigned char*)video->pixels, video->size);

    if (!data || !data->size) return;
    if (data->frame_type != NORMAL_SMART_FRAME_ID) {
        std::cout << "drop or error or unknow smart frame\n";
        return;
    }

    x2::FrameMessage frame;
    bool bret = frame.ParseFromArray(data->data, data->size);
    if (!bret)
    {
        std::cout << "Parse smart message failed" << std::endl;
        return;
    }
    x2::SmartFrameMessage smart = frame.smart_msg_();

    m_rectPool.clear();
    m_textPool.clear();
    for (int i = 0; i < smart.targets__size(); i++)
    {
        const x2::Target &target = smart.targets_(i);
        char  buffer[64];
        sprintf(buffer, "TrackId %ld", target.track_id_());
        std::wstring idLabel(converter.from_bytes(buffer));

        ///////////////////////////////////////////////////////////
        //display attribute

        //build attribute string
        std::wstring gender;
        std::wstring age;
        //注意源文件编码格式，编译器打开格式，二者需要保持一致
        for (int i = 0; i < target.attributes__size(); i++)
        {
            x2::Attributes attr = target.attributes_(i);
            if (attr.type_() == "gender")
            {
                gender = attr.value_() < 0 ? L"女性" : L"男性";
            }
            else if (attr.type_() == "age")
            {
                age = attr.value_() < 60 ? L"Young" : L"Old";
            }
        }

        //retrieve face and render attribute string around it.
        for (int i = 0; i < target.boxes__size(); i++)
        {
            x2::Box box = target.boxes_(i);
            x2::Point tl = box.top_left_();
            x2::Point br = box.bottom_right_();
            if (box.type_() == "face")
            {
                SGTextNode *label = m_textPool.alloc();
                if (label)
                {
                    label->setColor(0x00ff00);
                    label->setPos(br.x_(), tl.y_());
                    label->setText(idLabel + L"\n" + gender + L" " + age);
                }
                SGRectangleNode *rect = m_rectPool.alloc();
                if (rect)
                {
                    rect->setSize(br.x_() - tl.x_(), br.y_() - tl.y_());
                    rect->setPos(tl.x_(), tl.y_());
                    rect->setColor(0x00ff00);
                    rect->setOpacity(0.5f);
                    rect->setRotation(0);
                    // printf("face:%f %f %f %f\n", tl.x_(), tl.y_(), br.x_(), br.y_());
                }
            } else if (box.type_() == "head")
            {
                SGRectangleNode *rect = m_rectPool.alloc();
                if (rect)
                {
                    rect->setSize(br.x_() - tl.x_(), br.y_() - tl.y_());
                    rect->setPos(tl.x_(), tl.y_());
                    rect->setColor(0xff0000);
                    rect->setOpacity(0.3f);
                    rect->setRotation(0);
                }
            } else if (box.type_() == "body")
            {
                SGRectangleNode *rect = m_rectPool.alloc();
                if (rect)
                {
                    rect->setSize(br.x_() - tl.x_(), br.y_() - tl.y_());
                    rect->setPos(tl.x_(), tl.y_());
                    rect->setColor(0x0000ff);
                    rect->setOpacity(0.2f);
                    rect->setRotation(0);
                    // printf("body:%f %f %f %f\n", tl.x_(), tl.y_(), br.x_(), br.y_());
                }
            }
        }

        ///////////////////////////////////////////////////////////
        //display skeleton
        for (int i=0;i<target.points__size();i++)
        {
            const x2::Points &points = target.points_(i);
            if (points.type_() == "landmarks")
            {
                //draw skeleton
                this->drawSkeleton(points);
            }
        }
    }
}

void SmartScene::recvCommnBlob(const SmartDataBlob * data)
{
    //non-realtime data, such as snapshot, recognization, which wont used when rendering
    //not used currently...
}

void SmartScene::startStream()
{
    //start stream worker
    m_thdStream = std::thread([this] {this->streamWorker(); });
}

bool SmartScene::isStreamStop()
{
    return m_stopStream;
}

void SmartScene::setStreamPluginPath(const char * plugin)
{
    m_pluginPath = plugin;
}

void SmartScene::streamWorker()
{
    const char* pluginpath = "lib96boardstream.so";

    if (m_pluginPath.size())
    {
        pluginpath = m_pluginPath.data();
    }

#ifdef _WIN32
    HMODULE handle = LoadLibrary(converter.from_bytes(pluginpath).data());
    if (!handle)
    {
        printf("plugin %s load error.\n", pluginpath);
    }
    CreateDeviceStream creator = (CreateDeviceStream)GetProcAddress(handle, "CreateDeviceStream");
    DestroyDeviceStream destroyer = (DestroyDeviceStream)GetProcAddress(handle, "DestroyDeviceStream");

    InitDeviceStream initor = (InitDeviceStream)GetProcAddress(handle, "InitDeviceStream");
    if (initor)
    {
        int ret = initor();
        if (ret)
        {
            printf("plugin %s initialized error.\n", pluginpath);
        }
    }
#else
    void* handle = dlopen(pluginpath, RTLD_LAZY);
    if (!handle)
    {
        printf("plugin %s load error.\n", pluginpath);
    }
    CreateDeviceStream creator = (CreateDeviceStream)dlsym(handle, "CreateDeviceStream");
    DestroyDeviceStream destroyer = (DestroyDeviceStream)dlsym(handle, "DestroyDeviceStream");
#endif // _WIN32

    if (!creator || !destroyer) return;

    InterfaceDeviceStream *reader = creator();

    reader->setReceiver(this);//set stream receiver callback
    reader->start();//start stream

    //waiting for exit signal.
    std::unique_lock<std::mutex> locker(m_mtxStop);
    m_cvStop.wait(locker, [this] { return isStreamStop(); });
    locker.unlock();

    reader->stop();
    destroyer(reader);

#ifdef _WIN32
    FreeLibrary(handle);
#else
    dlclose(handle);
#endif // _WIN32
}
