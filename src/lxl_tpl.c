/**
 * @File      lxl_tpl.c
 * @Function  
 * @Author    LXL ,1010361011@qq.com
 * @version   0.1
 * @Date      2017-12-21
 */

#include "lxl_tpl.h"
#include "lxl_misc.h"
#include "lxl_log.h"
#include "lxl_thread_queue.h"


static volatile int threads_on_hold;//暂停线程池的标志位
static void set_thread_signal_handlers(void);
static void lxl_thread_hold(int sig_id);

/**
 * @Function  lxl_tpl_create 
 *
 * @Param     min
 * @Param     max
 * @Param     error
 *
 * @Returns   
 */
lxl_tpl_st *lxl_tpl_create(int min, int max, char **error)
{
    const char *__function_name = "lxl_tpl_create";
    lxl_tpl_st  *tpl = NULL;


    lxl_log_info(LEVEL_LOG_DEBUG, "In %s().", __function_name);


    tpl = (lxl_tpl_st *)malloc(sizeof(lxl_tpl_st));
    if(NULL == tpl)
    {
        *error = lxl_strdup("allocation tpl memory failed.");
        goto out;
    }

    memset(tpl, 0, sizeof(lxl_tpl_st));
    tpl->threads_min = min;
    tpl->threads_max = max;
    tpl->threads_cur = min;
    lxl_log_info(LEVEL_LOG_DEBUG, "End %s(), tpl:%lu",__function_name, (uint64_t)tpl);

out:
    return tpl;
}



/**
 * @Function  tpl_work_thread
 *
 * @Param     arg
 *
 * @Returns
 */
static void *tpl_work_thread(void *arg)
{


    const char *__function_name = "tpl_work_thread";
    lxl_thread_info_st *thread_info = (lxl_thread_info_st *)arg;
    lxl_tpl_st *tpl = thread_info->thread_pool;

    set_thread_signal_handlers();
#if 0
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    act.sa_handler = lxl_thread_hold;
    if (sigaction(SIGUSR1, &act, NULL) == -1)
    {
        lxl_log_error("tpl_work_thread(): cannot handle SIGUSR1");
    }
#endif
    //notice master thread, ready
    pthread_mutex_lock(&tpl->master_mutex);
    pthread_cond_signal(&tpl->master_cond);
    pthread_mutex_unlock(&tpl->master_mutex);




    while(tpl->run_flag)
    {



        lxl_log_info(LEVEL_LOG_DEBUG, "In %s(), thread ID:%d.", __function_name, thread_info->thread_num);
        if(thread_info->proc_func)
        {

            thread_info->proc_func(thread_info->arg);

            lxl_log_info(LEVEL_LOG_DEBUG, "执行回掉函数\n");
            thread_info->arg = NULL;
            thread_info->proc_func = NULL;
            thread_info->is_busy = NO;
            lxl_thread_enqueue(tpl->thread_queue_list, thread_info);
        }
        pthread_mutex_lock(&thread_info->mutex);
        pthread_cond_wait(&thread_info->cond, &thread_info->mutex);
        pthread_mutex_unlock(&thread_info->mutex);


        lxl_log_info(LEVEL_LOG_DEBUG, "End %s().", __function_name);

    }

    return 0;
}

#define BUSY_THRESHOLD 0.5

/**
 * @Function  get_tpl_status
 *
 * @Param     tpl
 *
 * @Returns
 */
static int get_tpl_status(lxl_tpl_st *tpl)
{
    const char *__function_name = "get_tpl_status";
    int ret = FAIL;

    float busy_num = 0.0;



    lxl_log_info(LEVEL_LOG_DEBUG, "In %s().", __function_name);

    busy_num = tpl->threads_cur - tpl->thread_queue_list->count;

    if(busy_num / (tpl->threads_cur) < BUSY_THRESHOLD)
        ret = 0; // idle status
    else
        ret =  1;// busy or normal status


    lxl_log_info(LEVEL_LOG_DEBUG, "End %s().status:%d.",__function_name, ret);


    return ret;
}



/**
 * @Function  delete_tpl_thread
 *
 * @Param     tpl
 *
 * @Returns
 */
static int delete_tpl_thread(lxl_tpl_st *tpl)
{
    const char *__function_name = "delete_tpl_thread";
    lxl_thread_info_st *thread_info = NULL;
    int ret = FAIL;

    lxl_log_info(LEVEL_LOG_DEBUG, "In %s().", __function_name);

    thread_info = lxl_thread_dequeue(tpl->thread_queue_list);
    if(thread_info == NULL)
        goto out;

    pthread_mutex_lock(&tpl->master_mutex);
    //未测试
    pthread_kill(thread_info->thread_id, SIGKILL);
    pthread_mutex_destroy(&thread_info->mutex);
    pthread_cond_destroy(&thread_info->cond);
    memset(thread_info, 0, sizeof(lxl_thread_info_st));
    pthread_mutex_unlock(&tpl->master_mutex);
    ret = SUCCESS;
out:
    lxl_log_info(LEVEL_LOG_DEBUG, "End %s().status:%d.",__function_name, ret);
    return ret;
}





/**
 * @Function  tpl_master_thread
 *
 * @Param     arg
 *
 * @Returns
 */
static void * tpl_master_thread(void *arg)
{
    lxl_tpl_st *tpl = (lxl_tpl_st *)arg;



    while(tpl->run_flag)
    {

        if(get_tpl_status(tpl) == 0)
        {
            do
            {
                if(FAIL == delete_tpl_thread(tpl))
                    break;
            }while(tpl->run_flag);
        }
        sleep(10);
    }



    return 0;
}




/**
 * @Function  lxl_tpl_init
 *
 * @Param     tpl
 * @Param     error
 *
 * @Returns
 */
int lxl_tpl_init(lxl_tpl_st *tpl, char **error)
{
    int ret  = FAIL;
    int index = 0;
    const char *__function_name = "lxl_tpl_init";
    lxl_thread_info_st *thread_info;
    lxl_log_info(LEVEL_LOG_DEBUG, "In %s().", __function_name);

    tpl->run_flag = YES;

    tpl->thread_info = (lxl_thread_info_st *)malloc(sizeof(lxl_thread_info_st) * tpl->threads_max);
    if(NULL == tpl->thread_info)
    {
        *error = lxl_strdup("allocate lxl_thread_info_st memory failed.");
        free(tpl);
        goto out;
    }

    memset(tpl->thread_info , 0, sizeof(lxl_thread_info_st) * tpl->threads_max);



    tpl->thread_queue_list = lxl_thread_queue_create(error);
    if(NULL == tpl->thread_queue_list)
    {
        free(tpl->thread_info);
        free(tpl);
        goto out;
    }

    pthread_mutex_init(&tpl->master_mutex, NULL);
    pthread_cond_init(&tpl->master_cond, NULL);



    for(; index < (int)tpl->threads_min; index++)
    {
        thread_info = tpl->thread_info + index;

        thread_info->is_busy = NO;
        thread_info->proc_func = NULL;
        thread_info->thread_num = index;
        thread_info->thread_pool = tpl;
        thread_info->arg = NULL;
        pthread_mutex_init(&thread_info->mutex, NULL);
        pthread_cond_init(&thread_info->cond, NULL);
        pthread_create(&thread_info->thread_id, NULL,tpl_work_thread,thread_info);
        pthread_detach(thread_info->thread_id);
        lxl_thread_enqueue(tpl->thread_queue_list, thread_info);
    }

    pthread_create(&tpl->master_thread_id, NULL, tpl_master_thread, tpl);
    pthread_detach(tpl->master_thread_id);



    //wait all work threads
    while(index++ < tpl->threads_cur)
    {

        pthread_mutex_lock(&tpl->master_mutex);
        pthread_cond_wait(&tpl->master_cond, &tpl->master_mutex);
        pthread_mutex_unlock(&tpl->master_mutex);
    }

    ret = SUCCESS;
out:
    lxl_log_info(LEVEL_LOG_DEBUG, "End %s().", __function_name);
    return ret;
}










/**
 * @Function  lxl_tpl_destroy 
 *
 * @Param     tpl
 */
void lxl_tpl_destroy(lxl_tpl_st *tpl)
{
    const char *__function_name = "lxl_tpl_destroy";
    int index;
    lxl_thread_info_st *thread_info = NULL;


    lxl_log_info(LEVEL_LOG_DEBUG, "In %s().", __function_name);

    tpl->run_flag = NO;
    usleep(100);
    for(index = 0; index < tpl->threads_cur; index ++)
    {
        thread_info = tpl->thread_info + index;

        pthread_cond_signal(&thread_info->cond);

        pthread_mutex_destroy(&thread_info->mutex);
        pthread_cond_destroy(&thread_info->cond);
    }


    usleep(100);
    lxl_thread_queue_destroy(tpl->thread_queue_list);

    free(tpl->thread_info);

    pthread_mutex_destroy(&tpl->master_mutex);
    pthread_cond_destroy(&tpl->master_cond);
    free(tpl);


    lxl_log_info(LEVEL_LOG_DEBUG, "End %s().", __function_name);
}



/**
 * @Function  tpl_add_thread 
 *
 * @Param     tpl
 * @Param     proc_fun
 * @Param     arg
 * @Param     error
 *
 * @Returns   
 */
static lxl_thread_info_st *tpl_add_thread(lxl_tpl_st *tpl, process_job proc_fun, void *arg, char **error)
{
    lxl_thread_info_st *new_thread;

    pthread_mutex_lock(&tpl->master_mutex);
    if (tpl->threads_max <=tpl->threads_cur)
    {
        pthread_mutex_unlock(&tpl->master_mutex);
        return NULL;
    }

    //malloc new thread info struct
    new_thread = tpl->thread_info + tpl->threads_cur;
    tpl->threads_cur++;
    pthread_mutex_unlock(&tpl->master_mutex);

    new_thread->thread_pool = tpl;

    pthread_mutex_init(&new_thread->mutex, NULL);
    pthread_cond_init(&new_thread->cond, NULL);

    //init status is busy, only new process job will call this function
    new_thread->is_busy = YES;
    new_thread->proc_func = proc_fun;
    new_thread->arg = arg;

    pthread_create(&new_thread->thread_id, NULL, tpl_work_thread, new_thread);
    pthread_detach(new_thread->thread_id);
    return new_thread;
}



/**
 * @Function  tpl_process_job 
 *
 * @Param     tpl
 * @Param     proc_fun
 * @Param     arg
 * @Param     error
 *
 * @Returns   
 */
int tpl_process_job(lxl_tpl_st *tpl, process_job proc_fun, void *arg, char **error)
{
    const char *__function_name = "tpl_process_job";

    lxl_log_info(LEVEL_LOG_DEBUG, "In %s().", __function_name);


    lxl_thread_info_st *thread_info = NULL;

    thread_info  = lxl_thread_dequeue(tpl->thread_queue_list);
    if(thread_info)
    {

        thread_info->is_busy = YES;
        thread_info->proc_func = proc_fun;
        thread_info->arg = arg;
        pthread_cond_signal(&thread_info->cond);
    }
    else
    {

        if(!(thread_info = tpl_add_thread(tpl, proc_fun, arg, error)))
        {
            return -1;
        }
    }

    lxl_log_info(LEVEL_LOG_DEBUG, "End %s().", __function_name);


    return 0;
}




/**
 * @Function  lxl_thread_hold 
 * 暂停所有线程的信号处理函数
 * @Param     sig_id
 */

static void thread_hold(int sig, siginfo_t *siginfo, void *context)
{
    threads_on_hold = 1;
    while (threads_on_hold)
    {
        sleep(1);
    }
}



/**
 * @Function  lxl_thpool_pause 
 *
 * @Param     queue
 * 给线程池里面的线程发送信号，暂停
 * @Returns   
 */
void lxl_tpl_pause(lxl_thread_queue_st *queue)
{
    lxl_thread_item_st *tmp = queue->head;
    if(tmp != queue->tail)
    {
        pthread_mutex_lock(&queue->queue_mutex);
        for(;tmp != queue->tail;tmp = tmp->next)
        {
            pthread_kill(tmp->thread_info->thread_id, SIGUSR1);
        }
        pthread_mutex_unlock(&queue->queue_mutex);
    }

}



static void set_thread_signal_handlers(void)
{
    struct sigaction phan;


    sigemptyset(&phan.sa_mask);
    phan.sa_sigaction = thread_hold;
    sigaction(SIGUSR1, &phan, NULL);

}



/**
 * @Function  lxl_thread_resume 
 * 恢复所有线程
 */
void lxl_thread_resume(void)
{
    threads_on_hold = 0;
}