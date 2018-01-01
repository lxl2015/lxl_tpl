#ifndef LXL_THREAD_QUEUE_H
#define LXL_THREAD_QUEUE_H
#include "lxl_tpl_common.h"





lxl_thread_queue_st *lxl_thread_queue_create(char **error);
/*
 * 入队
 * */
int lxl_thread_enqueue(lxl_thread_queue_st *queue, lxl_thread_info_st *thread_info);

/*
 * 出队
 *
 * */
lxl_thread_info_st * lxl_thread_dequeue(lxl_thread_queue_st *queue);

/*
 * 销毁队列
 * */
int lxl_thread_queue_destroy( lxl_thread_queue_st *queue);

#endif
