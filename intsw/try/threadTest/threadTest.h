#ifndef _THREAD_TEST_H_
#define _THREAD_TEST_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>

//任务链表节点信息
typedef struct task_node
{
    int (*task_call)(void *);       //函数指针
    void *arg;                      //参数指针
    struct task_node *next;         //任务链表节点next指针
}task_node_t;

typedef struct 
{
    pthread_mutex_t mutex;          //锁线程池的锁
    pthread_cond_t cond;            //工作线程等待条件
    pthread_cond_t condGetTask;     //管理者线程等待条件
    pthread_t managerTid;            //管理者线程ID

    task_node_t *pTaskHead;         //任务链表表头
    task_node_t *pTaskTail;         //任务链表表尾

    int iMinThreadCount;            //最小线程数量
    int iMaxThreadCount;            //最大线程数量
    int iWorkThreadCount;           //当前正在工作的线程数量
    int iAliveThreadCount;          //当前存活的线程数量（不包括管理者线程）

    int iShutdownFlag;              //关闭标志
}thread_info_t;

typedef struct
{
    uint8_t 
}arg_struct_t;


//创建并初始化线程池
int thread_pool_create(int min_cout, int max_count);
// int thread_pool_create(thread_info_t *sgThreadInfo, int min_cout, int max_count);

//添加任务节点到任务链表
int thread_pool_add_tasker(int (*task_call)(void *), void *arg);
// int thread_pool_add_tasker(thread_info_t *sgThreadInfo, int (*task_call)(void *), void *arg);

//销毁线程池
int thread_pool_destory();
// int thread_pool_destory(thread_info_t sgThreadInfo);

//线程任务函数，自我销毁
int thread_kill_self(void *arg);
 
//管理者线程信号处理函数，清理多余空闲线程
void sig_clean_call(int sig_num);

//工作线程回调函数
void *thread_work_call(void *arg);

//管理者线程回调函数
void *thread_manager_call(void *arg);


// 在线程池中，通常会有两种类型的线程：存活线程（live threads）和工作线程（worker threads）。

// 存活线程（Live Threads）：
// 存活线程是已创建并仍处于活动状态的线程。它们可能正在执行任务，也可能处于空闲状态等待新的任务分配。
// 存活线程的数量通常由线程池的配置参数决定，例如最小线程数和最大线程数。

// 工作线程（Worker Threads）：
// 工作线程是线程池中正在执行任务的线程。它们从任务队列中获取任务，并执行相应的工作。
// 工作线程负责执行实际的任务逻辑，例如处理请求、计算、IO 操作等。线程池的主要目的是重用工作线程，以避免频繁地创建和销毁线程，从而提高效率。

// 线程池的设计思想是将任务与线程的生命周期分离开来。存活线程在整个线程池的生命周期内存在，而工作线程根据任务的到达和完成动态地创建和销毁。

// 通过维护一定数量的存活线程，线程池可以更快地响应新的任务，而无需为每个任务都创建新的线程。工作线程执行任务后，可以继续从任务队列中获取新的任务，以保持高效的任务处理能力。

// 总结起来，存活线程是线程池中已创建且仍处于活动状态的线程，而工作线程是正在执行任务的线程。线程池通过维护存活线程和动态创建工作线程的方式，提供了一种高效管理任务执行的机制。
#endif /* _THREAD_TEST_H_ */