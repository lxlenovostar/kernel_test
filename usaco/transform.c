/*
 *ID:lxlenovos1
 *LANG:C
 *TASK:transform
 **/
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

int 
main()
{
		FILE *fin,*fout;
		char src[10][10];
		char dst[10][10];
		char tmp_in[10];
		int i, j, num;

		fin = fopen("transform.in","r");
		fout = fopen("transform.out","w");

		//scanf("%d", &num);
		fscanf(fin, "%d", &num);
		
		for(i = 0; i < num; ++i){
			//scanf("%d %d", &low, &high);
			fscanf(fin, "%s", &tmp_in);
			strcpy(src + num, tmp_in);
			printf("tmp_in is %s\n", tmp_in);		
			printf("src num is %s\n", src + num);		
		}

		exit(0);
}
