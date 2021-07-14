/**
 * log
 *
 * Copyright (C) 2012-2013 honeysense.com
 * @author rui.sun 2012-7-3
 */
#include "ce_log.h"

#include <pthread.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <sys/time.h>

#ifdef __ANDROID__
#include <android/log.h>
#endif

/* color definitions modify from ffmpeg avutil/log.c */
#if defined(_WIN32) && !defined(__MINGW32CE__)
#include <windows.h>
#include <stdint.h>
static const uint8_t color[] = {
    7, /* debug */
    FOREGROUND_GREEN | FOREGROUND_INTENSITY, /* info */
    FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_INTENSITY, /* warn */
    FOREGROUND_RED | FOREGROUND_INTENSITY, /* error */
};

static int16_t background, attr_orig;
static HANDLE con;
#define set_color(x)  SetConsoleTextAttribute(con, background | color[x])
#define reset_color() SetConsoleTextAttribute(con, attr_orig)
#else
static const uint8_t color[]={0x03,9,0x11,0x41};
#define set_color(x)  fprintf(stderr, "\033[%d;3%dm", color[x]>>4, color[x]&15)
#define reset_color() fprintf(stderr, "\033[0m")
#endif

static int use_color = -1;
#undef fprintf
void colored_fputs(FILE * fp, int level, const char * str)
{
    if(use_color < 0) {
#if defined(_WIN32) && !defined(__MINGW32CE__)
        CONSOLE_SCREEN_BUFFER_INFO con_info;
        con = GetStdHandle(STD_ERROR_HANDLE);
        use_color = (con != INVALID_HANDLE_VALUE);
        if (use_color) {
            GetConsoleScreenBufferInfo(con, &con_info);
            attr_orig  = con_info.wAttributes;
            background = attr_orig & 0xF0;
        }
#elif defined(__APPLE__)
        use_color = 0;
#endif
    }
    
    if(use_color) {
        set_color(level);
    }
    
    fprintf(fp, str, NULL);
    
    if(use_color) {
        reset_color();
    }
}
/* color definitions modify from ffmpeg avutil/log.c */

static ce_log_level s_level = ce_log_level_none;
static pthread_mutex_t s_lock = PTHREAD_MUTEX_INITIALIZER;
static FILE * s_fp = NULL;

#ifdef WIN32
static bool s_enable_win32 =
#ifdef DEBUG
true
#else
false
#endif
#endif
;

#ifdef __ANDROID__
static char s_android_tag[64] = { "cevent" };
static bool s_enable_android =
#ifdef DEBUG
true
#else
false
#endif
#endif
;

void ce_log_open(FILE * fp)
{
    s_fp = fp;
}

void ce_log_close()
{
    if (s_fp != NULL && s_fp != stdin && s_fp != stdout && s_fp != stderr) {
        fflush(s_fp);
        fclose(s_fp);
        s_fp = NULL;
    }
}

#ifdef WIN32
void ce_log_win32(bool enable)
{
    s_enable_win32 = enable;
}
#endif

#ifdef __ANDROID__
void ce_log_android(const char * tag, bool enable)
{
    strncpy(s_android_tag, tag, sizeof(s_android_tag));
    s_enable_android = enable;
}
#endif

void ce_log_show_level(ce_log_level level)
{
    s_level = level;
}

#define MAX_LOG 4096

void ce_log_write(ce_log_level level, const char * c_file, const char * function, int line, const char * format, ...)
{
    const char * c_level;
    char logs[MAX_LOG];
    char log[MAX_LOG];
    
    if (level <= s_level || s_fp == NULL) {
#ifdef __ANDROID__
        if (!s_enable_android) {
            return;
        }
#endif
        
#ifdef WIN32
        if (!s_enable_win32) {
            return;
        }
#endif
        
        return;
    }
    
    if (format != NULL) {
        va_list args;
        
        va_start(args, format);
        vsnprintf(log, sizeof(log) - 1, format, args);
        va_end(args);
    } else {
        strcpy(log, "");
    }
    
    {
        const char * suffix;
        
        suffix = strrchr(c_file, '/');
        if (suffix == NULL) suffix = strrchr(c_file, '\\');
        if (suffix != NULL) suffix += 1;
        if (suffix == NULL) suffix = c_file;
        c_file = suffix;
    }
    
    switch (level) {
        case ce_log_level_debug:
            c_level = "DEBUG";
            break;
        case ce_log_level_info:
            c_level = "INFO";
            break;
        case ce_log_level_warn:
            c_level = "WARN";
            break;
        case ce_log_level_error:
        default:
            c_level = "ERROR";
            break;
    }
    
    {
        struct timeval tv;
        struct tm * tm;
        
        gettimeofday(&tv, NULL);
        tm = localtime(&tv.tv_sec);
        
        snprintf(logs, sizeof(logs) - 1, "%04d-%02d-%02d %02d:%02d:%02d.%03d %-5s - %s [%s/%s(%d)]\r\n",
                 tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
                 tm->tm_hour, tm->tm_min, tm->tm_sec, (int)(tv.tv_usec / 1000),
                 c_level, log, c_file, function, line);
    }
    
    pthread_mutex_lock(&s_lock);
    
    if (s_fp != NULL && (s_fp == stdout || s_fp == stderr || s_fp == stdin)) {
        colored_fputs(s_fp, level, logs);
    }
    
#ifdef WIN32
    if (s_enable_win32) {
        OutputDebugStringA(logs);
    }
#endif
    
#ifdef __ANDROID__
    if (s_enable_android) {
        switch (level) {
            case ce_log_level_debug:
                __android_log_print(ANDROID_LOG_DEBUG, s_android_tag, logs, NULL);
                break;
            case ce_log_level_info:
                __android_log_print(ANDROID_LOG_INFO, s_android_tag, logs, NULL);
                break;
            case ce_log_level_warn:
                __android_log_print(ANDROID_LOG_WARN, s_android_tag, logs, NULL);
                break;
            case ce_log_level_error:
                __android_log_print(ANDROID_LOG_ERROR, s_android_tag, logs, NULL);
                break;
            default:
                break;
        }
    }
#endif
    
    pthread_mutex_unlock(&s_lock);
}