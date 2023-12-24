#include <stdio.h>
#include <stdint.h>
#include "interface.h"
#include "add.h"
#include "sub.h"

void calcInterfaceInit(calcInterface_t *calcInterface)
{
    calcInterface->calcAddFunc = add;
    calcInterface->calcSubFunc = sub;
}

void unknownInterfaceInit(calcInterface_t *calcInterface)
{
    calcInterface->calcAddFunc = NULL;
    calcInterface->calcSubFunc = NULL;
}

calcInterface_t initCalcInterface(calcInterface_e type)
{
    calcInterface_t calcInterface;
    switch(type)
    {
    case INTERFACE_CALC:
        calcInterfaceInit(&calcInterface);
        break;
    
    default:
        unknownInterfaceInit(&calcInterface);
        break;
    }
    return calcInterface;
}