
#include <stdio.h>

extern int sharedInited;

int main(void) {
    int a = 255;
    swap(&a, &sharedInited);

    return 0;
}

