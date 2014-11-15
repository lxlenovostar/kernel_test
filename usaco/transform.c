/*
 *ID:lxlenovos1
 *LANG:C
 *TASK:transform
 **/
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

int num;

int transform_90(char (*t_src)[10], char (*t_dst)[10])
{
		int i;
		char tmp[10][10];

		for (i = 0; i < num; ++i){
			strnpy(tmp[0] + i, (t_sec + i), num);
			printf("tmp is %s\n", tmp[0] + i);
		}	




		return 0;
}

int 
main()
{
		FILE *fin,*fout;
		char src[10][10];
		char dst[10][10];
		char tmp_in[10];
		int i, j;

		fin = fopen("transform.in","r");
		fout = fopen("transform.out","w");

		fscanf(fin, "%d", &num);
		
		for(i = 0; i < num; ++i){
			fscanf(fin, "%s", &tmp_in);
			strcpy((char*)(src + num), tmp_in);
			printf("tmp_in is %s\n", tmp_in);		
			printf("src num is %s\n", src + num);		
		}
		
		for(i = 0; i < num; ++i){
			fscanf(fin, "%s", &tmp_in);
			strcpy((char*)(dst + num), tmp_in);
			printf("tmp_in is %s\n", tmp_in);		
			printf("src num is %s\n", dst + num);		
		}

		for(i = 0; i < 7; ++i){
				switch(i){
						case 0:
							if (transform_90(src, dst) == 0){
								
							}
							break;
						case 1:
							transform_180();
							break;
						case 2:
							transform_270();
							break;
						case 3:
							transform_inver();
							break;
						case 4:
							transform_comb();
							break;
						case 5:
							transform_nothing();
							break;
						default:
							break;
				}
		}

		exit(0);
}
