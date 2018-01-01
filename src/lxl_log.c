/**
 * @File      lxl_log.c
 * @Function  
 * @Author    LXL ,1010361011@qq.com
 * @version   0.1
 * @Date      2017-12-21
 */
#include "lxl_log.h"


static pthread_mutex_t  lxl_log_mutex;
static uint32_t         lxl_log_level;
static char*            lxl_log_name[MAX_STRING_LEN];


#   define      LOG_LOCK        pthread_mutex_lock(&lxl_log_mutex)

#   define      LOG_UNLOCK      pthread_mutex_unlock(&lxl_log_mutex)



/**
 * @Function  lxl_open_log
 *
 * @Param     level 日志级别
 *
 * @Returns   互斥锁的初始化状态
 */
int lxl_open_log(uint32_t   level, char *filename)
{

    const char *l = "lxl_tpl.log";

    if(filename == NULL)
        memcpy(filename, l, strlen(l));
    else
        memcpy(lxl_log_name, filename, strlen(filename));


    switch (level)
    {
        case INFOMATION:
            lxl_log_level = 0x1;
            break;
        case ERROR:
            lxl_log_level = 0x3;
            break;
        case WARNING:
            lxl_log_level = 0x7;
            break;
        case DEBUG:
            lxl_log_level = 0xF;
            break;
        default:
            printf("LOG LEVEL ERROR\n");
            exit(EXIT_FAILURE);

    }


    return pthread_mutex_init(&lxl_log_mutex, NULL);

}




/**
 * @Function  lxl_close_log 
 * 销毁日志的互斥锁
 * @Returns   
 */
int lxl_close_log()
{
    return pthread_mutex_destroy(&lxl_log_mutex);

}


/**
 * @Function  lxl_log_error 
 * 日志的错误信息
 * @Param     fmt
 * @Param     ...
 */
void lxl_log_error(const char *fmt, ...)
{
    va_list     args;
    FILE *log_file = NULL;

    LOG_LOCK;
    if (NULL != (log_file = fopen((const char *)lxl_log_name, "a+")))
    {

        struct tm   tm;
        struct timeval current_time;
        gettimeofday(&current_time, NULL);
        localtime_r(&current_time.tv_sec, &tm);


        fprintf(log_file,
                "%6li:%.4d%.2d%.2d:%.2d%.2d%.2d.%04ld [ERROR]",
                pthread_self(),
                tm.tm_year + 1900,
                tm.tm_mon + 1,
                tm.tm_mday,
                tm.tm_hour,
                tm.tm_min,
                tm.tm_sec,
                current_time.tv_usec / 100
               );

        va_start(args, fmt);
        vfprintf(log_file, fmt, args);
        va_end(args);

        fprintf(log_file, "\n");

        fclose(log_file);


    }
    LOG_UNLOCK;

}




/**
 * @Function  lxl_log_info 
 * 输出日志格式
 * @Param     log_level
 * @Param     fmt
 * @Param     ...
 */
void lxl_log_info(uint32_t log_level, const char *fmt, ...)
{
    va_list     args;
    FILE *log_file = NULL;

    if((log_level & lxl_log_level))
    {
        LOG_LOCK;
        if (NULL != (log_file = fopen((const char *)lxl_log_name, "a+")))
        {

            struct tm   tm;
            struct timeval current_time;
            gettimeofday(&current_time, NULL);
            localtime_r(&current_time.tv_sec, &tm);


            fprintf(log_file,
                    "%6li:%.4d%.2d%.2d:%.2d%.2d%.2d.%04ld ",
                    pthread_self(),
                    tm.tm_year + 1900,
                    tm.tm_mon + 1,
                    tm.tm_mday,
                    tm.tm_hour,
                    tm.tm_min,
                    tm.tm_sec,
                    current_time.tv_usec / 100
                   );

            va_start(args, fmt);
            vfprintf(log_file, fmt, args);
            va_end(args);

            fprintf(log_file, "\n");

            fclose(log_file);


        }
        LOG_UNLOCK;

    }

}

