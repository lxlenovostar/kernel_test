/*
ID:lxlenovos1
LANG:C
TASK:combo
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

int john_set[3];
int master_set[3];
int num_digits;

int check_result(int *check_value) {
    int i; 
    int flag = 0;

    for (i = 0; i < 3; i++) {
         if (abs(john_set[i] - *(check_value + i)) <= 2 || ((john_set[i]%num_digits + *(check_value + i)%num_digits) <= 2) || ((john_set[i]%num_digits + (num_digits - *(check_value + i))%num_digits) <= 2) || (( (num_digits - john_set[i])%num_digits + *(check_value + i)%num_digits) <= 2) || (((num_digits - john_set[i])%num_digits + (num_digits -*(check_value + i))%num_digits) <= 2)) {
            ++flag;
        } 
    }

    if (flag == 3) 
        return 1;
    else 
        flag = 0; 
 
    for (i = 0; i < 3; i++) {
         if (abs(master_set[i] - *(check_value + i)) <= 2 || ((master_set[i]%num_digits + *(check_value + i)%num_digits) <= 2) || ((master_set[i]%num_digits + (num_digits - *(check_value + i))%num_digits) <= 2) || (( (num_digits - master_set[i])%num_digits + *(check_value + i)%num_digits) <= 2) || (((num_digits - master_set[i])%num_digits + (num_digits -*(check_value + i))%num_digits) <= 2)) {
            ++flag;
        } 
    }

    if (flag == 3) 
        return 1;
    else 
        return 0; 
}

void main(void)
{
    FILE *fin, *fout;
    int i, j, k;
    int result = 0;
    int tmp[3];

    fin = fopen("combo.in", "r");
    fout = fopen("combo.out", "w");
    
    assert(fin != NULL && fout != NULL);

    fscanf(fin, "%d", &num_digits);

    //printf("num_digits is:%d\n", num_digits);

    for(i = 0; i < 3; i++) {
        fscanf(fin, "%d", (john_set + i));
        //printf("john is:%d\n", *(john_set + i));
    }

    for(i = 0; i < 3; i++) {
        fscanf(fin, "%d", (master_set + i));
        //printf("master is:%d\n", *(master_set + i));
    }
    
    //printf("\n");

    for (i = 1; i <= num_digits; i++)
        for (j = 1; j <= num_digits; j++)
            for (k = 1; k <= num_digits; k++) {
                tmp[0] = i, tmp[1] = j, tmp[2] = k;
 
                if (check_result(tmp)) {
                    //printf("%d:%d:%d\n", i, j, k);
                    result++;
                }
            } 

    fprintf(fout, "%d\n", result);
    exit(0);
}
