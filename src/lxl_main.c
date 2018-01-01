/**
 * @File      lxl_main.c
 * @Function  
 * @Author    LXL ,1010361011@qq.com
 * @version   0.1
 * @Date      2017-12-27
 */
#include "lxl_tpl.h"
#include "lxl_log.h"
#define THD_NUM 10

static pthread_mutex_t lock;
static unsigned exit_cnt;
lxl_tpl_st *tpl;

static void terminate_signal_handler(int sig, siginfo_t *siginfo, void *context)
{
    lxl_tpl_destroy(tpl);
}



void lxl_set_process_signal_handlers(void)
{
    struct sigaction phan;


    sigemptyset(&phan.sa_mask);
    phan.sa_flags = SA_SIGINFO;

    phan.sa_sigaction = terminate_signal_handler;
    sigaction(SIGINT, &phan, NULL);
    sigaction(SIGQUIT, &phan, NULL);
    sigaction(SIGTERM, &phan, NULL);

}


void proc_fun(void *arg)
{

    printf("job %d\n", *(int *)idx);
}


int main(int argc, const char *argv[])
{

    char *error;
    uint32_t i;
    const char *log_name = "thread_pool.log";



    lxl_open_log(DEBUG, log_name);
    tpl = lxl_tpl_create(10,200,&error);

    if(tpl == NULL)
        printf("ERROR:%s\n",error);

    exit_cnt = 0;
    lxl_tpl_init(tpl,&error);


    for(i=0; i < THD_NUM; i++)
    {
        usleep(1);
        tpl_process_job(tpl, proc_fun,&i, &error);
    }



    lxl_tpl_destroy(tpl);
    lxl_close_log();
    return 0;
}


