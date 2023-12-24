#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "interface.h"
#include "app.h"
#include "cmdRegister.h"
#include "testMain.h"

extern calcInterface_t calcInterface;

int main(int argc, char *argv[])
{

    initModules();

    // match_cmd(argc, argv);

    intCmdtest();
    
    return 0;
}