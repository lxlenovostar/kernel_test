/*
 * store the key and value  of hashmap. 
 */
#include <stdlib.h>
#include <lcthw/dbg.h>
#include <string.h>

#define SHA 40 //SHA-1 need 40 bytes.
#define SHANUM 10000
#define NUM (SHA*SHANUM)

typedef struct keyvalue{
	unsigned long index;
	unsigned long capacity;
	unsigned long expand_rate;
	//void key[][40];
	void *key;
} keyvalue;

keyvalue *keyvalue_create();
void keyvalue_destroy(keyvalue *kv);
int keyvalue_full(keyvalue *kv);
int keyvalue_expand(keyvalue *kv);
int keyvalue_push(keyvalue *kv, void *el);
