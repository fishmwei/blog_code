#include "common.h"
#include <stdio.h>

int sharedInited = 1;
int sharedUndef;

void swap(int *a, int *b) {

    *a ^= *b ^= *a ^= *b;
}