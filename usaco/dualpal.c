/*
 ID:lxlenovos1
 LANG:C
 TASK:dualpal
 */
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

char palindromes[50];
char result[50];
char map[256];


int 
get_palindromes(int base, int seq)
{
		int i, num, remainder;

		if (base < 2 || base > 20)
				return -1;
		
		for (i = 0; ; ++i){
			num = seq / base;
			remainder = seq % base;
			if (remainder >= 10)
				palindromes[i] = map[remainder - 10 + '0'];
			else
				palindromes[i] = remainder + '0';

			seq = num;

			if (seq == 0)
					break;
		}

		return (i + 1);
}

int 
check_palindromes(int number)
{
	int i; 

	for (i = 0; i < number / 2; ++i)
	{
			if (palindromes[i] == palindromes[number - i - 1]){
				continue;			
			}
			else{
				return -1;
			}
	}
	return 0;
}

int
main(){
	FILE *fin, *fout;
	int num, start;
	int i, j, k, count;
	int result = 0;

	map['0'] = 'A';
	map['1'] = 'B';
	map['2'] = 'C';
	map['3'] = 'D';
	map['4'] = 'E';
	map['5'] = 'F';
	map['6'] = 'G';
	map['7'] = 'H';
	map['8'] = 'I';
	map['9'] = 'J';

	fin = fopen("dualpal.in", "r");
	fout = fopen("dualpal.out", "w");

	fscanf(fin, "%d %d", &num, &start);

	for (i = start+1; ; ++i) {
		count = 0;
		for (j = 2; j <= 10; ++j) {
				k = get_palindromes(j, i);
				if (k > 0 && check_palindromes(k) == 0){
							++count;
				}
		}
		if (count >= 2){
			fprintf(fout, "%d\n", i);
			++result;

			if (result >= num)
					break;
		}
	}

	fclose(fin);
	fclose(fout);
	exit(0);
}
