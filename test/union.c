#include <stdio.h>

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned long uint32_t;

typedef union {
    uint8_t rawdata;
    //struct {
    #if __BYTE_ORDER == __LITTLE_ENDIAN
    uint8_t bit0: 1,
            bit1: 1,
            bit2: 1,
            bit3: 1,
            bit4: 1,
            bit5: 1,
            bit6: 1,
            bit7: 1;
    #else

    uint8_t bit7: 1,
            bit6: 1,
            bit5: 1,
            bit4: 1,
            bit3: 1,
            bit2: 1,
            bit1: 1,
            bit0: 1;
    #endif
    //};

}__attribute__((packed)) data_t;

int main(void) 
{   
    data_t data;
    data.rawdata = 0x02;

    printf("size is %d\r\n", sizeof(data));
    printf("bit0 is %d\r\n", data.bit0);
    printf("bit1 is %d\r\n", data.bit1);
    printf("bit2 is %d\r\n", data.bit2);
    printf("bit3 is %d\r\n", data.bit3);
    printf("bit4 is %d\r\n", data.bit4);
    printf("bit5 is %d\r\n", data.bit5);
    printf("bit6 is %d\r\n", data.bit6);
    printf("bit7 is %d\r\n", data.bit7);

    return 0;
}