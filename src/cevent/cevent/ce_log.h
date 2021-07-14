/**
 * log
 *
 * Copyright (C) 2012-2013 honeysense.com
 * @author rui.sun 2012-7-3
 */
#ifndef CE_LOG_H
#define CE_LOG_H

#include <stdio.h>

#define CE_LOGD(...) ce_log_write(ce_log_level_debug, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#define CE_LOGI(...) ce_log_write(ce_log_level_info, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#define CE_LOGW(...) ce_log_write(ce_log_level_warn, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#define CE_LOGE(...) ce_log_write(ce_log_level_none, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)

typedef enum ce_log_level {
    ce_log_level_none,
    ce_log_level_error,
    ce_log_level_warn,
    ce_log_level_info,
    ce_log_level_debug,
} ce_log_level;

void ce_log_open(FILE * fp);
void ce_log_close();

#ifdef __ANDROID__
void ce_log_android(const char * tag, bool enable);
#endif

#ifdef WIN32
void ce_log_win32(bool enable);
#endif

void ce_log_show_level(ce_log_level level);
void ce_log_write(ce_log_level level, const char * c_file, const char * function, int line, const char * format, ...);

#endif