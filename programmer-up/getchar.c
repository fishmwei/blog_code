#include <stdio.h>

int main() {
    int input;

    printf("enter a letter:\r\n");
    do {
        
        input = getchar();
        if ((input >= 'a' && input <= 'z') || (input >= 'A' && input <= 'Z')) {
            printf("get letter %c\r\n", input);
            printf("enter a letter:\r\n");
        } else {
            
        }
    } while (input != 'q');

    return 0;
}