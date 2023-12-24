#include "threadTest.h"

#define _CLEAN_TIME_ 30
#define _RETRY_CREATE_TIMES_ 3

static thread_info_t sgThreadInfo;

//释放任务链表节点，减少工作线程数量
//调用exit函数，退出线程，减少工作线程和存活线程数量

//工作线程回调函数，根据关闭标志和任务链表是否为空执行
//发送condGetTask信号，等待cond信号
void *thread_work_call(void *arg)
{
    int (*task_call)(void *);
    void *argTmp;
    task_node_t *plTmp;
    int ilRet;

    //子线程中，设置线程分离，不用在主线程中join阻塞等待线程回收
    //主线程使用pthread_detach(threadId)，threadId指子线程id
    pthread_detach(pthread_self());

    //线程循环等待新的任务节点
    while(1)
    {
        //互斥锁，进入新的任务，加锁，进入临界区
        //保证pthread_cond_wait的并发性
        pthread_mutex_lock(&(sgThreadInfo.mutex));

        //1. 无任务、非关闭状态下：等待工作线程信号cond
        if(sgThreadInfo.pTaskHead == NULL && sgThreadInfo.iShutdownFlag == 0)
        {
            //等待signal(cond)通知，等待期间pthread_cond_wait函数会自动释放锁供其他线程使用，收到cond后再上锁
            pthread_cond_wait(&(sgThreadInfo.cond), &(sgThreadInfo.mutex));
        }

        //2.1 关闭标志为1：优先级(shutdown > task)；任务队列的存活任务直接抛弃
        if(sgThreadInfo.iShutdownFlag == 1)
        {
            //抛弃存活线程
            sgThreadInfo.iAliveThreadCount--;
            printf("Shutdown , tid[%lu] exit! Alive thread count [%d]\n" , pthread_self() , \
                                                                            sgThreadInfo.iAliveThreadCount);
            pthread_mutex_unlock(&(sgThreadInfo.mutex));
            //只退出当前线程，已设置线程分离，不能调用join回收资源
            pthread_exit(NULL);
        }
        //2.2. 关断标志为0，且任务链表不为空：增加工作线程到任务队列中去
        else if(sgThreadInfo.pTaskHead)
        {
            //工作线程增加
            sgThreadInfo.iWorkThreadCount++;
            printf("New task! Tid[%lu], Alive count[%d] ~ Work count[%d]\n" , pthread_self() , \
                                                                            sgThreadInfo.iAliveThreadCount , \
                                                                            sgThreadInfo.iWorkThreadCount);
            //获取线程的函数指针和参数指针，使全局变量的head向后移动
            task_call = sgThreadInfo.pTaskHead->task_call;
            argTmp = sgThreadInfo.pTaskHead->arg;
            plTmp = sgThreadInfo.pTaskHead;
            sgThreadInfo.pTaskHead = sgThreadInfo.pTaskHead->next;
            if(sgThreadInfo.pTaskHead == NULL)
            {
                sgThreadInfo.pTaskTail = NULL;
            }
            //修改任务链表头结点和工作线程数量后，发送condGetTask信号让manager线程结束等待分配下一个任务
            pthread_cond_signal(&(sgThreadInfo.condGetTask));
            //解锁，去执行任务
            pthread_mutex_unlock(&(sgThreadInfo.mutex));

            /***********************************************************************/
            //执行任务，检测任务返回值
            ilRet = (*task_call)(argTmp);
            if(ilRet != 0)
            {
                printf("[ERROR] 执行函数指针失败！\n");
            }
            //执行完成，释放节点
            free(plTmp);
            //释放参数结构体指针指向的空间
            free(argTmp);
            /***********************************************************************/

            //任务执行完成，上锁修改工作线程的数量
            pthread_mutex_lock(&(sgThreadInfo.mutex));
            //工作线程数量减少，存活线程数量保持
            sgThreadInfo.iWorkThreadCount--;
            printf("Tid[%lu] exec func completed! Alive count[%d] ~ Work count[%d]\n" , pthread_self() , \
                                                                                     sgThreadInfo.iAliveThreadCount , \
                                                                                     sgThreadInfo.iWorkThreadCount); 
            //全部完成，解锁
            pthread_mutex_unlock(&(sgThreadInfo.mutex));
            //等待获取下一个任务
        }
        else
        {
            printf("Invalid condition! Continue.\n");
            continue;
        }
    }
    return NULL;
}

//管理者线程回调函数，循环定时清理多余进程
//发送cond信号，等待condGetTask信号
void *thread_manager_call(void *arg)
{
    //根据空闲线程存在标志，控制存活线程数量
    int ilIsHaveIdleThreadFlag = 0;
    int ilRet = 0;

    //SIGALRM信号设定一定响应函数（回调函数）
    signal(SIGALRM, sig_clean_call);
    //当前时刻seconds秒之后产生一个SIGALRM信号
    alarm(_CLEAN_TIME_);

    while(1)
    {
        //设置取消点，结合取消线程函数pthread_cancel()使用;
        //创建manager线程managerId，testcancel设置取消点，调用cancel取消managerId线程。调用成功后不执行取消点后的任务
        thread_testcancel();
        
        //上锁判断是否有空闲线程
        pthread_mutex_lock(&(sgThreadInfo.mutex));
        //检测任务链表是否为空，即任务头节点是否存在，
        //若存在则判断是否有空闲线程去执行任务，存活线程数量大于工作线程数量，表示有空闲线程
        //或者创建新的线程作为空闲线程去执行任务，存活线程数量等于工作线程数量，且存活线程数量小于最大线程池线程数量
        if(sgThreadInfo.pTaskHead)
        {
            //1. 创建新的线程作为空闲线程去执行任务
            if(sgThreadInfo.iAliveThreadCount == sgThreadInfo.iWorkThreadCount && \
                sgThreadInfo.iAliveThreadCount < sgThreadInfo.iMaxThreadCount)
            {
                pthread_t tid;
                ilRet = pthread_create(&tid, NULL, thread_work_call, NULL);
                if(ilRet != 0)
                {
                    printf("pthread_create() failed! Retry...\n");
                    pthread_mutex_unlock(&(sgThreadInfo.mutex));
                    continue;
                }
                printf("Thread count is not full , pthread_create() successful! tid[%lu]\n" , tid);
                //存活线程增加
                sgThreadInfo.iAliveThreadCount++;
                ilIsHaveIdleThreadFlag = 1;
            }
            //2. 有空闲线程不需要创建新线程去执行任务
            else if(sgThreadInfo.iAliveThreadCount > sgThreadInfo.iWorkThreadCount)
            {
                printf("Have idle thread! alive[%d]~work[%d]\n" , sgThreadInfo.iAliveThreadCount ,\
                        sgThreadInfo.iWorkThreadCount);
                ilIsHaveIdleThreadFlag = 1;
            }
            //3. 超出线程池线程最大数量
            else
            {
                ilIsHaveIdleThreadFlag = 0;
            }
            pthread_mutex_unlock(&(sgThreadInfo.mutex));

            //存在空闲线程发送信号cond运行任务，并等待返回的condGetTask通知
            if(ilIsHaveIdleThreadFlag == 1)
            {
                //有空闲任务，发送cond信号去执行任务
                pthread_mutex_lock(&(sgThreadInfo.mutex));
                pthread_cond_signal(&(sgThreadInfo.cond));
                //等待工作线程成功获取任务修改工作线程数量,并且移动任务链表头结点再进入下次循环，执行后面的任务
                pthread_cond_wait(&(sgThreadInfo.condGetTask), &(sgThreadInfo.mutex));
                pthread_mutex_unlock(&(sgThreadInfo.mutex));
            }
        }
        else
        {
            pthread_mutex_lock(&(sgThreadInfo.mutex));
        }
    }
    
}

//管理者线程信号回调函数，清理空闲线程
//存活线程数量大于二倍工作线程数量，空闲线程数量大于存活线程数量的一半时清理多余的空闲线程
void sig_clean_call(int sig_num)
{
    int ilCleanThreadCount = 0;
    int i = 0;

    pthread_mutex_lock(&(sgThreadInfo.mutex));
    if((sgThreadInfo.iWorkThreadCount * 2) < sgThreadInfo.iAliveThreadCount && \
        sgThreadInfo.iShutdownFlag == 0)
    {
        if((sgThreadInfo.iWorkThreadCount * 2) > sgThreadInfo.iMinThreadCount)
        {
            ilCleanThreadCount = sgThreadInfo.iAliveThreadCount - (sgThreadInfo.iWorkThreadCount * 2);
        }
        else
        {
            ilCleanThreadCount = sgThreadInfo.iAliveThreadCount - sgThreadInfo.iMinThreadCount;
        }
    }

    if(ilCleanThreadCount > 0)
    {
        printf("********Clean thread start! Current alive thread count[%d] , clean count[%d]********\n" , \
                                            sgThreadInfo.iAliveThreadCount , ilCleanThreadCount);
    }
    pthread_mutex_unlock(&(sgThreadInfo.mutex));
    //添加自杀任务
    for(i = 0; i < ilCleanThreadCount; i++)
    {
        thread_pool_add_tasker(thread_kill_self, NULL);
    }
    alarm(_CLEAN_TIME_);
}

int thread_kill_self(void *arg)
{
    pthread_mutex_lock(&(sgThreadInfo.mutex));
    sgThreadInfo.iAliveThreadCount--;
    sgThreadInfo.iWorkThreadCount--;
    printf("Clean self , tid[%lu] exit! Alive count is [%d]\n" , pthread_self() , sgThreadInfo.iAliveThreadCount);
    pthread_mutex_unlock(&(sgThreadInfo.mutex));
    pthread_exit(NULL);

    return 0;
}

int thread_pool_create(int min_cout, int max_count)
{
    int ilRet = 0;
    int ilRetry = 0;
    int i = 0;

    memset(&sgThreadInfo, 0, sizeof(thread_info_t));
    if(pthread_mutex_init(&(sgThreadInfo.mutex), NULL) != 0)
    {
        printf("pthread_mutex_init() failed!\n");
        return -1;
    }

    if(pthread_cond_init(&(sgThreadInfo.cond), NULL) != 0)
    {
        puts("pthread_cond_init() cond failed!\n");
        return -1;
    }

    if(pthread_cond_init(&(sgThreadInfo.condGetTask), NULL) != 0)
    {
        puts("pthread_cond_init() condGetTask failed!\n");
        return -1;
    }

    sgThreadInfo.iMinThreadCount = min_cout;
    sgThreadInfo.iMaxThreadCount = max_count;
    sgThreadInfo.iShutdownFlag = 0;

    //创建min_cout数量的线程
    for(i = 0; i < min_cout; )
    {
        pthread_t tid;
        ilRet = pthread_create(&tid, NULL, thread_work_call, NULL);
        if(ilRet != 0)
        {
            if(ilRetry == _RETRY_CREATE_TIMES_)
            {
                printf("[ERROR] pthread_create() retry [%d] times failed! Exit...\n" , _RETRY_CREATE_TIMES_);
                return -1;
            }
            printf("[ERROR] pthread_create() failed! Retry ..\n");
            ilRetry++;
            continue;
        }
        sgThreadInfo.iAliveThreadCount++;
        i++;
    }

    printf("Thread pool init completed! Alive_thread_count is [%d]..\n" , sgThreadInfo.iAliveThreadCount);

    ilRet = pthread_create(&(sgThreadInfo.managerTid) , NULL , thread_manager_call , NULL);
    if(ilRet != 0)
    {
        printf("[ERROR] Create thread manager failed! Exit...\n");
        return -1;
    }
    printf("Create thread manager successful! Manager's tid is [%lu]\n" , sgThreadInfo.managerTid);
 
    return 0;
}

int thread_pool_add_tasker(int (*task_call)(void *), void *arg)
{
    //上锁增加任务节点
    pthread_mutex_lock(&(sgThreadInfo.mutex));
    //任务链表是否为空
    if(sgThreadInfo.pTaskTail == NULL)
    {
        sgThreadInfo.pTaskHead = (task_node_t *)malloc(sizeof(task_node_t));
        memset(sgThreadInfo.pTaskHead, 0, sizeof(task_node_t));
        sgThreadInfo.pTaskTail = sgThreadInfo.pTaskHead;
        sgThreadInfo.pTaskHead->task_call = task_call;
        sgThreadInfo.pTaskHead->arg = arg;
    }
    else
    {
        sgThreadInfo.pTaskTail->next = (task_node_t *)malloc(sizeof(task_node_t));
        memset(sgThreadInfo.pTaskTail->next, 0, sizeof(task_node_t));
        sgThreadInfo.pTaskTail = sgThreadInfo.pTaskTail->next;
        sgThreadInfo.pTaskTail->task_call = task_call;
        sgThreadInfo.pTaskTail->arg = arg;
    }

    pthread_mutex_unlock(&(sgThreadInfo.mutex));
    return 0;
}

int thread_pool_destory()
{
    int ilRet = 0;
    task_node_t *plCur;
    task_node_t *plFreeTmp;

    pthread_mutex_lock(&(sgThreadInfo.mutex));
    sgThreadInfo.iShutdownFlag = 1;
    pthread_mutex_unlock(&(sgThreadInfo.mutex));

    //循环等待所有线程销毁
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
    printf("Closed common thread successful!\n");

    ilRet = pthread_cancel(sgThreadInfo.managerTid);
    //管理者线程回调函数未调用detach函数，需调用join回收线程资源
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
    printf("Free task list successful!\n");
 
    pthread_mutex_destroy(&(sgThreadInfo.mutex));
    pthread_cond_destroy(&(sgThreadInfo.cond));
    printf("Destroyed mutex and cond successful!\n");
        
    return 0;

}



