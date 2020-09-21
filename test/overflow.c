#include <stdio.h>

typedef union src_s {
    unsigned int rawdata;
    struct {
        unsigned int  type: 1,                    
                    id: 4,
                    index: 27;
                    
    };
} src_t;

typedef union dst_s {
    unsigned int rawdata[2];
    struct {
        unsigned : 28;
        unsigned high: 4;
        unsigned int data;
    };
} dst_t;

int main(void)
{
    src_t data;
    data.type = 0;
    data.id = 0;
    data.index = 67108864; // 最高位当作符号位了?
#if 0
    printf("base rawdata 0x%x index %x\r\n", data.rawdata, data.index);
    //unsigned long long result = ((unsigned long long)data.index << 5) + 9;

    //unsigned  temp = data.index << 5;
    
    unsigned long long result = (data.index << 5) + 9;
    printf("result  %llx %llu high32 %llx \r\n", result, result, result>>32);

    dst_t dst;
    dst.data = result;
    dst.high = result>>32;
    printf("result [0] 0x%x [1] 0x%x\r\n", dst.rawdata[0], dst.rawdata[1]);

#endif
    unsigned long a = (unsigned long)data.index << 5;
    printf("a is 0x%lx\r\n", a);


    return 0;
}