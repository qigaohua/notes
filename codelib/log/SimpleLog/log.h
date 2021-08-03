/*
 * This file is part of the SimpleLoger .
 *
 * Copyright (c) 2017-2099, GaoHua Qi, <qigaohua168@163.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * 'Software'), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *
 * @file log.h
 * @brief   SimpleLoger 头文件
 * @author qigaohua, qigaohua168@163.com
 * @version 01.022
 * @date 2021-08-03
 */


#ifndef __SLOG_H__
#define __SLOG_H__

#ifdef __cplusplus
extern "C" {
#endif

// color
#if defined (__WIN32__) || defined (WIN32) || defined (_MSC_VER)
#define B_RED(str)      str
#define B_GREEN(str)    str
#define B_YELLOW(str)   str
#define B_BLUE(str)     str
#define B_MAGENTA(str)  str
#define B_CYAN(str)     str
#define B_WHITE(str)    str
#define RED(str)        str
#define GREEN(str)      str
#define YELLOW(str)     str
#define BLUE(str)       str
#define MAGENTA(str)    str
#define CYAN(str)       str
#define WHITE(str)      str
#else
#define FG_BLACK        30
#define FG_RED          31
#define FG_GREEN        32
#define FG_YELLOW       33
#define FG_BLUE         34
#define FG_MAGENTA      35
#define FG_CYAN         36
#define FG_WHITE        37
#define BG_BLACK        40
#define BG_RED          41
#define BG_GREEN        42
#define BG_YELLOW       43
#define BG_BLUE         44
#define BG_MAGENTA      45
#define BG_CYAN         46
#define BG_WHITE        47
#define B_RED(str)      "\033[1;31m" str "\033[0m"
#define B_GREEN(str)    "\033[1;32m" str "\033[0m"
#define B_YELLOW(str)   "\033[1;33m" str "\033[0m"
#define B_BLUE(str)     "\033[1;34m" str "\033[0m"
#define B_MAGENTA(str)  "\033[1;35m" str "\033[0m"
#define B_CYAN(str)     "\033[1;36m" str "\033[0m"
#define B_WHITE(str)    "\033[1;37m" str "\033[0m"
#define RED(str)        "\033[31m" str "\033[0m"
#define GREEN(str)      "\033[32m" str "\033[0m"
#define YELLOW(str)     "\033[33m" str "\033[0m"
#define BLUE(str)       "\033[34m" str "\033[0m"
#define MAGENTA(str)    "\033[35m" str "\033[0m"
#define CYAN(str)       "\033[36m" str "\033[0m"
#define WHITE(str)      "\033[37m" str "\033[0m"
#endif

// 支持多线程
// #define SLOG_MULTITHREAD_ENABLE

#ifdef SLOG_MULTITHREAD_ENABLE
#include <pthread.h>
#endif


// TODO 多平台支持
#define GET_PID     getpid
#ifdef SLOG_MULTITHREAD_ENABLE
#define GET_TID     pthread_self
#else
#define GET_TID     getpid
#endif



#define OUTBUFF_SIZE                2048
#define SLOG_FILE_MAXSIZE           (20 << 20) // 20M
#define SLOG_SAVEFILE_MAXCOUNT      3

#define SLOG_FMT_DEFAULT   (SLOG_FMT_LEVEL | SLOG_FMT_TIME | SLOG_FMT_FILE | SLOG_FMT_LINE | SLOG_FMT_TAG)
#define SLOG_LEVEL_DEFAULT (SLOG_LEVEL_INFO)

#define SLOG_FMT_LEVEL_ISSET(fmt)   ((SLOG_FMT_LEVEL) & fmt)
#define SLOG_FMT_TIME_ISSET(fmt)    ((SLOG_FMT_TIME)  & fmt)
#define SLOG_FMT_TAG_ISSET(fmt)     ((SLOG_FMT_TAG)   & fmt)
#define SLOG_FMT_PID_ISSET(fmt)     ((SLOG_FMT_PID)   & fmt)
#define SLOG_FMT_TID_ISSET(fmt)     ((SLOG_FMT_TID)   & fmt)
#define SLOG_FMT_FILE_ISSET(fmt)    ((SLOG_FMT_FILE)  & fmt)
#define SLOG_FMT_FUNC_ISSET(fmt)    ((SLOG_FMT_FUNC)  & fmt)
#define SLOG_FMT_LINE_ISSET(fmt)    ((SLOG_FMT_LINE)  & fmt)


// 日志输出级别
typedef enum {
    SLOG_LEVEL_ASSERT = 0,
    SLOG_LEVEL_ERROR,
    SLOG_LEVEL_WARN,
    SLOG_LEVEL_INFO,
    SLOG_LEVEL_DEBUG
} SLogerLevel_e;


// 日志输出包括哪些信息
typedef enum {
    SLOG_FMT_LEVEL = 0x01,
    SLOG_FMT_TIME  = 0x02,
    SLOG_FMT_TAG   = 0x04,   // 标签，一般设置模块名称
    SLOG_FMT_PID   = 0x08,
    SLOG_FMT_TID   = 0x10,
    SLOG_FMT_FILE  = 0x20,
    SLOG_FMT_FUNC  = 0x40,
    SLOG_FMT_LINE  = 0x80
} SLogFmt_e;


// 模块日志结构
// typedef struct SLogerMoudle {
//     const char name[64];
//     SLogerLevel_e level;
//     struct SLogerMoudle *next;
// } SLogerMoudle_t;


typedef struct SLoger {
    // 日志级别
    SLogerLevel_e level;

    // 输出日志包括哪些信息
    int  fmt;

    /**
     * fmt: #name1[level]#name2[level]#name3[level]#
     *  #:      模块之间的分隔符
     *  name:   模块名称
     *  level:  过滤模块日志级别
     */
    char *moudle_log_filter;

    FILE *fp;
    char *logdir;
    char *logfile;
    char *save_logfile_array[SLOG_SAVEFILE_MAXCOUNT];
    int save_logfile_cnt;  // 备份的文件数量

    // SLogerMoudle_t *moudle;

#ifdef SLOG_MULTITHREAD_ENABLE
    pthread_mutex_t mutex;
#endif
} SLoger_t;



#define SLOG_OUTPUT_ENABLE
#define SLOG_LEVEL   SLOG_LEVEL_DEBUG

#if !defined(SLOG_MOUDLE_TAG)
#define SLOG_MOUDLE_TAG     ""
#endif


#ifdef SLOG_OUTPUT_ENABLE

// #if 后面不能跟宏定义，只能是常量
// #if (SLOG_LEVEL > SLOG_LEVEL_DEBUG) || (SLOG_LEVEL == SLOG_LEVEL_DEBUG)
#define slog_debug(LOG_TAG, ...)    SLogerOut(LOG_TAG, SLOG_LEVEL_DEBUG, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
// #else
// #define slog_debug(LOG_TAG, ...)
// #endif

// #if (SLOG_LEVEL > SLOG_LEVEL_INFO) || (SLOG_LEVEL == SLOG_LEVEL_INFO)
#define slog_msg(LOG_TAG, ...)      SLogerOut(LOG_TAG, SLOG_LEVEL_INFO, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
// #else
// #define slog_msg(LOG_TAG, ...)
// #endif

// #if (SLOG_LEVEL > SLOG_LEVEL_WARN) || (SLOG_LEVEL == SLOG_LEVEL_WARN)
#define slog_warn(LOG_TAG, ...)     SLogerOut(LOG_TAG, SLOG_LEVEL_WARN, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
// #else
// #define slog_warn(LOG_TAG, ...)
// #endif

// #if (SLOG_LEVEL > SLOG_LEVEL_ERROR) || (SLOG_LEVEL == SLOG_LEVEL_ERROR)
#define slog_error(LOG_TAG, ...)     SLogerOut(LOG_TAG, SLOG_LEVEL_ERROR, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
// #else
// #define slog_error(LOG_TAG, ...)
// #endif

// #if (SLOG_LEVEL > LOG_LEVEL_ASSERT) || (SLOG_LEVEL == LOG_LEVEL_ASSERT)
#define slog_assert(LOG_TAG, ...)    SLogerOut(LOG_TAG, SLOG_LEVEL_ASSERT, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
// #else
// #define slog_assert(LOG_TAG, ...)
// #endif
#else
#define slog_assert(LOG_TAG, ...)
#define slog_error(LOG_TAG, ...)
#define slog_warn(LOG_TAG, ...)
#define slog_msg(LOG_TAG, ...)
#define slog_debug(LOG_TAG, ...)
#endif



// API
#define slog_a(...)        slog_assert(SLOG_MOUDLE_TAG, __VA_ARGS__)
#define slog_e(...)        slog_error(SLOG_MOUDLE_TAG, __VA_ARGS__)
#define slog_w(...)        slog_warn(SLOG_MOUDLE_TAG, __VA_ARGS__)
#define slog_m(...)        slog_msg(SLOG_MOUDLE_TAG, __VA_ARGS__)

#define RUN_ON_DEBUG_MODE
#ifdef RUN_ON_DEBUG_MODE
#include <stdlib.h>
#define slog_d(...)        slog_debug(SLOG_MOUDLE_TAG, __VA_ARGS__)

#define str(x)      #x
#define xstr(x)     str(x)
#define BUG_ON(expr) { \
    if ((expr)) {                 \
        slog_a("BUG ON at %s:%d(%s)", __FILE__, __LINE__, __func__);    \
        slog_a("Error code : %s\r\n", xstr(expr));  \
        exit(1);                                    \
    }                                               \
}
#else
#define slog_d(...)
#define BUG_ON(expr)
#endif


int SLogerInit(int level, int logfmt, const char *logfile);
int SLogerOut(const char *tag, int level, const char *file,
        const char *func, const int line, const char *fmt,  ...);
int SLogerSetMouldeLogFilterTag(const char *tag, SLogerLevel_e level);
int SLogerOutCache(const char *errmsg);
char *SLogerGetError(void);
void SLogerDeinit();



#ifdef __cplusplus
}
#endif

#endif

