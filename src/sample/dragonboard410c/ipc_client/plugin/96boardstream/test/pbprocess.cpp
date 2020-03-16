#include "pbprocess.h"


X2Processer::X2Processer()
{
}

X2Processer::~X2Processer()
{
}

void X2Processer::GenTestSmartData(const char *fname)
{
    x2::SmartFrameMessage smart;
    smart.set_timestamp_(0);
    smart.set_error_code_(0);
    x2::Target *target = smart.add_targets_();
    if (target == nullptr)
        return;
    target->set_type_("person");
    target->set_track_id_(0);
    x2::Box *box = target->add_boxes_();
    if (box == nullptr)
        return;
    x2::Point *p0 = box->mutable_top_left_();
    x2::Point *p1 = box->mutable_bottom_right_();
    if (!p0 || !p1)
        return;
    p0->set_x_(200);
    p0->set_y_(200);
    p1->set_x_(400);
    p1->set_y_(400);

    //output to file
    int length = smart.ByteSize() + 1;
    char *buf = new char[length];
    if (!buf)
        return;
    buf[length - 1] = 0;
    bool bret = smart.SerializeToArray(buf, length);
    if (!bret)
        std::cout << "generate smart message failed" << std::endl;
    else
    {
        FILE *f = fopen(fname, "wb");
        if (f)
        {
            //fprintf(f, "%s", buf);//二进制输出存在问题
            fwrite(buf, length, 1, f);
            fclose(f);

            std::cout << "output smart message ok" << std::endl;
        }
    }
    delete[] buf;
    return;
}

int X2Processer::GenTestSmartData(const char *path, int num)
{
#if WIN32
    int ret = _access(path, 0); // check the dir
    if (ret == -1)
    {
        _mkdir(path);
    }
#else
    int ret = access(path, 0); // check the dir
    if (ret == -1)
    {
        mkdir(path, S_IRWXU);
    }
#endif

    x2::SmartFrameMessage smart;
    smart.set_timestamp_(0);
    smart.set_error_code_(0);
    x2::Target *target = smart.add_targets_();
    if (target == nullptr)
        return 0;

    target->set_type_("person");
    target->set_track_id_(0);
    x2::Box *box = target->add_boxes_();
    if (box == nullptr)
        return 0;
    x2::Point *p0 = box->mutable_top_left_();
    x2::Point *p1 = box->mutable_bottom_right_();
    if (!p0 || !p1)
        return 0;
    p0->set_x_(1);//赋值0无法正确初始化 smart
    p0->set_y_(1);
    p1->set_x_(1);
    p1->set_y_(1);
    int length = smart.ByteSize()/* + 1*/;//加一个字节会导致解析失败
    char *buf = new char[length];
    if (!buf)
        return 0;

    std::string fname;
    int w = 1920, margin = 50;
    int xlen = 200, ylen = 200;
    int xpos = margin, ypos = 200;
    int xstep = (w - 2 * margin - 200) / num;
    int num_gen = 0, num_out = 0;
    for (int i = 0; i < num; ++i)
    {
        fname = path;
        fname.append("/");
        fname.append(std::to_string(i));

        p0->set_x_(xpos);
        p0->set_y_(ypos);
        p1->set_x_(xpos + xlen);
        p1->set_y_(ypos + ylen);

        xpos += xstep;

        //output to file
        memset(buf, 0, length);
        bool bret = smart.SerializeToArray(buf, length);
        if (!bret)
            std::cout << " generate smart message failed" << std::endl;
        else
        {
            num_gen++;
            FILE *f = fopen(fname.c_str(), "wb");
            if (f)
            {
                num_out++;
                //fprintf(f, "%s", buf);
                fwrite(buf, length, 1, f);
                fclose(f);
            }
        }
    }
    delete[] buf;
    std::cout << "generate smart message " << num_gen << std::endl;
    std::cout << "output smart message " << num_out << std::endl;
    return num_out;
}

char * X2Processer::LoadSmartData(const char * fname, int & length)
{
    std::fstream fin(fname, std::ios::in | std::ios::binary);
    if (!fin.is_open())
    {
        cout << "open file failed." << endl;
        return nullptr;
    }

    fin.seekg(0, ios::end);
    length = fin.tellg();
    fin.seekg(0, ios::beg);

    char *buf = new char[length];
    fin.read(buf, length);
    if (!fin.good())
    {
        cout << "read failed failed" << endl;
        delete[] buf;
        buf = nullptr;
    }
    fin.close();
    cout << "Load smart file success." << endl;
    return buf;
}

void X2Processer::ParseSmart(const char * buf, int length)
{
    if (!buf)
        return;
    x2::SmartFrameMessage smart;
    bool bret = smart.ParseFromArray(buf, length);
    //bool bret = smart.ParsePartialFromArray(buf, length);
    if (!bret)
    {
        cout << "Parse smart message failed" << endl;
        return;
    }
    cout << "Parse smart message success." << endl;
    cout << "timestamp " << smart.timestamp_() << endl;
    cout << "error_code " << smart.error_code_() << endl;
    if (smart.targets__size() == 0)
    {
        cout << "no target in smart" << endl;
        return;
    }
    const x2::Target &target = smart.targets_(0);
    cout << " target 0, data:" << endl;
    cout << " type " << target.type_() << endl;
    cout << " track_id " << target.track_id_() << endl;
    if (target.boxes__size() == 0)
    {
        cout << " no box in target" << endl;
        return;
    }
    const x2::Box &box = target.boxes_(0);
    cout << " box 0, data:" << endl;
    cout << "  top_left(" << box.top_left_().x_() << ", " << box.top_left_().y_() << ")" << endl;
    cout << "  top_left(" << box.bottom_right_().x_() << ", " << box.bottom_right_().y_() << ")" << endl;

    return;
}
