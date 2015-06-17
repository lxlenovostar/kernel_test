/*
 *  验证如何将数据包的数据负载部分按大约500字节进行切分
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 30000
#define PACKET_LEN 30000

static int count = 0;

// Compute hash for a string.
unsigned long
hash(char *key, int M, int R, long Q)
{
	int j;
	unsigned long h = 0;

	for (j = 0; j < M; j++) {
		h = (R * h + key[j]) % Q;
	}
	return h;
}

int
calculate_hash(char *playload, int playload_len, long Q, int R, long RM,
	       int zero_value, int chunk_num)
{
	long i;
	int delay_time = 0;

	unsigned long txthash = hash(playload, chunk_num, R, Q);

	printf("%d|%d|", count++,playload_len);
	if ((txthash & zero_value) == 0) {
		printf("0 ");
	}

	for (i = chunk_num; i < playload_len; i++) {
		txthash = (txthash + Q - RM * playload[i - chunk_num] % Q) % Q;
		txthash = (txthash * R + playload[i]) % Q;
		
		if ((txthash & zero_value) == 0) {
			printf("%d ", i);
		}
	}
	printf("\n");

	return 0;

      error:
	return -1;
}

int
read_file(char *filename, long Q, int R, long RM, int zero_value, int chunk_num)
{
    FILE *input;
    char *source = NULL;
    char *temp = NULL;
    char *packet = NULL;
    int copy_len = 0;
    int rc; 
	int j = 0;

    // read the file
    input = fopen(filename, "r");
    //check((input != NULL), "Can't find the file.");

    source = (char *) malloc(SIZE);
    //check((source != NULL), "Can't alloc merrory.");

    temp = (char *) malloc(SIZE);
    //check((temp != NULL), "Can't alloc merrory.");

    packet = (char *) malloc(SIZE);
    //check((packet != NULL), "Can't alloc merrory.");

    while (fgets(source, SIZE, input) != NULL) {
        // First: delete the '\n'
        int len = strlen(source);
        strncpy(temp, source, len - 1); 
        temp[len - 1] = '\0';

        // Second: copy and calculate the hash value    
        // 30000 is the lagest pack playload. 
        memcpy(packet + copy_len, temp, strlen(temp));
		copy_len += strlen(temp);
		packet[copy_len] = '\0';

        if (copy_len >= chunk_num) {
            //calculate the hash
            rc = calculate_hash(packet, copy_len, Q, R, RM, 
                        zero_value, chunk_num);
            //check(rc == 0, "calculate hash maybe error.");
        	copy_len = 0;
        }   
	}

    fclose(input);
    free(source);
    free(temp);
    free(packet);
    return 0;

      error:
    fclose(input);
    free(source);
    free(temp);
    free(packet);
    return -1;
}

int
main()
{
	long Q = 1;
	int i = 0;
	int R = 1048583;
	//int R = 10;
	long RM = 1;
	int chunk_num = 48;
	int zero_num = 10; 
	int zero_value = 1;
	int rc = 0;
	char source[1024] = "aa";
	
	// precalculate
    for (i = 0; i < 60; ++i)
        Q = (2 * Q);

	for (i = 1; i <= chunk_num - 1; i++)
    	RM = (R * RM) % Q;
	
	for (i = 0; i < zero_num; ++i)
		zero_value = (2 * zero_value);
	zero_value = zero_value - 1;

	rc = read_file(source, Q, R, RM, zero_value, chunk_num);
    //check(rc == 0, "Failed to calculate hash.");

}
