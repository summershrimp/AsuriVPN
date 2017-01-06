//
// Created by ubuntu on 1/5/17.
//

#include <stdio.h>
#include <time.h>
#include <string.h>

#include "logger.h"
#include <stdarg.h>

FILE *flog = NULL
;
enum _log_level log_level = LOG_DEBUG;

void log_set_level(enum _log_level level) {
    log_level = level;
}

void log_format(const char* tag, const char* message, va_list args) {
    if(flog == NULL){
        flog = stderr;
    }
    time_t now;
    time(&now);
    char* date = ctime(&now);
    date[strlen(date) - 1] = 0;
    fprintf(flog,"%s [%s] ", date, tag);
    vfprintf(flog, message, args);
    fprintf(flog, "\n");
}

void log_error(const char* message, ...){
    if(log_level > LOG_ERROR){
        return;
    }
    va_list args;
    va_start(args, message);
    log_format("error", message, args);
    va_end(args);
}
void log_warn(const char* message, ...){
    if(log_level > LOG_WARN){
        return;
    }
    va_list args;
    va_start(args, message);
    log_format("warn", message, args);
    va_end(args);
}
void log_debug(const char* message, ...){
    if(log_level > LOG_DEBUG){
        return;
    }
    va_list args;
    va_start(args, message);
    log_format("debug", message, args);
    va_end(args);
}
void log_info(const char* message, ...){
    if(log_level > LOG_INFO){
        return;
    }
    va_list args;
    va_start(args, message);
    log_format("info", message, args);
    va_end(args);
}