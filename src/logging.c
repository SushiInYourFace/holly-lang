#include <stdio.h>
#include <stdarg.h>

#include "logging.h"
#include "types.h"

void valueToString(Value val, char* buf, size_t size) {
    switch(val.type) {
        case(VAL_INT):
            snprintf(buf, size, "%lld", val.val_int);
            break;
        case(VAL_DOUBLE):
            snprintf(buf, size, "%.12g", val.val_float);
            break;
        case(VAL_BOOL):
            const char* bool_str = (val.val_bool) ? "true" : "false";
            snprintf(buf, size, "%s", bool_str);
            break;
        case(VAL_STRING):
            snprintf(buf, size, "%s", val.val_str);
            break;
        case(VAL_CHAR):
            snprintf(buf,size,"%c", val.val_char);
            break;
        case(VAL_INVALID):
            snprintf(buf, size, "VAL_INVALID");
            break;
        case(VAL_VOID):
            snprintf(buf, size, "void");
            break;
        case(VAL_UNSET):
            snprintf(buf, size, "unset");
            break;
    }
}

static void logMessage(LogLevel level, const char *format, va_list args) {

    if(!(LOG_LEVELS_ENABLED & level)) return;   

    const char *level_string;
    switch(level) { //log strings
        case LOG_CRITICAL:  level_string = "critical";  break;
        case LOG_ERROR:     level_string = "error";     break;
        case LOG_WARN:      level_string = "warning";   break;
        case LOG_INFO:      level_string = "info";      break;
        case LOG_VERBOSE:   level_string = "verbose";   break;
        case LOG_DEBUG:     level_string = "debug";     break;
        case LOG_TRACE:     level_string = "trace";     break;
        case LOG_TOKEN:     level_string = "token";     break;
        case LOG_NODE:      level_string = "node";      break;
        case LOG_DATA:      level_string = "data";      break;
        default:            level_string = "???";       break;
    }

    fprintf(stderr, "%s: ", level_string);
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
}

void logTrace(const char* format, ...) {
    va_list args;
    va_start(args, format);
    logMessage(LOG_TRACE, format, args);
    va_end(args);
}
void logDebug(const char* format, ...) {
    va_list args;
    va_start(args, format);
    logMessage(LOG_DEBUG, format, args);
    va_end(args);
}
void logVerbose(const char* format, ...) {
    va_list args;
    va_start(args, format);
    logMessage(LOG_VERBOSE, format, args);
    va_end(args);
}
void logInfo(const char* format, ...) {
    va_list args;
    va_start(args, format);
    logMessage(LOG_INFO, format, args);
    va_end(args);
}
void logWarn(const char* format, ...) {
    va_list args;
    va_start(args, format);
    logMessage(LOG_WARN, format, args);
    va_end(args);
}
void logToken(const char* format, ...) {
    va_list args;
    va_start(args, format);
    logMessage(LOG_TOKEN, format, args);
    va_end(args);
}
void logNode(const char* format, ...) {
    va_list args;
    va_start(args, format);
    logMessage(LOG_NODE, format, args);
    va_end(args);
}

void logData(const char* format, ...) {
    va_list args;
    va_start(args, format);
    logMessage(LOG_DATA, format, args);
    va_end(args);
}

void displayHelpText() {
    printf(
        "\n"
        "Usage: holly [options] -f <path/to/source.holly>\n"
        "Options: \n"
        "-d     Delay for two seconds before finishing execution. This is useful mainly for leak testing and debuggers\n"
    );
}

