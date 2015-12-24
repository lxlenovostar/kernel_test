/* SLAB cache for hash item data */
extern struct kmem_cache *sha_data;

#define CACHE_NAME_ITEM "sha_cache"

int alloc_slab(void);
void free_slab(void);
