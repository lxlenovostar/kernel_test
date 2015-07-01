/*
 * store the key and value  of hashmap. 
 */
#include <stdlib.h>
#define NUM 1024
#define SHA 40 //SHA-1 need 40 bytes.

struct keyvalue {
	unsigned long index;
	unsigned long capacity;
	unsigned long expand_rate;
	//void key[][40];
	void *key;
	DArray *buckets;
	//void value[][]; //maybe don't use it.
}

keyvalue* keyvalue_create();
void keyvalue_destory(keyvalue *kv);
int keyvalue_full(keyvalue *kv);
static inline int keyvalue_resize(keyvalue *kv, size_t newsize);
int keyvalue_expand(keyvalue *kv);

