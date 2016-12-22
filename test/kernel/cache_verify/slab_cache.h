/* SLAB cache for hash item data */
extern struct kmem_cache * hash_item_data;

extern struct kmem_cache * slab_chunk1;
extern struct kmem_cache * slab_chunk2;
extern struct kmem_cache * slab_chunk3;

#define CACHE_NAME_ITEM "hash_cache_item"
#define CACHE_SLAB_CHUNK1 "slab_chunk_1"
#define CACHE_SLAB_CHUNK2 "slab_chunk_2"
#define CACHE_SLAB_CHUNK3 "slab_chunk_3"

int alloc_slab(void);
void free_slab(void);
