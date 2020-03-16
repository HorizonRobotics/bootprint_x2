#pragma once

#include "stl_inc.h"

#include "x2.pb.h"

class X2Processer
{
public:
    X2Processer();
    ~X2Processer();

    void GenTestSmartData(const char *fname);
    int GenTestSmartData(const char *path, int num);

    char* LoadSmartData(const char *fname, int &length);
    void ParseSmart(const char *buf, int length);

private:

};
