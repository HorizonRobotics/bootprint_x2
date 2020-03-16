#include "common.h"

const char* GetErrorStr(ErrorCode ecode)
{
    switch (ecode)
    {
    case EC_OK:
        return "Success";
    case EC_SYSTEM_ERROR:
        return "System error";
    case EC_BAD_PARAMETER:
        return "Bad parameter";
    case EC_BAD_CONFIG:
        return "Bad config";
    case EC_INVALID_DATA:
        return "Invalid message";
    case EC_FILE_NOT_EXIST:
        return "File not exist";
    case EC_FILE_ERROR:
        return "File error";
    case EC_HBIPC_ERROR:
        return "Hbipc error";
    case EC_NO_CACHE:
        return "No cache";
    case EC_NO_DATA:
        return "No data";
    case EC_TIMEOUT:
        return "Time out";
    case EC_NETWORK_ERROR:
        return "Network error";
    case EC_UNKNOWN:
    default:
        break;
    }
    return "Unknown error";
}

void deleteVideoFrame(VIDEO_FRAME *videoFrame)
{
    if(videoFrame != nullptr)
    {
        if(videoFrame->data != nullptr)
        {
            delete[] videoFrame->data;
            videoFrame->data = nullptr;
        }
        delete videoFrame;
        videoFrame = nullptr;
    }
}

void deleteSmartFrame(SMART_FRAME *smartFrame)
{
    if(smartFrame != nullptr)
    {
        if(smartFrame->data != nullptr)
        {
            delete[] smartFrame->data;
            smartFrame->data = nullptr;
        }
        delete smartFrame;
        smartFrame = nullptr;
    }
}
