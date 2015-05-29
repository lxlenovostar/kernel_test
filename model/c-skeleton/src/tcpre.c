#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "lcthw/dbg.h"
#include "lcthw/hashmap.h"

#define SIZE 1024
#define PACKET_LEN 1460
#define BUCKETS_LEN 997
#define NUM 150000
#define KEYNUM BUCKETS_LEN * NUM
#define REMEDY 12
Hashmap *map = NULL;
FILE *fp = NULL;
FILE *fpw = NULL;

static unsigned long hashmapnum = 0;
static int recount = 0;
static unsigned long sumbytes = 0;
unsigned long index_key[KEYNUM];
unsigned long stats[KEYNUM];
static int reindex = 0;

char expect[3] = "Hi";

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

/*
 * choose 1460 or the other bytes calculate their hash values.
 */
int
calculate_hash(char *playload, int palyload_len, long Q, int R, long RM,
	       int zero_value, int chunk_num)
{
	int i;
	unsigned long txthash = hash(playload, chunk_num, R, Q);
	if ((txthash & zero_value) == 0) {
		//insert the hashmap.

		//just for test
		char temphash[65];
		memcpy(temphash, playload, chunk_num);
		temphash[chunk_num] = '\0';
		fprintf(fpw, "%s %lu\n", temphash, txthash);

		if (Hashmap_get(map, &txthash)) {
			++recount;

			char temp[65];
			memcpy(temp, playload, chunk_num);
			temp[chunk_num] = '\0';
			fprintf(fp, "%s\n", temp);
		} else {
			if (reindex < KEYNUM) {
				index_key[reindex] = txthash;
				int rc = Hashmap_set(map, (index_key + reindex),
						     expect);
				check(rc == 0, "Failed to set hashmap");
				++reindex;

				++hashmapnum;
				stats[txthash % BUCKETS_LEN] += 1;
				/*
				   char temphash[65];
				   memcpy(temphash, playload, chunk_num);
				   temphash[chunk_num] = '\0';
				   fprintf(fpw, "%s %lu\n", temphash, txthash);
				 */
			}
		}
	}

	for (i = chunk_num; i < palyload_len; i++) {
		txthash = (txthash + Q - RM * playload[i - chunk_num] % Q) % Q;
		txthash = (txthash * R + playload[i]) % Q;

		if ((txthash & zero_value) == 0) {
			//check whether we have the same one.

			// just for test
			char temphash[65];
			memcpy(temphash, playload + i - chunk_num, chunk_num);
			temphash[chunk_num] = '\0';
			fprintf(fpw, "%s %lu\n", temphash, txthash);

			if (Hashmap_get(map, &txthash)) {
				++recount;

				char temp[65];
				memcpy(temp, playload + i - chunk_num,
				       chunk_num);
				temp[chunk_num] = '\0';
				fprintf(fp, "%s\n", temp);
				continue;
			}

			if (reindex < KEYNUM) {
				index_key[reindex] = txthash;
				int rc = Hashmap_set(map, (index_key + reindex),
						     expect);
				check(rc == 0, "Failed to set hashmap");

				++reindex;
				++hashmapnum;
				stats[txthash % BUCKETS_LEN] += 1;
				/*
				   char temphash[65];
				   memcpy(temphash, playload + i - chunk_num,
				   chunk_num);
				   temphash[chunk_num] = '\0';
				   fprintf(fpw, "%s %lu\n", temphash, txthash); */
			}
		}
	}

	return 0;

      error:
	return -1;
}

static uint32_t
Hashmap_mode_hash(void *data)
{
	uint32_t hash = (*(unsigned long *) data) % BUCKETS_LEN;
	//uint32_t hash = (*(unsigned long *) data) % 512;
	debug("hash is %d", hash);
	return hash;
}

static int
Hashmap_mode_compare(void *a, void *b)
{
	// 0 stands for equal.
	debug("a is %lu", *(unsigned long *) a);
	debug("b is %lu", *(unsigned long *) b);

	//if ( *(unsigned long *) a == 938095141181661967 || *(unsigned long *) b == 938095141181661967)
	//      printf(" a is %lu and b is %lu\n", *(unsigned long *) a, *(unsigned long *) b);

	return (*(unsigned long *) a - *(unsigned long *) b);
}

int
read_file(char *filename, long Q, int R, long RM, int zero_value, int chunk_num)
{
	FILE *input;
	char *source = NULL;
	char *data = NULL;
	char *temp = NULL;
	char *packet = NULL;
	int copy_len = 0;
	int rc;

	// read the file
	input = fopen(filename, "r");
	check((input != NULL), "Can't find the file.");

	source = (char *) malloc(SIZE);
	check((source != NULL), "Can't alloc merrory.");

	data = (char *) malloc(SIZE);
	check((data != NULL), "Can't alloc merrory.");

	temp = (char *) malloc(SIZE);
	check((temp != NULL), "Can't alloc merrory.");

	packet = (char *) malloc(PACKET_LEN);
	check((packet != NULL), "Can't alloc merrory.");

	while (fgets(source, SIZE, input) != NULL) {
		// First: delete the '\n'
		int len = strlen(source);
		strncpy(temp, source, len - 1);
		temp[len - 1] = '\0';

		// Second: copy and calculate the hash value    
		len = strlen(temp);
		// Fixup: maybe len > PACKET_LEN
		strncpy(packet + copy_len, temp, len);
		copy_len += len;

		sumbytes += len;

		if (copy_len >= chunk_num) {
			//calculate the hash
			rc = calculate_hash(packet, copy_len, Q, R, RM,
					    zero_value, chunk_num);
			check(rc == 0, "calculate hash maybe error.");
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
	fclose(input);
	free(source);
	free(data);
	free(temp);
	free(packet);
	return -1;
}

int
main()
{
	//int chunk_num = 64;	// divide the string into chunk
	int chunk_num = 64;	// divide the string into chunk
	int zero_num = 5;	// the number of bits which equals 0
	int zero_value = 1;
	long Q = 1;
	int i = 0;
	int R = 1048583;
	long RM = 1;
	int rc;
	char source[1024] = "/root/kernel_test/model/c-skeleton/bin/a";

	//open a count file.
	fp = fopen("count", "w");
	fpw = fopen("hash", "w");

	// precalculate
	for (i = 0; i < 60; ++i)
		Q = (2 * Q);

	for (i = 0; i < zero_num; ++i)
		zero_value = (2 * zero_value);
	zero_value = zero_value - 1;

	// precompute R^(M-1) % Q for use in removing leading digit
	for (i = 1; i <= chunk_num - 1; i++)
		RM = (R * RM) % Q;

	// bulid a hash map
	map =
	    Hashmap_create(BUCKETS_LEN, Hashmap_mode_compare,
			   Hashmap_mode_hash);
	check(map != NULL, "Failed to create map.");

	rc = read_file(source, Q, R, RM, zero_value, chunk_num);
	check(rc == 0, "Failed to calculate hash.");

	unsigned long result = sumbytes - recount * (chunk_num - REMEDY);
	printf("sum bytes is %lu and re count is %d and result is %lu\n",
	       sumbytes, recount, result);
	printf("total hashkey is %lu\n", hashmapnum);
	//for(i = 0; i < BUCKETS_LEN; i++) {
	//     printf("%lu\n", stats[i]);
	//}

	fclose(fp);
	fclose(fpw);
	Hashmap_destroy(map);
	return 0;

      error:
	fclose(fp);
	fclose(fpw);
	Hashmap_destroy(map);
	return -1;
}
