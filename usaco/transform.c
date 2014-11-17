/*
 *ID:lxlenovos1
 *LANG:C
 *TASK:transform
 **/
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

int num;
char test[10][10];

/*
 * 返回选择90度的图形
 **/
void transform_90(char (*t_src)[10])
{
		int i, j;
		char tmp[10][10];

		for (i = 0; i < num; ++i){
			printf("src is %s\n", (t_src + i));
			strncpy(tmp[0] + i, (const char *)(t_src + i), num);
			printf("tmp is %s\n", tmp[0] + i);
		}	

		for (i = 0; i < num; ++i)
			for (j = 0; i < num; ++j){
				printf("%c ", tmp[i][j]);

				if (j == (num - 1))
						printf("\n");
			}

		/*for (i = 0; i < num; ++i)
				for (j = num - 1; j > 0; --j){
					printf("char is %c and j is %d, i is %d\n", tmp[j][i], j, i);
					test[j][i] = tmp[j][i];
				}
		
		for (i = 0; i < num; ++i){
			//strncpy((test[0] + i), (const char *)(tmp[0] + i), num);
			printf("dst_90 is %s\n", test[0] + i);
		}	*/
}


int cmp(char (*cmp_a)[10], char (*cmp_b)[10])
{
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
			strcpy((char*)(src + i), tmp_in);
			//printf("tmp_in is %s\n", tmp_in);		
			//printf("src num is %s\n", src + i);		
		}
		
		for(i = 0; i < num; ++i){
			fscanf(fin, "%s", &tmp_in);
			strcpy((char*)(dst + i), tmp_in);
			//printf("tmp_in is %s\n", tmp_in);		
			//printf("src num is %s\n", dst + i);		
		}

		for(i = 0; i < 7; ++i){
				switch(i){
						case 0:
							transform_90(&src[0]);
							break;
						case 1:
							//transform_180();
							break;
						case 2:
							//transform_270();
							break;
						case 3:
							//transform_inver();
							break;
						case 4:
							//transform_comb();
							break;
						case 5:
							//transform_nothing();
							break;
						default:
							break;
				}
		}

		exit(0);
}
