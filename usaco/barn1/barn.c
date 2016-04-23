#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define MAXSTALL 200
int hascow[MAXSTALL];

int
intcmp(const void *va, const void *vb)
{
    return *(int*)vb - *(int*)va;
}

void
main(void)
{
    FILE *fin, *fout;
    int n, m, nstall, ncow, i, j, c, lo, hi, nrun;
    int run[MAXSTALL];

    fin = fopen("barn1.in", "r");
    fout = fopen("barn1.out", "w");
    
    assert(fin != NULL && fout != NULL);

    fscanf(fin, "%d %d %d", &m, &nstall, &ncow);
    for(i = 0; i < ncow; i++) {
        fscanf(fin, "%d", &c);
        hascow[c-1] = 1;
    }

    n = 0;  /* answer: no. of uncovered stalls */

    /* count empty stalls on left */
    for(i = 0; i < nstall && !hascow[i]; i++)
        n++;
    
    lo = i; //真正的位置，而不是从0开始计数

    /* count empty stalls on right */
    for(i= nstall-1; i >= 0 && !hascow[i]; i--)
        n++;
    
    hi = i+1;

    /* count runs of empty stalls */
    nrun = 0;
    i = lo; //这个位置有牛  
    while(i < hi) {
        while(hascow[i] && i < hi) //发现i位置没有牛就退出
            i++;

        for(j = i; j < hi && !hascow[j]; j++) //退出循环时，j的位置是有牛的。
            ;

        run[nrun++] = j-i; //计算多少个栏里没有牛
        i = j;
    }

    /* sort list of runs */
    qsort(run, nrun, sizeof(run[0]), intcmp); //快排函数的使用 

    /* uncover best m-1 runs */
    for(i = 0; i < nrun && i < m-1; i++)
        n += run[i];

    fprintf(fout, "%d\n", nstall-n);
    exit(0);
}
