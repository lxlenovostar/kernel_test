/* SLAB cache for hash item data */
extern struct kmem_cache *sha_data;

extern struct kmem_cache *slab_chunk1;
extern struct kmem_cache *slab_chunk2;
extern struct kmem_cache *slab_chunk3;
extern struct kmem_cache *reskb_cachep;
extern struct kmem_cache *readskb_cachep;
extern struct kmem_cache *listhead_cachep;
extern struct kmem_cache *tasklet_cachep;

#define CACHE_NAME_ITEM "hash_cache_item"
#define CACHE_SLAB_CHUNK1 "slab_chunk_1"
#define CACHE_SLAB_CHUNK2 "slab_chunk_2"
#define CACHE_SLAB_CHUNK3 "slab_chunk_3"
#define CACHE_NAME_SKB "skb_cache"
#define CACHE_NAME_LISTHEAD "listhead_cache"
#define CACHE_NAME_READSKB "readskb_cache"
#define CACHE_NAME_TASKLET "tasklet_cache"

int alloc_slab(void);
void free_slab(void);
