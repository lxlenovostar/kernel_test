#ifndef _lcthw_Hashmap_h
#define _lcthw_Hashmap_h

#include <stdint.h>
#include <lcthw/darray.h>

#define DEFAULT_NUMBER_OF_BUCKETS 100

typedef int (*Hashmap_compare) (void *a, void *b);
typedef uint32_t(*Hashmap_hash) (void *key);

typedef struct Hashmap {
	DArray *buckets;
	Hashmap_compare compare;
	Hashmap_hash hash;
} Hashmap;

typedef struct HashmapNode {
	void *key;
	void *data;
	uint32_t hash;

	/*
     * make a mult-link hashmap.
     * next: a array which contains all next nodes.
	 * next_count: count the next nodes.
     */
	void *next;      	 
	int next_count;  
} HashmapNode;

typedef int (*Hashmap_traverse_cb) (HashmapNode * node);

/*
 * if buckets_len is 0, we will use the default value, 
 * else we will user our value.
 */
Hashmap *Hashmap_create(long buckets_len, Hashmap_compare compare, Hashmap_hash);
void Hashmap_destroy(Hashmap * map);

int Hashmap_set(Hashmap * map, void *key, void *data);
void *Hashmap_get(Hashmap * map, void *key);

int Hashmap_traverse(Hashmap * map, Hashmap_traverse_cb traverse_cb);

void *Hashmap_delete(Hashmap * map, void *key);

#endif
