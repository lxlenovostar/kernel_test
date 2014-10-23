#include <stdio.h>
#include <stdlib.h>

#define NUM 3 

int 
main()
{
		int num, i,j, low, high, long_distance, intevel;
		int data[NUM][2];
		printf("pleas input data\n");
		scanf("%d", &num);

		for (i = 0; i < NUM; ++i)
			for (j = 0; j < 2; ++j){
				scanf("%d", &data[i][j]);
			}

		for (i = 0; i < NUM; ++i)
			for (j = 0; j < 2; ++j){
				if (i == 0 && j == 0){
					low = data[i][j];
					high = data[i][j+1];
					long_distance = high - low;
					intevel = 0;
				}
				else if (i != 0 && j == 0){
					if (low > data[i][j] && (long_distance < (data[i][j+1] - data[i][j]))){
						low = data[i][j];	
						high = data[i][j+1];
						long_distance = high - low;	
					}
					else if (high > data[i][j] && high < data[i][j+1]){
						high = data[i][j+1];
						long_distance = high - low;	
					}
					else if (high < data[i][j] && (data[i][j] - high > intevel)){
						intevel = data[i][j] - high;
					}
				} 
			}
	
		printf("long_distance is %d; intevel is %d\n", long_distance, intevel);
	
}
