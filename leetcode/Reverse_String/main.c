/*
 * Reverse String
 */
#include <stdio.h>
#include <stdlib.h>

char* reverseString(char* s) {
    int i; 
    int length = 0;
    char tmp;

    for (i = 0; *(s+i) != '\0'; i++)
        length++;

    printf("length is:%d\n", length);

    for (i = 0; i < (length/2); i++) {
        printf("s+i is:%c\n", *(s + i));
        printf("s+length-1-i is:%c\n", *(s + length - 1 - i));
        tmp = *(s + i);
        printf("what1\n");
        //*(s + i) = *(s + length - 1 - i);
        *(s + i) = 'c';
        printf("what2\n");
        *(s + length - 1 - i) = tmp;
        printf("what3\n");
    }

    return s;
}

int main()
{
    char *s = "abc";
    printf("%s\n", reverseString(s));
}
