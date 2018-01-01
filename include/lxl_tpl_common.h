/**
 * @File      lxl_tpl_common.h
 * @Function  
 * @Author    LXL ,1010361011@qq.com
 * @version   0.1
 * @Date      2017-12-27
 */
#ifndef LXL_TPL_COMMON_H
#define LXL_TPL_COMMON_H
#include "lxl_common.h"


typedef void (*process_job)(void *arg);
typedef struct __lxl_thread_info__      lxl_thread_info_st;
typedef struct __lxl_tpl__              lxl_tpl_st;


typedef struct __queue_item__
{
    lxl_thread_info_st      *thread_info;
    struct __queue_item__   *next;
}lxl_thread_item_st;


typedef struct __lxl_thread__queue__
{
    lxl_thread_item_st  *node;
    lxl_thread_item_st  *head;
    lxl_thread_item_st  *tail;
    lxl_thread_item_st  *idle_queue_item;
    pthread_mutex_t     idle_queue_item_mutex;
    pthread_mutex_t     queue_mutex;
    uint32_t            count;
}lxl_thread_queue_st;


/*
 *  * 线程信息
 *   * */
struct __lxl_thread_info__
{
    pthread_t           thread_id;              //线程的ID
    int                 thread_num;             //第几个线程
    pthread_mutex_t     mutex;
    pthread_cond_t      cond;
    BOOL                is_busy;                //线程是否忙碌
    process_job         proc_func;              //回掉函数入口地址
    void                *arg;                   //回掉函数的参数
    lxl_tpl_st          *thread_pool;           //线程池
};



/*
 *  * 线程池的信息
 *   * */
struct __lxl_tpl__
{
    uint32_t                    threads_max;        //线程池线程最大值
    uint32_t                    threads_min;        //线程池线程最小值
    uint32_t                    threads_cur;        //线程池线程当前值
    volatile BOOL               run_flag;           //停止线程标志位
    pthread_mutex_t             master_mutex;
    pthread_cond_t              master_cond;
    pthread_t                   master_thread_id;   //管理线程池线程的ID
    lxl_thread_info_st          *thread_info;       //线程池分配位置
    lxl_thread_queue_st         *thread_queue_list; //空闲线程队列
};



#endif
