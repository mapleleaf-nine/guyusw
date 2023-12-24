#include <stdio.h>
#include <stdint.h>
#include "add.h"
#include "addCmd.h"
#include "sub.h"
#include "subCmd.h"
#include "interface.h"
#include "cmdRegister.h"

calcInterface_t calcInterface;

void intInterface()
{
    calcInterface = initCalcInterface(INTERFACE_CALC);
}

void calcInit()
{
    calcInterface.calcAddFunc(1, 3);
    calcInterface.calcSubFunc(5, 2);
}

void initModules()
{
    intInterface();
    // calcInit();
    addCmdInit();

    subCmdInit();
}