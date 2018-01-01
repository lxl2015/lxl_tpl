/**
 * @File      lxl_tpl.h
 * @Function  
 * @Author    LXL ,1010361011@qq.com
 * @version   0.1
 * @Date      2017-12-27
 */
#ifndef LXL_TPL_H
#define LXL_TPL_H

#include "lxl_tpl_common.h"




lxl_tpl_st  *lxl_tpl_create(int min, int max, char **error);
int lxl_tpl_init(lxl_tpl_st *tpl, char **error);
int tpl_process_job(lxl_tpl_st *tpl, process_job proc_fun, void *arg, char **error) ;
void lxl_tpl_destroy(lxl_tpl_st *tpl);
void lxl_thread_resume(void);
void lxl_tpl_pause(lxl_thread_queue_st *queue);
#endif
