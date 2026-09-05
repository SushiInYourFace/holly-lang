#pragma once

#include "types.h"

//how large a value string buffer should be
#define VAL_STR_LEN (64)

typedef enum {
    LOG_CRITICAL =  1 << 0,
    LOG_ERROR =     1 << 1,
    LOG_WARN =      1 << 2,
    LOG_INFO =      1 << 3,
    LOG_VERBOSE =   1 << 4,
    LOG_DEBUG =     1 << 5,
    LOG_TRACE =     1 << 6,
    //specific categories
    LOG_TOKEN =     1 << 7,
    LOG_NODE =      1 << 8,
    LOG_DATA =      1 << 9,
    LOG_EVAL =      1 << 10       
} LogLevel;

#define LOG_LEVELS_ENABLED (~0u)


//funs

//convert a value to a useable string
void valueToString(Value val, char* buf, size_t size);

//log at the debug level
void logTrace(const char* format, ...);
void logDebug(const char* format, ...);
void logVerbose(const char* format, ...);
void logInfo(const char* format, ...);
void logWarn(const char* format, ...);
void logToken(const char* format, ...);
void logNode(const char* format, ...);
void logData(const char* format, ...);
void logEval(const char* format, ...);
//help text
void displayHelpText();