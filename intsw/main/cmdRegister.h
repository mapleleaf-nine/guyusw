#ifndef _CMD_REGISTER_H_
#define _CMD_REGISTER_H_

#include <stdint.h>

#define MAX_CMD_NAME_LENGTH 50
#define MAX_CMDS_COUNT 100

//type description:
//the common type of function
//type description end.
//parameter description:
//parameter0: argc     ->  count of arg param
//parameter1: argv     ->  the arg list which input by users
//parameter description end.
typedef void (*cmdHandler)(int argc, char* argv[]);

//type description:
// cmd - name pair
//type description end.
//parameter description:
//parameter0: cmd_name     ->  the name of cmd which registed by each models
//parameter1: cmd_operate  ->  the operate of cmd name, registed by each models
//parameter description end.
typedef struct cmd
{
    char cmd_name[MAX_CMD_NAME_LENGTH + 1];
    cmdHandler cmd_operate;
} cmd_t;

//type description:
// cmd - name pair list, save all cmd-name pair
//type description end.
//parameter description:
//parameter0: cmds     ->  the list of cmd-name pair
//parameter1: num      ->  the num of list
//parameter description end.
typedef struct cmd_list
{
    cmd_t cmds[MAX_CMDS_COUNT];
    int num;
} cmd_list_t;

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
int8_t match_cmd(int argc, char* argv[]);

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
int8_t register_cmds(cmd_t reg_cmds[], int length);

#endif /*_CMD_REGISTER_H_*/