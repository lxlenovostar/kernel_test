/* SLAB cache for hash item data */
extern struct kmem_cache * hash_item_data;
//extern struct kmem_cache * slab_work;

#define CACHE_NAME_ITEM "hash_cache_item"
#define CACHE_SLAB_WORK "cache_slab_work"

int alloc_slab(void);
void free_slab(void);
