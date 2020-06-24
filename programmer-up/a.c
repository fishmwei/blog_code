
#include <stdio.h>

extern int sharedInited;

int main(void) {
    int a = 9;
    swap(&a, &sharedInited);

    return 0;
}

