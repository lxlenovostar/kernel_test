#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "lcthw/dbg.h"
#include "lcthw/hashmap.h"

#define CHUNK 64
#define SIZE 30000
#define PACKET_LEN 30000
#define BUCKETS_LEN 997
#define NUM 30000
#define KEYNUM BUCKETS_LEN * NUM
#define REMEDY 12
Hashmap *map = NULL;
FILE *fp = NULL;
FILE *fpw = NULL;

static unsigned long hashmapnum = 0;
static int recount = 0;
static unsigned long sumbytes = 0;
unsigned long hash_key[KEYNUM];
char hash_keydata[KEYNUM][CHUNK*3 + 1];
unsigned long stats[KEYNUM];
static int reindex = 0;
static unsigned long commonbytes = 0;

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
calculate_hash(char *playload, int playload_len, long Q, int R, long RM,
	       int zero_value, int chunk_num)
{
	int i;
	char *data = NULL;
	int delay_time = 0;

	unsigned long txthash = hash(playload, chunk_num, R, Q);

	if ((txthash & zero_value) == 0) {
		//insert the hashmap.

		//just for test
		/*char temphash[65];
		memcpy(temphash, playload, chunk_num);
		temphash[chunk_num] = '\0';
		fprintf(fpw, "%s %lu\n", temphash, txthash);
		*/

		if ((data = Hashmap_get(map, &txthash)) != NULL) {
			if (memcmp(data, playload, chunk_num) == 0) {
				++recount;
				delay_time = chunk_num;

				/*
				//fixup : strlen maybe a bug
				int cmp_len = strlen(data) > playload_len ? playload_len : strlen(data);
				for (i = chunk_num; i < cmp_len; ++i) {
					if (data[i] == playload[i]) {
						++delay_time;
						++commonbytes;
						debug("find");
					}
					else
						break;
				}
				*/

				char temp[65];
				memcpy(temp, playload, chunk_num);
				temp[chunk_num] = '\0';
				fprintf(fp, "%s\n", temp);

				debug("data is %s\ntemp is %s", data, temp);
			}
		} else {
			if (reindex < KEYNUM) {
				hash_key[reindex] = txthash;
			
				if (playload_len < chunk_num*3)
					memcpy(hash_keydata + reindex, playload, playload_len);
				else
					memcpy(hash_keydata + reindex, playload, chunk_num*3);

				int rc = Hashmap_set(map, (hash_key + reindex),
						     hash_keydata + reindex);
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
			} else {
				goto error;
			}
		}
	}

	for (i = chunk_num; i < playload_len; i++) {
		txthash = (txthash + Q - RM * playload[i - chunk_num] % Q) % Q;
		txthash = (txthash * R + playload[i]) % Q;

		if (delay_time == 0) {
			if ((txthash & zero_value) == 0) {
				//check whether we have the same one.

				// just for test
				/*char temphash[65];
				memcpy(temphash, playload + i - chunk_num,
				       chunk_num);
				temphash[chunk_num] = '\0';
				fprintf(fpw, "%s %lu\n", temphash, txthash);
				*/

				if ((data = Hashmap_get(map, &txthash)) != NULL) {
					if (memcmp(data, playload + i - chunk_num, chunk_num) == 0) {
						++recount;
						delay_time = chunk_num;
						
						/*
						//fixup : strlen maybe a bug
						int cmp_len = strlen(data) > playload_len - i? playload_len - i : strlen(data);
						int j, k;
						k = i;
						for (j = chunk_num; j < cmp_len; ++j) {
							if (data[j] == playload[k]) {
								++delay_time;
								++commonbytes;
								++k;
								debug("find1");
							}
							else
								break;
						}
						*/

						char temp[65];
						memcpy(temp,
						       playload + i - chunk_num,
						       chunk_num);
						temp[chunk_num] = '\0';
						fprintf(fp, "%s\n", temp);

						debug
						    ("data is %s\ntemp is %s\n",
						     data, temp);
					}
					continue;
				}

				if (reindex < KEYNUM) {
					hash_key[reindex] = txthash;
					
					if (playload_len < chunk_num*3)
						memcpy(hash_keydata + reindex, playload + i - chunk_num, playload_len);
					else
						memcpy(hash_keydata + reindex, playload + i - chunk_num, chunk_num*3);
					
					int rc = Hashmap_set(map, (hash_key + reindex), hash_keydata + reindex);
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
				} else {
					goto error;
				}
			}
		} else {
			--delay_time;
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
	debug("hash is %d", hash);
	return hash;
}

static int
Hashmap_mode_compare(void *a, void *b)
{
	// 0 stands for equal.
	debug("a is %lu", *(unsigned long *) a);
	debug("b is %lu", *(unsigned long *) b);

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
		copy_len = strlen(temp);
		// 30000 is the lagest pack playload. 
		memcpy(packet, temp, copy_len);

		packet[copy_len] = '\0';
		sumbytes += copy_len;

		if (copy_len >= chunk_num) {
			//calculate the hash
			rc = calculate_hash(packet, copy_len, Q, R, RM,
					    zero_value, chunk_num);
			check(rc == 0, "calculate hash maybe error.");
		}
		copy_len = 0;
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
	int chunk_num = CHUNK;	// divide the string into chunk
	int zero_num = 5;	// the number of bits which equals 0
	int zero_value = 1;
	long Q = 1;
	int i = 0;
	int R = 1048583;
	long RM = 1;
	int rc;
	char source[1024] = "/root/kernel_test/model/c-skeleton/bin/a";

	//open a count file.
	fp = fopen("/root/kernel_test/model/c-skeleton/bin/count", "w");
	fpw = fopen("/root/kernel_test/model/c-skeleton/bin/hash", "w");

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

	unsigned long result = sumbytes - recount * (chunk_num - REMEDY) - commonbytes;
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
