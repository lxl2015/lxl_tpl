#include "lxl_thread_queue.h"
#include "lxl_log.h"
#include "lxl_misc.h"
#define IDLE_QUEUE_LEN          160


static void thread_queue_init(lxl_thread_queue_st *queue)
{
    lxl_thread_item_st *item;
    pthread_mutex_init(&queue->idle_queue_item_mutex, NULL);
    pthread_mutex_init(&queue->queue_mutex, NULL);

    queue->head = NULL;
    queue->tail = NULL;

    queue->count = 0;
    queue->idle_queue_item = (lxl_thread_item_st *)malloc(sizeof(lxl_thread_item_st) * IDLE_QUEUE_LEN);
    item = queue->idle_queue_item;
    queue->node = item;
    int index;
    for(index = 1; index < IDLE_QUEUE_LEN; index ++)
        item[index -1].next = &item[index];

    pthread_mutex_lock(&queue->idle_queue_item_mutex);
    item[IDLE_QUEUE_LEN - 1].next = queue->idle_queue_item;
    queue->idle_queue_item = item;
    pthread_mutex_unlock(&queue->idle_queue_item_mutex);

}


lxl_thread_queue_st *lxl_thread_queue_create(char ** error)
{
    const char *__function_name = "lxl_thread_queue_create";
    lxl_thread_queue_st *queue = NULL;
    lxl_log_info(LEVEL_LOG_DEBUG, "In %s().", __function_name);
    queue = (lxl_thread_queue_st *)malloc(sizeof(lxl_thread_queue_st));
    if(NULL == queue)
    {
        *error = lxl_strdup("allocate lxl_thread_queue_st memory failed.");
        goto out;
    }


    memset(queue, 0, sizeof(lxl_thread_queue_st));
    thread_queue_init(queue);
out:
    lxl_log_info(LEVEL_LOG_DEBUG, "End %s().", __function_name);
    return queue;
}

static lxl_thread_item_st *idle_queue_item(lxl_thread_queue_st *queue)
{
    const char *__function_name = "idle_queue_item";
    lxl_thread_item_st *item = NULL;

    lxl_log_info(LEVEL_LOG_DEBUG, "In %s().", __function_name);


    pthread_mutex_lock(&queue->idle_queue_item_mutex);
    item = queue->idle_queue_item;
    if(item->next == item)
    {
        return NULL;
    }
    queue->idle_queue_item = item->next;
    pthread_mutex_unlock(&queue->idle_queue_item_mutex);

    lxl_log_info(LEVEL_LOG_DEBUG, "End %s().",__function_name);
    return item;
}
/*
 * 入队
 * */
int lxl_thread_enqueue(lxl_thread_queue_st *queue, lxl_thread_info_st *thread_info)
{
    const char *__function_name = "lxl_thread_enqueue";
    int ret = FAIL;
    lxl_thread_item_st *item = NULL;
    lxl_log_info(LEVEL_LOG_DEBUG, "In %s().", __function_name);

    //only first enqueue maybe happen item = NULL
    item = idle_queue_item(queue);
    if(item == NULL)
        goto out;


    item->thread_info = thread_info;

    pthread_mutex_lock(&queue->queue_mutex);
    item->next = NULL;
    if(NULL == queue->tail)
        queue->head = item;
    else
        queue->tail->next = item;

    queue->tail = item;
    queue->count ++;
    pthread_mutex_unlock(&queue->queue_mutex);

out:
    lxl_log_info(LEVEL_LOG_DEBUG, "End %s().", __function_name);
    return ret;
}

/*
 * 出队
 *
 * */
lxl_thread_info_st * lxl_thread_dequeue(lxl_thread_queue_st *queue)
{
    const char * __function_name = "lxl_thread_dequeue";
    lxl_thread_info_st *thread_info = NULL;
    lxl_thread_item_st *item = NULL;

    lxl_log_info(LEVEL_LOG_DEBUG, "In %s().", __function_name);

    pthread_mutex_lock(&queue->queue_mutex);
    item = queue->head;
    if(item)
    {
        queue->head = item->next;
        if(queue->head)
            queue->tail = NULL;
        queue->count --;
    }
    pthread_mutex_unlock(&queue->queue_mutex);

    if(item)
    {
        pthread_mutex_lock(&queue->idle_queue_item_mutex);
        item->next = queue->idle_queue_item;
        queue->idle_queue_item = item;
        pthread_mutex_unlock(&queue->idle_queue_item_mutex);
        thread_info = item->thread_info;


    }



    lxl_log_info(LEVEL_LOG_DEBUG, "End %s().", __function_name);

    return thread_info;
}

/*
 * 销毁队列
 * */
int lxl_thread_queue_destroy( lxl_thread_queue_st *queue)
{

    const char *__function_name = "lxl_thread_queue_destroy";

    lxl_log_info(LEVEL_LOG_DEBUG, "In %s().", __function_name);
    pthread_mutex_destroy(&queue->idle_queue_item_mutex);
    pthread_mutex_destroy(&queue->queue_mutex);
    
    free(queue->node);
    free(queue);



    lxl_log_info(LEVEL_LOG_DEBUG, "End %s().", __function_name);
    return SUCCESS;
}
