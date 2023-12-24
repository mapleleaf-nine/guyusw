#include "threadManager.h"
 
#define _CLEAN_TIME_ 30
#define _RETRY_CREATE_TIMES_ 3
 
static thread_info_t sgThreadInfo;
 
int create_thread_pool(int min_count , int max_count)
{
    int i = 0;
    int ilRet = 0;
    int ilRetry = 0;
 
    memset(&sgThreadInfo , 0 , sizeof(thread_info_t));
    if( pthread_mutex_init(&(sgThreadInfo.mutex) , NULL) != 0 )
    {
        puts("pthread_mutex_init() failed!");
        return -1;
    }
    if(pthread_cond_init(&(sgThreadInfo.cond), NULL) != 0)
    {
        puts("pthread_cond_init() cond failed!");
        return -1;
    }
    if(pthread_cond_init(&(sgThreadInfo.condGetTask) , NULL) != 0)
    {
        puts("pthread_cond_init() condGetTask failed!");
        return -1;
    }
    sgThreadInfo.iMinThreadCount = min_count;
    sgThreadInfo.iMaxThreadCount = max_count;
    sgThreadInfo.iShutdownFlag = 0;
 
    //创建min_count数量的线程
    for(i = 0 ; i<min_count ; )
    {
        pthread_t tid;
        ilRet = pthread_create(&tid , NULL , thread_handle , NULL);
        if(ilRet != 0)
        {
            if(ilRetry == _RETRY_CREATE_TIMES_)
            {
                printf("[ERROR] pthread_create() retry [%d] times failed! Exit...\n" , _RETRY_CREATE_TIMES_);
                return -1;
            }
            puts("[ERROR] pthread_create() failed! Retry ..");
            ilRetry++;
            continue;
        }
        sgThreadInfo.iAliveThreadCount++;
        i++;
    }
 
    printf("Thread pool init completed! Alive_thread_count is [%d]..\n" , sgThreadInfo.iAliveThreadCount);
   
    ilRet = pthread_create(&(sgThreadInfo.managerTid) , NULL , thread_manager , NULL);
    if(ilRet != 0)
    {
        puts("[ERROR] Create thread manager failed! Exit...");
        return -1;
    }
    printf("Create thread manager successful! Manager's tid is [%lu]\n" , sgThreadInfo.managerTid);
 
    return 0;
}
 
int add_task(int (*call)(void *arg) , void *arg)
{
    //上锁添加任务节点
    pthread_mutex_lock(&(sgThreadInfo.mutex));
    if(sgThreadInfo.pTaskTail == NULL)
    {
        sgThreadInfo.pTaskHead = (task_node_t *)malloc(sizeof(task_node_t));
        memset(sgThreadInfo.pTaskHead , 0 , sizeof(task_node_t));
        sgThreadInfo.pTaskTail = sgThreadInfo.pTaskHead;
        sgThreadInfo.pTaskHead->call = call;
        sgThreadInfo.pTaskHead->arg  = arg;
    }
    else
    {
        sgThreadInfo.pTaskTail->next = (task_node_t *)malloc(sizeof(task_node_t));
        memset(sgThreadInfo.pTaskTail->next , 0 , sizeof(task_node_t));
        sgThreadInfo.pTaskTail = sgThreadInfo.pTaskTail->next;
        sgThreadInfo.pTaskTail->call = call;
        sgThreadInfo.pTaskTail->arg  = arg;
    }
    pthread_mutex_unlock(&(sgThreadInfo.mutex));
 
    return 0;
}
 
void *thread_handle(void *arg)
{
    int (*call)(void *);
    void *argTmp;
    task_node_t *plTmp;
    int ilRet = 0;
 
    //设置线程分离，不必在主进程join等待线程回收
    pthread_detach(pthread_self());
 
    //线程无限循环等待新的任务
    while(1)
    {
        pthread_mutex_lock(&(sgThreadInfo.mutex));
 
        //无任务、非关闭状态下则等待signal信号
        if(sgThreadInfo.pTaskHead == NULL && \
           sgThreadInfo.iShutdownFlag == 0)
        {
            //等待signal通知，等待期间pthread_cond_wait函数会自动释放锁供其他线程使用，收到cond后再上锁
            pthread_cond_wait(&(sgThreadInfo.cond) , &(sgThreadInfo.mutex));
        }
 
        //优先级 :shutdown > task ; 关闭时任务队列中的任务直接抛弃
        if(sgThreadInfo.iShutdownFlag == 1)
        {
            //减少存活线程数量
            sgThreadInfo.iAliveThreadCount--;
            printf("Shutdown , tid[%lu] exit! Alive thread count [%d]\n" , pthread_self() , sgThreadInfo.iAliveThreadCount);
            pthread_mutex_unlock(&(sgThreadInfo.mutex));
            pthread_exit(NULL);
        }
        else if(sgThreadInfo.pTaskHead)
        {
            //增加工作线程数量
            sgThreadInfo.iWorkThreadCount++;
            printf("New task! Tid[%lu], Alive count[%d] ~ Work count[%d]\n" , pthread_self() , \
                                                                            sgThreadInfo.iAliveThreadCount , \
                                                                            sgThreadInfo.iWorkThreadCount);
            //获取线程中的函数及参数指针，并使全局变量的head向后偏移
            call = sgThreadInfo.pTaskHead->call;
            argTmp = sgThreadInfo.pTaskHead->arg;
            plTmp = sgThreadInfo.pTaskHead;
            sgThreadInfo.pTaskHead = sgThreadInfo.pTaskHead->next;
            if(sgThreadInfo.pTaskHead == NULL)
            {
                sgThreadInfo.pTaskTail = NULL;
            }
            //修改任务链表头结点和工作线程数量后，发送condGetTask信号让manager线程结束等待分配下一个任务
            pthread_cond_signal(&(sgThreadInfo.condGetTask));
            //解锁去执行任务
            pthread_mutex_unlock(&(sgThreadInfo.mutex));
 
            ilRet = (*call)(argTmp);
            if(ilRet != 0)
            {
                printf("[ERROR] 执行函数指针失败！\n");
            
            }
            //执行完成，释放节点
            free(plTmp);
            //释放参数结构体指针指向的空间
            free(argTmp);
 
            //任务执行完成，上锁去修改工作线程数量的值
            pthread_mutex_lock(&(sgThreadInfo.mutex));
            sgThreadInfo.iWorkThreadCount--;
            printf("Tid[%lu] exec func completed! Alive count[%d] ~ Work count[%d]\n" , pthread_self() , \
                                                                                     sgThreadInfo.iAliveThreadCount , \
                                                                                     sgThreadInfo.iWorkThreadCount);
            //全部完成，解锁
            pthread_mutex_unlock(&(sgThreadInfo.mutex));
            //等待获取下一个任务
            continue;
        }
        else
        {
            printf("Invalid condition! Continue.\n");
            continue;
        }
 
    }
 
    return NULL;
}
 
int fun_thread_kill_self(void *arg)
{
    pthread_mutex_lock(&(sgThreadInfo.mutex)); 
    sgThreadInfo.iAliveThreadCount--;
    sgThreadInfo.iWorkThreadCount--;
    printf("Clean self , tid[%lu] exit! Alive count is [%d]\n" , pthread_self() , sgThreadInfo.iAliveThreadCount);
    pthread_mutex_unlock(&(sgThreadInfo.mutex));
    pthread_exit(NULL);
    
    return 0;
}
 
//线程管理者，循环定时清理多余线程
void *thread_manager(void *arg)
{
    //根据规则控制存活线程数量
    int ilIsHaveIdleThreadFlag = 0;
    int ilRet = 0;
 
    signal(SIGALRM , sig_clean);
    alarm(_CLEAN_TIME_);
 
    while(1)
    {
        //取消点
        pthread_testcancel();
        //上锁判断是否有空闲线程
        pthread_mutex_lock(&(sgThreadInfo.mutex));
        //每次循环都检查任务头节点并是否存在，若存在则判断是否有空闲线程执行任务或创建新的线程
        if(sgThreadInfo.pTaskHead)
        {
            //符合条件则创建新线程，判断存活线程数量是否等于工作线程，根据情况判定是否需要新增线程执行任务
            if(sgThreadInfo.iAliveThreadCount == sgThreadInfo.iWorkThreadCount && \
                sgThreadInfo.iAliveThreadCount < sgThreadInfo.iMaxThreadCount)
            {
                pthread_t tid;
                ilRet = pthread_create(&tid , NULL , thread_handle , NULL);
                if(ilRet != 0)
                {
                    puts("pthread_create() failed! Retry...");
                    pthread_mutex_unlock(&(sgThreadInfo.mutex));
                    continue;
                }
                printf("Thread count is not full , pthread_create() successful! tid[%lu]\n" , tid);
                sgThreadInfo.iAliveThreadCount++;
                ilIsHaveIdleThreadFlag = 1;
            }
            else if(sgThreadInfo.iAliveThreadCount > sgThreadInfo.iWorkThreadCount)
            {
                printf("Have idle thread! alive[%d]~work[%d]\n" , sgThreadInfo.iAliveThreadCount ,\
                        sgThreadInfo.iWorkThreadCount);
                ilIsHaveIdleThreadFlag = 1;
            }
            else
            {
                ilIsHaveIdleThreadFlag = 0;
            }
            pthread_mutex_unlock(&(sgThreadInfo.mutex));
        
            //存在空闲线程发送信号使其运行，并等待其返回getTask条件
            //（若不等待其返回getTask条件，会导致工作线程还未收到singnal时，主进程已进入下一个add_task函数并上锁，
            //导致工作线程数量iWorkThreadCount并未增加，数据混乱）
            if(ilIsHaveIdleThreadFlag)
            {
                //有空闲线程，则发送信号执行任务
                pthread_mutex_lock(&(sgThreadInfo.mutex));
                pthread_cond_signal(&(sgThreadInfo.cond));
                //等待工作线程成功获取任务修改工作线程数量并且移动任务链表头结点再进入下次循环，执行后面的任务
                pthread_cond_wait(&(sgThreadInfo.condGetTask) , &(sgThreadInfo.mutex));
                pthread_mutex_unlock(&(sgThreadInfo.mutex));
            }
        }
        else
            pthread_mutex_unlock(&(sgThreadInfo.mutex));
    }
 
    return NULL;
}
 
void sig_clean(int signum)
{
    int ilCleanThreadCount = 0;
    int i = 0;
 
    pthread_mutex_lock(&(sgThreadInfo.mutex));
    if((sgThreadInfo.iWorkThreadCount*2) < sgThreadInfo.iAliveThreadCount && \
        sgThreadInfo.iShutdownFlag == 0)
    {
        if((sgThreadInfo.iWorkThreadCount*2) > sgThreadInfo.iMinThreadCount)
        {
            ilCleanThreadCount = sgThreadInfo.iAliveThreadCount - \
                                            (sgThreadInfo.iWorkThreadCount*2);
        }
        else
        {
            ilCleanThreadCount = sgThreadInfo.iAliveThreadCount - \
                                            sgThreadInfo.iMinThreadCount;
        }
    }
    if(ilCleanThreadCount > 0)
    {
        printf("********Clean thread start! Current alive thread count[%d] , clean count[%d]********\n" , \
                                            sgThreadInfo.iAliveThreadCount , ilCleanThreadCount);
    }
    pthread_mutex_unlock(&(sgThreadInfo.mutex));
    //添加自杀任务
    for(i=0 ; i<ilCleanThreadCount ; i++)
    {
        add_task(fun_thread_kill_self , NULL);
    }
 
    alarm(_CLEAN_TIME_);
}
 
int destroy_thread_pool()
{   
    int ilRet = 0;
    task_node_t *plCur;
    task_node_t *plFreeTmp;
 
    pthread_mutex_lock(&(sgThreadInfo.mutex));
    sgThreadInfo.iShutdownFlag = 1;
    pthread_mutex_unlock(&(sgThreadInfo.mutex));
 
    //循环直到结束所有线程为止
    while(1)
    {
        pthread_mutex_lock(&(sgThreadInfo.mutex));
        if(sgThreadInfo.iAliveThreadCount == 0)
        {
            pthread_mutex_unlock(&(sgThreadInfo.mutex));
            break;
        }
        //广播唤醒所有的等待线程
        pthread_cond_broadcast(&(sgThreadInfo.cond));
        pthread_mutex_unlock(&(sgThreadInfo.mutex));
        sleep(1);
    }
    puts("Closed common thread successful!");
 
    ilRet = pthread_cancel(sgThreadInfo.managerTid);
    pthread_join(sgThreadInfo.managerTid , NULL);
    printf("Closed manager thread successful! %d \n", ilRet);
 
    //释放任务链表
    plCur = sgThreadInfo.pTaskHead;
    while(plCur)
    {
        plFreeTmp = plCur;
        plCur = plCur->next;
        free(plFreeTmp);
    }
    puts("Free task list successful!");
 
    pthread_mutex_destroy(&(sgThreadInfo.mutex));
    pthread_cond_destroy(&(sgThreadInfo.cond));
    puts("Destroyed mutex and cond successful!");
        
    return 0;
}