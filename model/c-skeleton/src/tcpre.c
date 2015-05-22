#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "lcthw/dbg.h"
#include "lcthw/hashmap.h"

#define SIZE 1024
#define PACKET_LEN 1460
#define BUCKETS_LEN 997
#define NUM 30000
#define KEYNUM BUCKETS_LEN * NUM
#define REMEDY 12
Hashmap *map = NULL;

static unsigned long hashmapnum = 0;
static int recount = 0;
static unsigned long sumbytes = 0;
unsigned long index_key[KEYNUM];
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
		if (reindex < KEYNUM) {
			index_key[reindex] = txthash;
			int rc =
			    Hashmap_set(map, (index_key + reindex), expect);
			check(rc == 0, "Failed to set hashmap");
			++reindex;
            ++hashmapnum;
		}
	}

	for (i = chunk_num; i < palyload_len; i++) {
		txthash = (txthash + Q - RM * playload[i - chunk_num] % Q) % Q;
		txthash = (txthash * R + playload[i]) % Q;

		if (reindex < KEYNUM) {
            if ((txthash & zero_value) == 0) {
                //check whether we have the same one.
                if (Hashmap_get(map, &txthash))
                    continue;

                index_key[reindex] = txthash;
                int rc =
                    Hashmap_set(map, (index_key + reindex), expect);
                check(rc == 0, "Failed to set hashmap");
                ++reindex;
                ++hashmapnum;
            }
        }
	}

	return 0;

      error:
	return -1;
}

void
lookup_hash(char *playload, int palyload_len, long Q, int R, long RM,
	    int zero_value, int chunk_num)
{
	int i;
	int delay_time = 0;
	unsigned long txthash = hash(playload, chunk_num, R, Q);

	if ((txthash & zero_value) == 0) {
		if (Hashmap_get(map, &txthash))
			++recount;
	}

	for (i = chunk_num; i < palyload_len; i++) {
		txthash = (txthash + Q - RM * playload[i - chunk_num] % Q) % Q;
		txthash = (txthash * R + playload[i]) % Q;

		if (delay_time == 0) {
			if ((txthash & zero_value) == 0) {
				if (Hashmap_get(map, &txthash)) {
					delay_time = chunk_num;
					++recount;
				}
			}
			
		}
        else {
				--delay_time;
        }
	}
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
read_file(char *filename, int flag, long Q, int R, long RM, int zero_value,
	  int chunk_num)
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

		if (flag == 2)
			sumbytes += len;

		if (copy_len >= chunk_num) {
			if (flag == 1) {
				//calculate the hash
				rc = calculate_hash(packet, copy_len, Q, R, RM,
						    zero_value, chunk_num);
				check(rc == 0, "calculate hash maybe error.");
				copy_len = 0;
			} else if (flag == 2) {
				lookup_hash(packet, copy_len, Q, R, RM,
					    zero_value, chunk_num);
				copy_len = 0;
			}
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
	int chunk_num = 64;	// divide the string into chunk
	int zero_num = 0;	// the number of bits which equals 0
	int zero_value = 1;
	long Q = 1;
	int i = 0;
	int R = 1048583;
	long RM = 1;
	int rc;
	char source[1024] =
	    "/root/kernel_test/model/c-skeleton/bin/a";
	char refile[1024] =
	    "/root/kernel_test/model/c-skeleton/bin/b";

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

	rc = read_file(source, 1, Q, R, RM, zero_value, chunk_num);
	check(rc == 0, "Failed to calculate hash.");

	rc = read_file(refile, 2, Q, R, RM, zero_value, chunk_num);
	check(rc == 0, "Failed to lookup hash.");

	unsigned long result = sumbytes - recount*(chunk_num - REMEDY);
	printf("sum bytes is %lu and re count is %d and result is %lu\n",
	       sumbytes, recount, result);
    printf("total hashkey is %lu\n", hashmapnum);

	// read the file
	/*input = fopen("/root/kernel_test/model/c-skeleton/bin/history.txt", "r");
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
	   free(packet); */
	Hashmap_destroy(map);
	return 0;

      error:
	Hashmap_destroy(map);
	return -1;
}
