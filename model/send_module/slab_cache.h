/* SLAB cache for hash item data */
extern struct kmem_cache *sha_data;
extern struct kmem_cache *replace_data;

#define CACHE_NAME_ITEM "sha_cache"
#define CACHE_REPLACE "replace_cache"

int alloc_slab(void);
void free_slab(void);
