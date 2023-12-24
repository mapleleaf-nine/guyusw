#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "sub.h"
#include "subCmd.h"
#include "cmdRegister.h"

void cmdSubInfoPrint()
{
    printf( "sub set pram <a> <b>\n"
            "sub get pram\n");
}

int8_t cmdArgCheckSub(int argc, int num)
{
    if(argc < num)
    {
        printf("Input parameter number not matched! Please check it!\n");
        cmdSubInfoPrint();
        return -1;
    }
    return 0;
}

// ./intsw sub set pram 1 2
void subSetCmd(int argc, char* argv[])
{
    if(cmdArgCheckSub(argc, 4) < 0)
    {
        return;
    }

    char *cmd = argv[3];
    if(!strcmp(cmd, "pram"))
    {
        if(cmdArgCheckSub(argc, 6) < 0)
        {
            return;
        }

        uint8_t a = strtol(argv[4], NULL, 0);
        uint8_t b = strtol(argv[5], NULL, 0);
        sub(a, b);
    }
    else
    {
        printf("subSetCmd [%s] not found! please check it!\n", cmd);
        cmdSubInfoPrint();
    }
}

// ./intsw sub get pram
void subGetCmd(int argc, char* argv[])
{
    if(cmdArgCheckSub(argc, 4) < 0) 
    {
        return;
    }

    char *cmd = argv[3];
    if(!strcmp(cmd, "pram"))
    {
        printf("a - b \n");
    }
    else
    {
        printf("subGetCmd [%s] not found! please check it!\n", cmd);
        cmdSubInfoPrint();
    }
}

void subCmd(int argc, char *argv[])
{
    if(cmdArgCheckSub(argc, 3) < 0)
    {
        return;
    }
    
    char *cmd = argv[2];
    if(!strcmp(cmd, "get"))
    {
        subGetCmd(argc, argv);
    }
    else if(!strcmp(cmd, "set"))
    {
        subSetCmd(argc, argv);
    }
    else
    {
        printf("subCmd [%s] not found! please check it!", cmd);
        cmdSubInfoPrint();
    }
}

void subCmdInit()
{
    cmd_t sub_cmds = {"sub", subCmd};
    register_cmds(&sub_cmds, 1);
}