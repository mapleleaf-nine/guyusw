#include <stdio.h>
#include <stdint.h>
#include "sub.h"

int sub(int a, int b){
    printf("sub: %d\n", a - b);
    return (a - b);
}