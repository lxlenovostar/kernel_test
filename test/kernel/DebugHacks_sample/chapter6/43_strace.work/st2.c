#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    while(1) {
        FILE *fp;
        fp = fopen("/etc/shadow", "r");
        if (fp == NULL)
            printf("Error!\n");
        else
            close(fp);
        sleep(3);
    }
    return EXIT_SUCCESS;
}

