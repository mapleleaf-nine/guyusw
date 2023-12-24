#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "add.h"
#include "addCmd.h"
#include "cmdRegister.h"

void cmdAddInfoPrint()
{
    printf( "add set pram <a> <b>\n"
            "add get pram\n");
}

int8_t cmdArgCheckAdd(int argc, int num)
{
    if(argc < num)
    {
        printf("Input parameter number not matched! Please check it!\n");
        cmdAddInfoPrint();
        return -1;
    }
    return 0;
}

// ./intsw add set pram 1 2
void addSetCmd(int argc, char* argv[])
{
    if(cmdArgCheckAdd(argc, 4) < 0)
    {
        return;
    }

    char *cmd = argv[3];
    if(!strcmp(cmd, "pram"))
    {
        if(cmdArgCheckAdd(argc, 6) < 0)
        {
            return;
        }

        uint8_t a = strtol(argv[4], NULL, 0);
        uint8_t b = strtol(argv[5], NULL, 0);
        add(a, b);
    }
    else
    {
        printf("addSetCmd [%s] not found! please check it!\n", cmd);
        cmdAddInfoPrint();
    }
}

// ./intsw add get pram
void addGetCmd(int argc, char* argv[])
{
    if(cmdArgCheckAdd(argc, 4) < 0) 
    {
        return;
    }

    char *cmd = argv[3];
    if(!strcmp(cmd, "pram"))
    {
        printf("a + b \n");
    }
    else
    {
        printf("addGetCmd [%s] not found! please check it!\n", cmd);
        cmdAddInfoPrint();
    }
}

void addCmd(int argc, char *argv[])
{
    if(cmdArgCheckAdd(argc, 3) < 0)
    {
        return;
    }
    
    char *cmd = argv[2];
    if(!strcmp(cmd, "get"))
    {
        addGetCmd(argc, argv);
    }
    else if(!strcmp(cmd, "set"))
    {
        addSetCmd(argc, argv);
    }
    else
    {
        printf("addCmd [%s] not found! please check it!", cmd);
        cmdAddInfoPrint();
    }
}

void addCmdInit()
{
    cmd_t add_cmds = {"add", addCmd};
    register_cmds(&add_cmds, 1);
}