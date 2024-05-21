#ifndef LOG_H
#define LOG_H
#include <stdio.h>
#include <stdarg.h>

#define LL_ALL 0
#define LL_INFO 1
#define LL_WARNING 2
#define LL_CRITICAL 3

#define LOG_LEVEL_ATUAL LL_INFO
#define maybe_log(level) if(level >= LOG_LEVEL_ATUAL)

void print_log(int level, const char *formatter, ...) {
    maybe_log(level) {
        va_list args;
        va_start(args, formatter);
        vprintf(formatter, args);
        va_end(args);
    }
}
#endif