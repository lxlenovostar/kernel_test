/*
 * store the key and value  of hashmap. 
 */
#include <stdlib.h>
#include <lcthw/dbg.h>

#define LEN 30000

typedef struct chunk{
	unsigned long end;	//don't use it.
	unsigned long limit;
	char *content;
} chunk;

chunk *chunk_create();
void chunk_destroy(chunk *ck);
void chunk_clean(chunk *ck);
int chunk_store(chunk *ck, char *src, long begin, long end);
int chunk_merge(chunk *ck, char *src, long begin, long end);
