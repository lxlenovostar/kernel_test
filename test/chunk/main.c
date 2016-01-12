#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int step = 32;
int chunk_num = 32;	
unsigned long RM = 1;
unsigned long zero_value = 1;
int zero_num = 6;
unsigned long Q = 1;
unsigned long R = 1048583;

void init_hash_parameters(void)
{
	int i;
	
	// precalculate
	for (i = 0; i < 60; ++i)
		Q = (2 * Q);

	for (i = 1; i <= chunk_num - 1; i++)
		RM = (R * RM) % Q;

	for (i = 0; i < zero_num; ++i)
        zero_value = (2 * zero_value);
    zero_value = zero_value - 1;
}

/*
 * Compute hash for a string.
 */
unsigned long pack_hash(char *key, int M, int R, long Q)
{
    int j;
    unsigned long h = 0;

    for (j = 0; j < M; j++) {
        h = (R * h + key[j]) % Q;
    }
	return h;
}

void calculate_partition(char *playload, int playload_len) 
{
	int i;
	int delay_time = 0;
	unsigned long txthash = pack_hash(playload, chunk_num, R, Q);

	printf("begin partition; hash value is:%lu.\n", txthash);

	if ((txthash & zero_value) == 0) {
		int pos = chunk_num - 1;
		printf("pos1 is:%d->%lu.\n", pos, txthash);
		delay_time = step;
	}

	for (i = chunk_num; i < playload_len; i++) {
		txthash = (txthash + Q - RM * playload[i - chunk_num] % Q) % Q;
		txthash = (txthash * R + playload[i]) % Q;

		if (delay_time == 0) {
			if ((txthash & zero_value) == 0) {
				printf("pos2 is:%d->%lu.\n", i, txthash);
				delay_time = step;
			}
		} else {
			delay_time--;
		}
	}
}

void main()
{
    int i;
    char *data = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa$01234567890123456789$aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
    init_hash_parameters();
    calculate_partition(data, strlen(data));
    printf("hello world! len is:%d\n", strlen(data));

    for (i = 0; i < strlen(data); i++) {
        if (data[i] == '$')
            printf("$ pos is:%d\n", i);
    }
    calculate_partition(data+54, strlen(data) - 54);
}
