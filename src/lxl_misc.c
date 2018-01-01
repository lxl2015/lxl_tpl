/**
 * @File      lxl_misc.c
 * @Function  
 * @Author    LXL ,1010361011@qq.com
 * @version   0.1
 * @Date      2017-12-27
 */
#include "lxl_misc.h"

/**
 * @Function  lxl_strdup 
 * 封装strdup
 * @Param     fmt
 * @Param     ...
 *
 * @Returns   
 */
char *lxl_strdup(const char *fmt, ...)
{
    va_list args;
    char buffer[MAX_STRING_LEN] = {0};

    va_start(args, fmt);
    vsprintf(buffer, fmt,  args);
    va_end(args);

    return strdup(buffer);
}
