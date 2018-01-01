/**
 * @File      lxl_common.h
 * @Function  
 * @Author    LXL ,1010361011@qq.com
 * @version   0.1
 * @Date      2017-12-27
 */

#ifndef  LXL_COMMON_H
#define LXL_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdint.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <sys/prctl.h>




#define SUCCESS             0
#define FAIL                -1
#define MAX_STRING_LEN      2048
#define MAX_BUFFER_LEN      65536

#ifndef BOOL
#define BOOL    uint8_t
#endif


#ifndef TRUE
#define TRUE    1
#endif

#ifndef FALSE
#define FALSE   0
#endif


#ifndef YES
#define YES     1
#endif

#ifndef NO
#define NO      0
#endif



#endif
