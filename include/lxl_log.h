/**
 * @File      lxl_log.h
 * @Function  
 * @Author    LXL ,1010361011@qq.com
 * @version   0.1
 * @Date      2017-12-27
 */
#ifndef LXL_LOG_H
#define LXL_LOG_H

#include "lxl_common.h"
#include <sys/time.h>

#define LEVEL_LOG_INFOMATION      0x1
#define LEVEL_LOG_WARNING         0x2
#define LEVEL_LOG_ERROR           0x4
#define LEVEL_LOG_DEBUG           0x8

typedef enum
{
    INFOMATION = 0,
    ERROR,
    WARNING,
    DEBUG
}LOG_LEVEL;



int lxl_open_log(uint32_t log_level, char *filename);
int lxl_close_log();
void lxl_log_error(const char *fmt, ...);
void lxl_log_info(uint32_t log_level, const char *fmt, ...);


#endif
