/*
 * store the key and value  of hashmap. 
 */
#include <stdlib.h>
#include <lcthw/dbg.h>
#include <string.h>

#define SHA 20 //SHA-1 need 20 bytes.
#define SHANUM 8000000


typedef struct keyvalue{
	unsigned long index;
	unsigned long capacity;
	unsigned long expand_rate;
	int step;
	//void key[][40];
	void *key;
} keyvalue;

keyvalue* keyvalue_create(size_t num, size_t step_s);
void keyvalue_destroy(keyvalue *kv);
int keyvalue_full(keyvalue *kv);
int keyvalue_expand(keyvalue *kv);
void* keyvalue_push(keyvalue *kv, void *el);
