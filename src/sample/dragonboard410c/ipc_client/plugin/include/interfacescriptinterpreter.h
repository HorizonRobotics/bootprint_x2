#ifndef INTERFACE_SCRIPT_INTERPRETER_H
#define INTERFACE_SCRIPT_INTERPRETER_H

#include <cstddef>
#include "interface.hpp"

class InterfaceScriptInterpreter
{
public:
    virtual const char* type() = 0;
    virtual bool create(const char* script, const char* home = nullptr, const char* engine = nullptr) = 0;
    
    virtual bool initialize() = 0;
    virtual bool cleanup() = 0;
    virtual bool decodeSmartVideo(const char *str, size_t len, void *dst) = 0;
    virtual bool decodeStatistics(const char *str, size_t len, void *dst) = 0;
};
PLUGIN_DEFINE(ScriptInterpreter, InterfaceScriptInterpreter)

#endif
