/* SLAB cache for hash item data */
extern struct kmem_cache * hash_item_data;

#define CACHE_NAME_ITEM "hash_cache_item"

int alloc_slab(void);
void free_slab(void);
