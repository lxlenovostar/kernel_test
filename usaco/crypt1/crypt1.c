/*
ID:lxlenovos1
LANG:C
TASK:crypt1
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define NUM 9 
int num[NUM];

int check_value(int value, size_t num_digits, int *num, int limit) {
    int decimal = 10;
    int checkvalue, i;
    int flag;
    int value_len = 0;

    do {
        checkvalue = value%decimal;
        flag = 0;
    
        if (value_len > limit) 
            return 0;

        for (i = 0; i < num_digits; i++) {
            if (checkvalue == num[i]) {
                flag = 1;
                break;
            }
        }    
      
        if (!flag)
            return 0; 

        value = value/decimal;
        value_len++;
    } while(value!=0);  
 
    return 1; 
}

void main(void)
{
    FILE *fin, *fout;
    size_t num_digits;
    int i, j, k, l, m;
    int result = 0;
    int multiplier;
    int multiplicand;
    int p1, p2, value;

    fin = fopen("crypt1.in", "r");
    fout = fopen("crypt1.out", "w");
    
    assert(fin != NULL && fout != NULL);

    fscanf(fin, "%d", &num_digits);
    for(i = 0; i < num_digits; i++) {
        fscanf(fin, "%d", (num+i));
    }

    /*
     乘数的位数只有三位，那么循环3次。就可以创建被乘数
     被乘数两位，那么循环2次，就可以创建乘数
     时间复杂度是O(N的5次方)
     */ 
    for (i = 0; i < num_digits; i++)
        for (j = 0; j < num_digits; j++)
            for (k = 0; k < num_digits; k++) {
                multiplicand = num[i]*100 + num[j]*10 + num[k];    
                for (l = 0; l < num_digits; l++) { 
                    p1 = multiplicand * num[l];
                    if (!check_value(p1, num_digits, num, 2))
                        continue;
                    for (m = 0; m < num_digits; m++) {
                        p2 = multiplicand * num[m];
                        if (!check_value(p2, num_digits, num, 2))
                            continue;
                        
                        value = p1 + p2*10;
                        if (check_value(value, num_digits, num, 3)) {
                            //printf("p1 is:%d, p2 is:%d, value is:%d\n", p1, p2, value);
                            result++;
                        }
                    } 
                }
            }

    fprintf(fout, "%d\n", result);
    exit(0);
}
