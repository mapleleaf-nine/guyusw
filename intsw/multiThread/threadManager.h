#ifndef _THREAD_MANAGER_H_
#define _THREAD_MANAGER_H_

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
 
/*任务链表节点*/
typedef struct task_node
{
	int (*call)(void *);    //函数指针
	void *arg;              //参数指针
	struct task_node *next;
}task_node_t;
 
/*线程池全局数据结构体*/
typedef struct
{
    pthread_mutex_t mutex;
    pthread_cond_t cond;        //工作线程等待条件
    pthread_cond_t condGetTask; //管理者线程等待条件
 
    pthread_t managerTid;       //管理者线程id
 
    task_node_t *pTaskHead;     //任务链表头
    task_node_t *pTaskTail;     //任务链表尾
 
    int iMaxThreadCount;        //最大线程数量
    int iMinThreadCount;        //最小线程数量
    int iWorkThreadCount;       //当前正在工作的线程数量
    int iAliveThreadCount;      //当前所有存活线程数量（不包括管理者线程）
 
    int iShutdownFlag;          //关闭标志
}thread_info_t;
 
//参数结构体，将所有待执行函数可能用到的参数都放入此结构体，对用处理函数用不到的参数赋0即可
//这里测试函数fun_demo1使用到id和name，fun_demo2使用到age和sex
typedef struct
{
    uint8_t cpriRate;
    uint8_t disableNum;
    uint32_t seed4;
}arg_struct_t;
 
//创建并初始化线程池
int create_thread_pool(int min_count , int max_count);
//添加任务节点到任务链表
int add_task(int (*call)(void *arg) , void *arg);
//工作线程
void *thread_handle(void *arg);
//管理者线程
void *thread_manager(void *arg);
//销毁线程池
int destroy_thread_pool();
//线程任务函数，自我销毁
int fun_thread_kill_self(void *arg);
 
//管理者线程信号处理函数，清理多余空闲线程
void sig_clean(int signum);

#endif /*_THREAD_MANAGER_H_*/