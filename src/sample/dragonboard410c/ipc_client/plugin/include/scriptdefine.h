#ifndef SCRIPTDEFINE_H
#define SCRIPTDEFINE_H

#include <string>
#include <vector>
#include <map>


struct ElementInfo
{
    enum Type { Block, Rect, Circular, Image, Text, Video };
    Type type;
};

struct BlockInfo : ElementInfo
{
    size_t size;
    char data;
};

struct DrawInfo : ElementInfo
{
    float x, y;
    float width, height;
    float rotation;

    //not worked currently.
    float opacity;
};

struct RectInfo : DrawInfo
{
    uint32_t color;
};

struct CircularInfo : DrawInfo
{
    float innerradius;
    uint32_t innercolor;
    uint32_t outercolor;
};

struct ImageInfo : DrawInfo
{
    char path[64];
};

struct TextInfo : DrawInfo
{
    uint32_t color;
    uint32_t ptsize;
    char str[256];//utf-8 encoding
};

struct VideoInfo : DrawInfo
{};

//to do.. check overflow
//SmartData parsed to user information.
struct ElementMemPool
{
    //these two alloc should be combined to one.
    template<class T>
    T* alloc()
    {
        num++;
        char* next = (char*)&ptr + offset;
        offset += sizeof(T);
        return reinterpret_cast<T*>(next);
    }
    char* alloc(size_t size)
    {
        BlockInfo *blob = alloc<BlockInfo>();
        offset += size;//with one byte more.
        blob->type = BlockInfo::Type::Block;
        blob->size = size;
        return &blob->data;
    }

    int32_t num = 0;
    int32_t offset = 0;
    ElementInfo *ptr = nullptr;
};

#endif