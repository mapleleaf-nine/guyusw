#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "intCmd.h"

#define FIFO_PATH "../fifoPath/cmd.fifo"    //有名管道 FIFO文件

int main(int argc, char* argv[])
{
    int ret = 0;
    int fd = open(FIFO_PATH, O_WRONLY);
    if(fd < 0){
        perror("open failed.");
        exit(0);
    }

    for(int i = 1; i < argc; i++){
	    ret = write(fd, argv[i], strlen(argv[i]) + 1);
        if(ret < 0)
        {
            printf("write error \n");
        }
        // printf("argv[%d]: %s \n", i, argv[i]);
        usleep(100);    //延时保证FIFO 命令写入文件，且线程能读取完整命令
    }
    // fsync(fd); // 刷新缓冲区

    // usleep(1000);
    //printf("close write fd.\n");  

    close(fd);

    return 0;
}

