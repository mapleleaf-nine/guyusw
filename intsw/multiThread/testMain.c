#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "testMain.h"
#include "threadManager.h"
#include "cmdRegister.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include "intCmd.h"

#define FIFO_PATH "../fifoPath/cmd.fifo"

int getIntCmd(void *argc)
{
    int fd;
    char cmd[64] = {0};
    char cmd_dfe[24][64] = {0};
    int ret = access(FIFO_PATH, F_OK);
    if(ret == -1) {
        printf("fifo not exist. create fifo.\n");
        ret = mkfifo(FIFO_PATH, 0777);
        if(ret == -1) {
            printf("mkfifo failed.");
	        exit(0);
        }
    }
    else {
        printf("fifo exists. Clearing file content.\n");
        fd = open(FIFO_PATH, O_WRONLY | O_TRUNC);
        if (fd < 0) {
            perror("open FIFO failed");
            exit(EXIT_FAILURE);
        }
        close(fd);
    }
    while(1) {
        fd = open(FIFO_PATH, O_RDONLY);
	    if(fd < 0)
	        printf("open FIFO dfe failed. \n");
        int i = 0;
	    //usleep(100);
	    int read_size = 0;  

        // while ((read_size = read(fd, cmd, 128)) > 0) 
        // {
        //     strcpy(cmd_dfe[i], cmd);
        //     printf("cmd %s \n", cmd_dfe[i]);
        //     i++;

        //     memset(cmd, 0, sizeof(cmd));
        // }

	    for(i=0; (read_size = read(fd, cmd, 64)); i++) {
	        //printf("read_size %i: %d\n", i, read_size);
	        //printf("cmd: %s\n", cmd);
	        strcpy(cmd_dfe[i], cmd);
	        //printf("cmd_dfe: %s\n", cmd_dfe[i]);
	        //memset(cmd, 0, 64);
	    }

        // printf("i = %d \n", i);
	    char *cmd_arr[i];
	    for(int j = 0; j < i; j++)
	        cmd_arr[j] = cmd_dfe[j];
	
        //call cmd func
        match_cmd(i, cmd_arr);

	    //printf("match_cmd call done.\n");
	    //usleep(1000);
	    close(fd);
    }
    return 0;
}


int intCmdtest()
{
    int ilRet = 0;

    ilRet = create_thread_pool(1 , 20);
    if(ilRet != 0)
    {
        puts("create_thread_pool() failed");
        return -1;
    }

    arg_struct_t* pTmp = (arg_struct_t *)malloc(sizeof(arg_struct_t));
    memset(pTmp , 0 , sizeof(arg_struct_t));

    //pTmp->cpriRate = 9;
    add_task(getIntCmd, pTmp);
    
	// temperature supervision task
	// add_task(tmp_supervision, pTmp);
	
    while(1){sleep(2);};
    //屏蔽子线程的alarm信号，避免打断sleep函数
    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset,SIGALRM);
    pthread_sigmask(SIG_SETMASK , &sigset , NULL);

    sleep(65);

    destroy_thread_pool();


    return 0;
}
