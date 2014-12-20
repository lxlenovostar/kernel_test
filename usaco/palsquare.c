/*
 ID:lxlenovos1
 LANG:C
 TASK:palsquare
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

void 
get_result(int base, int seq)
{
		int i, j, num, remainder;
		char tmp[50];
		int length = 0;

		for (i = 0; ; ++i){
			num = seq / base;
			remainder = seq % base;
			if (remainder >= 10)
				tmp[i] = map[remainder - 10 + '0'];
			else
				tmp[i] = remainder + '0';

			seq = num;

			if (seq == 0)
					break;
		}

		length = i;

		for (j = 0; j < length + 1; ++j, --i)
				result[j] = tmp[i];
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
	int base, i, square, num;

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

	fin = fopen("palsquare.in", "r");
	fout = fopen("palsquare.out", "w");

	fscanf(fin, "%d", &base);

	for (i = 1; i <= 300; ++i){
		square = i * i;
		
		num = get_palindromes(base, square);
		if (num > 0){
			if (check_palindromes(num) == 0){
					get_result(base, i);
					fprintf(fout, "%s %s\n", result, palindromes);
			}
		}
	}

	fclose(fin);
	fclose(fout);
	exit(0);
}
