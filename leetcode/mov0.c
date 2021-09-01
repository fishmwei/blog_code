#include <stdio.h>

void moveZeroes(int* nums, int numsSize){
    int pos=-1;
    int i = 0;
    for (i = 0; i < numsSize; i++) {
        if (nums[i] != 0) {
            if (pos != -1) {
                nums[pos++]=nums[i];
                nums[i]=0;
            }
        } else {
            if (pos == -1) {
                pos = i;
            }
        }
    }
}

int main()
{
    int nums[] = {1, 0, 2, 0,};
    moveZeroes(nums, sizeof(nums)/4);
    for (int i = 0; i < sizeof(nums)/4; i++) {
        printf("%d ", nums[i]);
    }   

    return 0;
}