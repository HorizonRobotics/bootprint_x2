/*
* Brief: Common definition
* Author: GYW
* LOG:
*   - Created 20190926
*/

#pragma once

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <exception>
#include <iostream>
#include <functional>
#include <list>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <time.h>
#include <unordered_map>
#include <vector>
#include <fstream>


enum ErrorCode
{
    EC_OK,
    EC_SYSTEM_ERROR,
    EC_BAD_PARAMETER,
    EC_BAD_CONFIG,
    EC_INVALID_DATA,
    EC_FILE_NOT_EXIST,
    EC_FILE_ERROR,
    EC_HBIPC_ERROR,
    EC_NO_CACHE,
    EC_NO_DATA,
    EC_TIMEOUT,
    EC_NETWORK_ERROR,
    EC_UNKNOWN
};
typedef struct VIDEO_FRAME
{
    char *data;
    int size;
    std::string format;
    int frame_id;
}VIDEO_FRAME, *pVIDEO_FRAME;

typedef struct SMART_FRAME
{
    char *data;
    int size;
    int index;
    // @type
    // 0: unknow smart frame
    // 1: normal smart frame
    // 2: drop smart frame
    // 3: error smart frame
    int frame_type;
}SMART_FRAME, *pSMART_FRAME;

void deleteVideoFrame(VIDEO_FRAME *videoFrame);

void deleteSmartFrame(SMART_FRAME *smartFrame);

