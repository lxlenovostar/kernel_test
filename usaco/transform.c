/*
 ID:lxlenovos1
 LANG:C
 TASK:transform
 */
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

int num;
char test[10][10];

/*
 * 反射
 **/
void transform_reflect(char (*t_src)[10])
{
		int i, j, k;
		char tmp[10][10];
		
		for (i = 0; i < num; ++i){
			strncpy((char*)(tmp[i]), (const char *)(t_src + i), num);
		}	

		for (i = 0; i < num; ++i){
				k = num - 1;
				for (j = 0; j < num; ++j){
						test[i][j] = tmp[i][k];
						--k;
				}
		}

}

/*
 * 返回选择90度的图形
 **/
void transform_90(char (*t_src)[10])
{
		int i, j, k;
		char tmp[10][10];
		
		for (i = 0; i < num; ++i){
			strncpy((char*)(tmp[i]), (const char *)(t_src + i), num);
		}	

		/*	
		for (i = 0; i < num; ++i)
				for (j = 0; j < num; ++j)
				{
						printf("%c", tmp[i][j]);

						if (j == num - 1)
								printf("\n");
				}
		*/

		for (i = 0; i < num; ++i){
				k = num - 1;
				for (j = 0; j < num; ++j){
					test[i][j] = tmp[k][i];
					--k;
				}
		}
		
		/*
		printf("end\n");
		for (i = 0; i < num; ++i)
				for (j = 0; j < num; ++j)
				{
						printf("%c", test[i][j]);

						if (j == num - 1)
								printf("\n");
				}
		*/
}

/*
 * return 1 strand for not equal.
 * **/
int cmp(char (*cmp)[10], int num)
{
		int i;
		for (i = 0; i < num; ++i){
			if (strcmp(cmp[i], test[i]) != 0){
					return 1;
			}	
		}

		return 0;
}

int cmp_str(char (*cmp1)[10], char (*cmp2)[10], int num)
{
		int i;
		for (i = 0; i < num; ++i){
			if (strcmp(cmp1[i], cmp2[i]) != 0){
					return 1;
			}	
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
		int i;
		int flag = 0;

		fin = fopen("transform.in","r");
		fout = fopen("transform.out","w");

		fscanf(fin, "%d", &num);
		
		for(i = 0; i < num; ++i){
			fscanf(fin, "%s", tmp_in);
			strcpy((char*)(src + i), tmp_in);
			//printf("tmp_in is %s\n", tmp_in);		
			//printf("src num is %s\n", src + i);		
		}
		
		for(i = 0; i < num; ++i){
			fscanf(fin, "%s", tmp_in);
			strcpy((char*)(dst + i), tmp_in);
			//printf("tmp_in is %s\n", tmp_in);		
			//printf("src num is %s\n", dst + i);		
		}

		for(i = 0; i < 7; ++i){
				switch(i){
						case 0:
							transform_90(src);
							if (cmp(dst, num) == 0){
									//printf("1\n");
									fprintf(fout, "1\n");
									flag = 1;
							}
							break;
						case 1:
							transform_90(src);
							transform_90(test);
							if (cmp(dst, num) == 0){
									//printf("2\n");
									fprintf(fout, "2\n");
									flag = 1;
							}
							break;
							//transform_180();
						case 2:
							transform_90(src);
							transform_90(test);
							transform_90(test);
							if (cmp(dst, num) == 0){
									//printf("3\n");
									fprintf(fout, "3\n");
									flag = 1;
							}
							break;
							//transform_270();
						case 3:
							transform_reflect(src);
							if (cmp(dst, num) == 0){
									//printf("4\n");
									fprintf(fout, "4\n");
									flag = 1;
							}
							break;
							//transform_inver();
						case 4:
							transform_90(src);
							if (cmp(dst, num) == 0){
									//printf("5\n");
									fprintf(fout, "5\n");
									flag = 1;
							}
							else{
								transform_90(test);
								if (cmp(dst, num) == 0){
										//printf("5\n");
										fprintf(fout, "5\n");
										flag = 1;
								}
								else{
									transform_90(test);
									if (cmp(dst, num) == 0){
											//printf("5\n");
											fprintf(fout, "5\n");
											flag = 1;
									}
								}

							}
							break;
							//transform_comb();
						case 5:
								if(cmp_str(src, dst, num) == 0){
									//printf("6\n");
									fprintf(fout, "6\n");
									flag = 1;
								}
							//transform_nothing();
							break;
						default:
							break;
				}
				if (flag)
					break;
		}

		if (flag == 0){
			//printf("7\n");
			fprintf(fout, "7\n");
		}

		exit(0);
}
