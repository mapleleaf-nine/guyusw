#ifndef _INTERFACE_H_
#define _INTERFACE_H_

#include <stdint.h>

typedef enum
{
    INTERFACE_CALC,
    INTERFACE_UNKNOWN
}calcInterface_e;

typedef int (*calcAddFuncPtr)(int a, int b);

typedef int (*calcSubFuncPtr)(int a, int b);

typedef struct
{
    calcAddFuncPtr calcAddFunc;
    calcSubFuncPtr calcSubFunc;
}calcInterface_t;

calcInterface_t initCalcInterface(calcInterface_e type);

#endif /* _INTERFACE_H_ */