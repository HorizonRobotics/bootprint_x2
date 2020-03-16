#ifndef INTERFACE_H
#define INTERFACE_H


#ifdef BUILDING_PLUGIN
    #ifdef _MSC_VER
        #define INTER_LIB_API __declspec(dllexport)
    #else
        #define INTER_LIB_API __attribute__((visibility("default")))
    #endif

    #define PLUGIN_DEFINE(name, interface) \
        extern "C" INTER_LIB_API int Init##name(); \
        extern "C" INTER_LIB_API void Close##name(); \
        extern "C" INTER_LIB_API interface* Create##name(); \
        extern "C" INTER_LIB_API void Destroy##name(interface *p);
#else
    #define PLUGIN_DEFINE(name, interface) \
        typedef int (*Init##name)();\
        typedef void (*Close##name)();\
        typedef interface *(*Create##name)();\
        typedef void (*Destroy##name)(interface *);
#endif


#endif
