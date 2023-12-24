#include <stdio.h>
#include <stdint.h>
#include "add.h"

int add(int a, int b){
    printf("add: %d\n", a + b);
    return (a + b);
}