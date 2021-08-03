/*
 * This file is part of the SampleLoger .
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
 * @file log.c
 * @brief   SampleLoger 初始化和其他api实现c文件
 * @author qigaohua, qigaohua168@163.com
 * @version 01.022
 * @date 2021-08-03
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>
#include <time.h>
#include "log.h"


#define SLoger_e(fmt, ...) {    \
    fprintf(stderr,  "%s [%s:%d] "fmt"\r\n", B_RED("E#"), __FILE__, __LINE__, ##__VA_ARGS__);   \
    syslog(LOG_ERR, "E# [%s:%d] "fmt"\e\n", __FILE__, __LINE__, ##__VA_ARGS__);   \
}

static SLoger_t *sLoger;
static char slog_cache_buf[OUTBUFF_SIZE];


struct {
    const char *str;
    const char *color;
} slog_level_str[] = {
    {"A#", B_BLUE("%s ")},
    {"E#", B_RED("%s ")},
    {"W#", B_YELLOW("%s ")},
    {"M#", B_GREEN("%s ")},
    {"D#", B_WHITE("%s ")},
} ;



static inline void SLogerGetCurTimestr(char *timestr, size_t len)
{
    struct tm tm;
    time_t secs = time(NULL);
    localtime_r(&secs, &tm); // len > 20
    strftime(timestr, len, "%Y-%m-%d %H:%M:%S", &tm);
}



static int SLogerGetFileSize(FILE *fp)
{
    int size, curoffset;

    if (!fp || fp == stdout)
        return -1;

    curoffset = ftell(fp);
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    fseek(fp, curoffset, SEEK_SET);

    return size;
}



static void SLogerCleanLogfile(int retain)
{
    // if (sLoger->save_logfile_cnt < SLOG_SAVEFILE_MAXCOUNT)
    //     return;

    // 当备份的日志文件太多时，删除较早的日志文件
    int i = 0, clean_cnt;

    if (retain) clean_cnt = sLoger->save_logfile_cnt - 1;
    else clean_cnt = sLoger->save_logfile_cnt;

    for (; i < clean_cnt; i++) {
        if (-1 == remove(sLoger->save_logfile_array[i])) {
            SLoger_e("Failed to remove `%s': %s\r\n",
                    sLoger->save_logfile_array[i], strerror(errno));
        }
        free(sLoger->save_logfile_array[i]);
    }
    if (retain) {
        sLoger->save_logfile_array[0] = sLoger->save_logfile_array[sLoger->save_logfile_cnt-1];
        sLoger->save_logfile_array[sLoger->save_logfile_cnt-1] = NULL;
        sLoger->save_logfile_cnt = 1;
        return;
    }
    sLoger->save_logfile_cnt = 0;
}


static void SLogerAddLogFile(const char *newfile)
{
    if (sLoger->save_logfile_cnt >= SLOG_SAVEFILE_MAXCOUNT)
        SLogerCleanLogfile(1);

    sLoger->save_logfile_array[sLoger->save_logfile_cnt++] = strdup(newfile);
}


/**
 * @brief save_logfile 日志文件过大时，保存文件重命名，重新打开新的日志文件
 *
 * @param fp 日志文件描述符
 *
 * @return 新的日志文件描述符
 */
static int SLogerSaveLogFile(FILE *fp)
{
    char new_filename[512] = {0};
    char datefmt[32] = {0};
    struct tm tm;
    time_t secs;

    if (!fp || fp == stdout) return 0;

    fflush(fp);
    if (fclose(fp) == EOF) {
        return -1;
    }
    sLoger->fp = NULL;

    secs = time(NULL);
    localtime_r(&secs, &tm);
    // 日志量不大，去掉后面-%H:%M:%S
    strftime(datefmt, sizeof datefmt, "%Y%m%d-%H:%M:%S", &tm);

    snprintf(new_filename, sizeof new_filename, "%s.%s", sLoger->logfile, datefmt);
    if (-1 == rename(sLoger->logfile, new_filename)) {
        SLoger_e("Failed to rename '%s' to '%s': %s\n", sLoger->logfile,
                new_filename, strerror(errno));
        // 重命名失败，则过去日志没了
        return -1;
    }
    SLogerAddLogFile(new_filename);

    return 0;
}



/**
 * @brief SLogerInit 初始化接口
 *
 * @param level     日志输出级别, 等于0为默认级别
 * @param logfmt    日志输出包括的内容，等于0设置为默认值
 * @param logfile   日志保存的文件，等于NULL输出日志到标准输出
 *
 * @return  0: 成功  -1:失败
 */
int SLogerInit(int level, int logfmt, const char *logfile)
{
    if (sLoger) {
        SLoger_e( "Failed to slog init, slog can only be initilized once !");
        return -1;
    }

    sLoger = malloc(sizeof(SLoger_t));
    if (!sLoger) {
        SLoger_e( "Failed to allocate (a very small) memory. exit!");
        exit(EXIT_FAILURE);
    }

    if (level > 0) {
        sLoger->level = level;
    }
    else if (level == 0) {
        sLoger->level = SLOG_LEVEL_DEFAULT;
    }
    else {
        SLoger_e( "Failed to set log level, invalied param: %d", level);
        exit(EXIT_FAILURE);
    }

    if (!logfile) {
        sLoger->logdir = NULL;
        sLoger->logfile = NULL;
        sLoger->fp = stdout;
    }
    else {
        char *p = NULL;
        if (logfile[0] == '/' && ((p = strrchr(logfile, '/')) != logfile)) {    // absolute path
            sLoger->logdir = strndup(logfile, p - logfile + 1);
        }
        else if ((logfile[0] == '.' && logfile[1] == '/' && !strchr(logfile + 3, '/'))
                || (logfile[0] != '.' && !strchr(logfile + 2, '/'))) {
            sLoger->logdir = strndup("./", 2);
        }
        else {
            SLoger_e( "Failed to slog init, param `logfile` should be an absolute path "
                    "or current path. exit !\r\n");
            exit(EXIT_FAILURE);
        }

        sLoger->logfile = strdup(logfile);

        if (access(sLoger->logdir, F_OK | W_OK) != 0) {
            SLoger_e( "%s: log path not exists. exit !\r\n", sLoger->logdir);
            exit(EXIT_FAILURE);
        }

        sLoger->fp = fopen(logfile, "w+");
        if (!sLoger->fp) {
            SLoger_e( "Failed to open a file(%s): %m", sLoger->logfile);
            exit(EXIT_FAILURE);
        }
    }

    if (logfmt == 0) {
        sLoger->fmt = SLOG_FMT_DEFAULT;
    }
    else if (logfmt > 0) {
        if (SLOG_FMT_FUNC_ISSET(logfmt) || SLOG_FMT_LINE_ISSET(logfmt))
            logfmt |= SLOG_FMT_FILE;
        if (SLOG_FMT_TID_ISSET(logfmt)) logfmt |= SLOG_FMT_PID;
        sLoger->fmt = logfmt;
    }
    else {
        SLoger_e( "Failed to set log format, invalied param: %d", logfmt);
        exit(EXIT_FAILURE);
    }

    sLoger->moudle_log_filter = NULL;

#ifdef SLOG_MULTITHREAD_ENABLE
    pthread_mutex_init(&sLoger->mutex, NULL);
#endif

    return 0;
}



/**
 * @brief SLogerDeinit 日志销毁接口
 */
void SLogerDeinit()
{
    if (!sLoger) return;
    if (sLoger->moudle_log_filter) free(sLoger->moudle_log_filter);
    if (sLoger->logfile) free(sLoger->logfile);
    if (sLoger->logdir) free(sLoger->logdir);
    SLogerCleanLogfile(0);

#ifdef SLOG_MULTITHREAD_ENABLE
    pthread_mutex_destroy(&sLoger->mutex);
#endif
}



/**
 * @brief SLogerSetMouldeLogFilterTag 设置模块日志的过滤接口
 *
 * @param tag       日志模块的名称
 * @param level     模块日志过滤到哪个级别
 *
 * @return          0: 成功   -1： 失败
 */
int SLogerSetMouldeLogFilterTag(const char *tag, SLogerLevel_e level)
{
#define SLOG_MOUDLE_LOG_FILTER_SIZE     512
    if (!sLoger) {
        SLoger_e( "SampleLoger must be init, you should be use slog_init()");
        return -1;
    }

    if (!tag || level < 0 || level > SLOG_LEVEL_DEBUG) {
        SLoger_e( "Failed to set module log filter, invalied param");
        return -1;
    }

#ifdef SLOG_MULTITHREAD_ENABLE
    pthread_mutex_lock(&sLoger->mutex);
#endif

    if (!sLoger->moudle_log_filter) {
        if ((sLoger->moudle_log_filter = malloc(SLOG_MOUDLE_LOG_FILTER_SIZE)) == NULL) {
            SLoger_e("Failed to allocate (a small) memory: %m");
            return -1;
        }
        memset(sLoger->moudle_log_filter, '\0', SLOG_MOUDLE_LOG_FILTER_SIZE);
        sLoger->moudle_log_filter[0] = '#';
    }

    // fmt: #tag1[level]#tag2#tag3#tagn#
    int len = strlen(sLoger->moudle_log_filter);
    strncpy(sLoger->moudle_log_filter + len, tag, SLOG_MOUDLE_LOG_FILTER_SIZE - len - 1);
    len = strlen(sLoger->moudle_log_filter);
    snprintf(sLoger->moudle_log_filter + len, SLOG_MOUDLE_LOG_FILTER_SIZE - len - 1,
            "[%d]#", level);
    // printf(">>>>>>%s\n", sLoger->moudle_log_filter);

#ifdef SLOG_MULTITHREAD_ENABLE
    pthread_mutex_unlock(&sLoger->mutex);
#endif

    return 0;
}



/**
 * @brief SLogerOut 输出日志
 *
 * @param tag       模块日志的模块名称
 * @param level     日志的级别
 * @param file      输出日志所在行
 * @param func      输出日志所在函数
 * @param line      输出日志所在文件
 * @param fmt       输出格式
 * @param ...       参数
 *
 * @return          0: 成功   -1: 失败
 */
int SLogerOut(const char *tag, int level, const char *file,
        const char *func, const int line, const char *fmt,  ...)
{
    if (!sLoger) {
        SLoger_e( "SampleLoger must be init, you should be use slog_init()");
        return -1;
    }
    if (sLoger->level < level) return 0;

    char *p;
    if (sLoger->moudle_log_filter && (p = strstr(sLoger->moudle_log_filter, tag)) != NULL) {
        // #name[3]#
        int len = strlen(tag), moudle_level = -1;
        if (*(p-1) == '#' && *(p+len) == '[' && *(p+len+3) == '#') {
            p = p + len + 1;
            moudle_level = atoi(p);
            if (level > moudle_level) return 0;
        }
    }


    int outlen = 0;
    char outbuf[OUTBUFF_SIZE+1] = {'\0'};
    char timestr[32] = {'\0'};
    pid_t pid;
    pthread_t tid;
    va_list ap;

    SLogerGetCurTimestr(timestr, sizeof timestr);
    pid = GET_PID();
    tid = GET_TID();

    if (SLOG_FMT_LEVEL_ISSET(sLoger->fmt)) {
            outlen += snprintf(outbuf + outlen, OUTBUFF_SIZE - outlen,
                    sLoger->fp == stdout ? slog_level_str[level].color : "%s ",
                    slog_level_str[level].str);

    }

    if (SLOG_FMT_TIME_ISSET(sLoger->fmt)) {
        outlen += snprintf(outbuf + outlen, OUTBUFF_SIZE - outlen,
                "%s ", timestr);
    }

    if (SLOG_FMT_TAG_ISSET(sLoger->fmt)) {
        outlen += snprintf(outbuf + outlen, OUTBUFF_SIZE - outlen,
                "%s ", tag);
    }

    if (SLOG_FMT_PID_ISSET(sLoger->fmt) || SLOG_FMT_TID_ISSET(sLoger->fmt)) {
        outlen += snprintf(outbuf + outlen, OUTBUFF_SIZE - outlen, "%s", "[");
        if (SLOG_FMT_PID_ISSET(sLoger->fmt))
            outlen += snprintf(outbuf + outlen, OUTBUFF_SIZE - outlen, "%d", pid);
        if (SLOG_FMT_TID_ISSET(sLoger->fmt))
            outlen += snprintf(outbuf + outlen, OUTBUFF_SIZE - outlen, ":%d", tid);
        outlen += snprintf(outbuf + outlen, OUTBUFF_SIZE - outlen, "%s", "] ");
    }

    if (SLOG_FMT_FILE_ISSET(sLoger->fmt) || SLOG_FMT_FUNC_ISSET(sLoger->fmt)
            || SLOG_FMT_LINE_ISSET(sLoger->fmt)) {
        outlen += snprintf(outbuf + outlen, OUTBUFF_SIZE - outlen, "%s", "[");
        if (SLOG_FMT_FILE_ISSET(sLoger->fmt))
            outlen += snprintf(outbuf + outlen, OUTBUFF_SIZE - outlen, "%s", file);
        if (SLOG_FMT_FUNC_ISSET(sLoger->fmt))
            outlen += snprintf(outbuf + outlen, OUTBUFF_SIZE - outlen, ":%s", func);
        if (SLOG_FMT_LINE_ISSET(sLoger->fmt))
            outlen += snprintf(outbuf + outlen, OUTBUFF_SIZE - outlen, ":%d", line);
        outlen += snprintf(outbuf + outlen, OUTBUFF_SIZE - outlen, "%s", "] ");
    }

    va_start(ap, fmt);
    outlen += vsnprintf(outbuf + outlen, OUTBUFF_SIZE - outlen, fmt, ap);
    va_end(ap);
    outbuf[outlen] = '\0';

#ifdef SLOG_MULTITHREAD_ENABLE
    pthread_mutex_lock(&sLoger->mutex);
#endif

    int filesize = SLogerGetFileSize(sLoger->fp);
    if (filesize >= SLOG_FILE_MAXSIZE) {
        SLogerSaveLogFile(sLoger->fp);
        if (!sLoger->fp) {
            // 重新写
            sLoger->fp = fopen(sLoger->logfile, "w+");
            if (!sLoger->fp) {
                // 不大可能
                SLoger_e( "Failed to open a  file(%s): %s\n", sLoger->logfile,
                        strerror(errno));
                sLoger->fp = stdout;
            }
        }
    }

    fprintf(sLoger->fp, "%s\r\n", outbuf);
#ifdef __USE_SYSLOG__
    syslog(LOG_ERR, "%s\n", outbuf);
#endif

#ifdef SLOG_MULTITHREAD_ENABLE
    pthread_mutex_unlock(&sLoger->mutex);
#endif

    return 0;
}



/**
 * @brief SLogerOutCache 输出日志到缓存
 *
 * @param errmsg         错误的信息
 *
 * @return              0: 成功  -1: 失败
 */
int SLogerOutCache(const char *errmsg)
{
    if (!errmsg) return -1;
    memset(slog_cache_buf, '\0', OUTBUFF_SIZE);
    strncpy(slog_cache_buf, errmsg, OUTBUFF_SIZE);
    return 0;
}



/**
 * @brief SLogerGetError 取出缓存信息
 *
 * @return  缓存信息
 */
char *SLogerGetError(void)
{
    return slog_cache_buf;
}



