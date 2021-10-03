#include "common.h"

void showData(struct data *pdata) {
    printf("data is a = %d, b = %d \r\n", pdata->a, pdata->b);
}

//extern struct data *getData();

int main()
{
    struct data *ret = getData();
    printf("get ret is %p\r\n", ret);
    ret->a = 100;
    ret->b = 200;
    showData(ret);

    free(ret);


    return 0;    
}


#if 0

gcc -c getData.c -o getData.o
ar rc libget_data.a getData.o
gcc truncret.c libget_data.a -o main


gcc getData.c truncret.c -o a
#endif




