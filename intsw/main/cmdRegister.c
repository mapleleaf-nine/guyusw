#include <stdio.h>
#include <string.h>
#include "cmdRegister.h"


//the commands by user registed
static cmd_list_t commands = {0};

//Function description:
//match the cmd operation and run it
//Function description end.
//Parameter description: 
//parameter0: argc    ->    count of arg param
//parameter1: argv    ->    the arg list which input by userssss
//Parameter description end.
//return description: 
//return: value    ->    0:successful  -1:failed
//return description end.
int8_t match_cmd(int argc, char* argv[])
{
    int i;
    uint8_t matched = 0;

    if(strlen(argv[1]) > MAX_CMD_NAME_LENGTH)
    {
        printf( "cmd name is too long\n");
        return -1;
    }

    for(i=0; i<commands.num; i++)
    {
        if(strcmp(commands.cmds[i].cmd_name, argv[1]) == 0)
        {
            matched = 1;
            commands.cmds[i].cmd_operate(argc, argv);
        }
    }

    if(matched == 0)
    {
        printf( "cmd not found\n");
        return -1;
    }

    return 0;
}

//Function description:
//match the cmd operation and run it
//Function description end.
//Parameter description: 
//parameter0: reg_cmds    ->    the cmds who want to regist
//parameter1: length      ->    the cmds length who want to regist
//Parameter description end.
//return description: 
//return: value    ->    0:successful  -1:failed
//return description end.
int8_t register_cmds(cmd_t reg_cmds[], int length)
{
    int i;

    if (commands.num + length > MAX_CMDS_COUNT)
    {
        printf( "cmd cannot add, the cmd list length is not enough\n");
        return -1;
    }

    for (i = 0; i < length; i++)
    {
        if (commands.num < MAX_CMDS_COUNT)
        {
            strcpy(commands.cmds[commands.num].cmd_name, reg_cmds[i].cmd_name);
            printf("commands.cmds[%d].cmd_name: %s \n", commands.num, commands.cmds[commands.num].cmd_name);
            commands.cmds[commands.num].cmd_operate = reg_cmds[i].cmd_operate;
            commands.num++;
        }
    }
    return 0;
}