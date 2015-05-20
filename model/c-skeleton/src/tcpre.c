#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "lcthw/dbg.h"
#include "lcthw/hashmap.h"

#define SIZE 1024
#define PACKET_LEN 1460
#define BUCKETS_LEN 997 
Hashmap *map = NULL;
unsigned long index_key[BUCKETS_LEN * 10];
static int index = 0;

//struct tagbstring test1 = bsStatic("test data 1");
struct tagbstring expect = bsStatic("Hi");

// Compute hash for a string.
unsigned long 
hash(char *key, int M, int R, long Q) {
	int j;
	unsigned long h = 0;

  	for (j = 0; j < M; j++) {
		h = (R * h + key[j]) % Q;
		//printf("key is %d\n", key[j]);
		//printf("h is %ld and %ld    and %ld\n", h, LONG_MAX, (h - LONG_MAX));
		//printf("%lu\n", h);
	}	
  	return h;
}

/*
 * choose 1460 or the other bytes calculate their hash values.
 */
int  
calculate_hash(char *playload, int palyload_len, long Q, int R, long RM, int zero_value, int chunk_num)
{
	int i;
	unsigned long *key = NULL;
	unsigned long txthash = hash(playload, chunk_num, R, Q);

	if ((txthash & zero_value) == 0) {
		//insert the hashmap.
		//key = (unsigned long *)malloc(unsigned long);
		//*key = txthash;
		printf("txthash is %lu and key is %lu\n", txthash, *key);
		
		if (index < BUCKETS_LEN*10) {
			index_key[index] = txthash;
			int rc = Hashmap_set(map, (index_key+index), &expect);
 	    	check(rc == 0, "Failed to set hashmap");	
			++index;	
		}	
	}

	for (i = chunk_num; i < palyload_len; i++) {
 		txtHash = (txtHash + Q - RM*playload[i-chunk_num] % Q) % Q;
        txtHash = (txtHash*R + playload[i]) % Q;
  
		if ((txtHash & zero_value) == 0) {
			//判断是否自身有相同的指纹
			//直接把hash值弄进去看看 
			bstring result = Hashmap_get(map, );
			if (hashmap.get(txtHash>>zero_num) != null)
				continue;
			hashmap.put(txtHash>>zero_num, "source");

			 //hashmap.put(txtHash>>zero_num, txt.substring(i - chunk_num + 1, i));
			 //StdOut.println("old is:" + (txtHash>>zero_num));
			 //StdOut.println("str is: " + txt.substring(i - chunk_num + 1, i));
		}
	}

	return 0;

	error:
		--index;
		return -1;
}

int
main()
{
	FILE *input;
	int chunk_num = 64;   // divide the string into chunk
	int zero_num = 0;    // the number of bits which equals 0
  	int zero_value = 1;
 	int remedy = 12; 
	char *source = NULL;
	char *data = NULL;
	char *temp = NULL;
	int copy_len = 0;
	Hashmap *map = NULL;
	long Q = 1;
	int i = 0;
	char *packet = NULL;
	int R = 1048583;
    long RM = 1;

	// precalculate
	for (i = 0; i < 60; ++i)
    	Q = (2 * Q);

	for (i = 0; i < zero_num; ++i)
    	zero_value = (2 * zero_value);
 	zero_value = zero_value - 1;
	
    // precompute R^(M-1) % Q for use in removing leading digit
    for (int i = 1; i <= M-1; i++)
    	RM = (R * RM) % Q;

	// bulid a hash map
	Hashmap_create(BUCKETS_LEN, NULL, NULL);		

	// read the file
	input = fopen("/root/c-skeleton/bin/history.txt", "r");
	check((input != NULL), "Can't find the file.");

	source = (char *)malloc(SIZE);
	check((source != NULL), "Can't alloc merrory.");
	
	data = (char *)malloc(SIZE);
	check((data != NULL), "Can't alloc merrory.");
	
	temp = (char *)malloc(SIZE);
	check((temp != NULL), "Can't alloc merrory.");
	
	packet = (char *)malloc(PACKET_LEN);
	check((packet != NULL), "Can't alloc merrory.");
	
	while(fgets(source, SIZE, input) != NULL){
		// First: delete the '\n'
		int len = strlen(source);
		strncpy(temp, source, len-1);
		temp[len-1] = '\0';
	
		// Second: copy and calculate the hash value 	
		len = strlen(temp);
		// Fixup: maybe len > PACKET_LEN
		strncpy(packet + copy_len, temp, len);
		copy_len += len;
	
		if (copy_len >= chunk_num) {
			//calculate the hash
			calculate_hash(packet, copy_len, Q, R, RM, zero_value, chunk_num);			
			copy_len = 0;
		}	
	}

	fclose(input);
	free(source);
	free(data);
	free(temp);
	free(packet);
	return 0;

    error:
		free(source);
		free(data);
		free(temp);
		return -1;
}
