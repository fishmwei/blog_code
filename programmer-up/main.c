
#include <stdio.h>
#include "common.h"

int main(void) {
    int a = 255;

    getchar();
    swap(&a, &sharedInited);
    printf("exec main end\r\n");

    return 0;
}

