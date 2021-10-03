#include "common.h"

struct data *getData()
{
    struct data *ret = (struct data *)malloc(sizeof(struct data));
    printf("ret is %p\r\n", ret);
    return ret;
}