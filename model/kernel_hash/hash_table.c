#include <asm/types.h>
#include <linux/version.h>
#include <linux/mm.h>
#include "hash_lock.h"
#include "hash_table.h"
#include "debug.h"

#define WS_SP_HASH_TABLE_BITS 20

uint32_t hash_tab_size  = (1<<WS_SP_HASH_TABLE_BITS);
uint32_t hash_tab_mask  = ((1<<WS_SP_HASH_TABLE_BITS)-1);

static struct list_head *hash_tab = NULL;

/* SLAB cache for sp hash */
static struct kmem_cache * hash_cachep/* __read_mostly*/;

/* counter for current wslvs connections */
atomic_t hash_count = ATOMIC_INIT(0);
uint32_t hash_max_count = 100*1000*1000;


static struct _aligned_lock hash_lock_array[CT_LOCKARRAY_SIZE];

static inline uint32_t _hash(uint32_t hash, struct hashinfo_item *cp)
{
    ct_write_lock_bh(hash, hash_lock_array);
    list_add(&cp->c_list, &hash_tab[hash]);
	atomic_inc(&cp->refcnt);
    ct_write_unlock_bh(hash, hash_lock_array);
    return 1;
}

static inline uint32_t _unhash(uint32_t hash, struct hashinfo_item *cp)
{
    ct_write_lock_bh(hash, hash_lock_array);
    list_del(&cp->c_list);
    atomic_dec(&cp->refcnt);
    ct_write_unlock_bh(hash, hash_lock_array);
    return 1;
}

static struct hashinfo_item* hash_new(uint8_t *info)
{
    uint32_t hash, bkt;
    struct hashinfo_item *cp;
    int hash_count_now = atomic_read(&hash_count);
    if (hash_count_now > hash_max_count){
    	DEBUG_LOG(KERN_INFO "%s\n", __FUNCTION__ );
        return NULL;
    }   

    cp = kmem_cache_zalloc(hash_cachep, GFP_ATOMIC);  
    if (cp == NULL) {
    	DEBUG_LOG(KERN_INFO "%s\n", __FUNCTION__ );
        return NULL;
    }   

    INIT_LIST_HEAD(&cp->c_list);
	memcpy(cp->sha1, info, SHA1SIZE);
	atomic_set(&cp->refcnt, 1);    
    atomic_inc(&hash_count);
	HASH_FCN(cp->sha1, SHA1SIZE, hash_tab_size, hash, bkt);
    _hash(bkt, cp);
    DEBUG_LOG(KERN_INFO "%s", __FUNCTION__ );
    
	return cp; 
}

int add_hash_info(uint8_t *info)
{
    struct hashinfo_item *cp=NULL;

    cp = hash_new(info);
    if(cp == NULL)
        return -1; 
    return 0;
}

struct hashinfo_item *get_hash_item(uint8_t *info)
{
    uint32_t hash, bkt;
    struct hashinfo_item *cp;
	DEBUG_LOG(KERN_INFO "info1 is:%p\n", info);
	HASH_FCN(info, SHA1SIZE, hash_tab_size, hash, bkt);
	DEBUG_LOG(KERN_INFO "info2 is:%p\n", info);
    ct_read_lock_bh(hash, hash_lock_array);
	DEBUG_LOG(KERN_INFO "info3 is:%p\n", info);
    list_for_each_entry(cp, &hash_tab[bkt], c_list) {
		DEBUG_LOG(KERN_INFO "info4 is:%p\n", info);
		if (memcmp(cp->sha1, info, SHA1SIZE) == 0) {
    			DEBUG_LOG(KERN_INFO "%s fuck\n", __FUNCTION__ );
                atomic_inc(&cp->refcnt);
                ct_read_unlock_bh(hash, hash_lock_array);
                return cp; 
        }   
    }   
    ct_read_unlock_bh(hash, hash_lock_array);
    return NULL;
}

int initial_hash_table_cache(void)
{
    int idx;
    
	hash_tab = vmalloc(hash_tab_size * sizeof(struct list_head));
    if (!hash_tab) {
        DEBUG_LOG(KERN_ERR"****** %s : vmalloc tab error\n", __FUNCTION__);
        return -ENOMEM;
    }

#if ( LINUX_VERSION_CODE < KERNEL_VERSION(2,6,25) )
    hash_cachep = kmem_cache_create(CACHE_NAME,
            sizeof(struct hashinfo_item),
            0, SLAB_HWCACHE_ALIGN, NULL, NULL);
#else
    hash_cachep = kmem_cache_create(CACHE_NAME,
            sizeof(struct hashinfo_item),
            0, SLAB_HWCACHE_ALIGN, NULL);
#endif

    if (!hash_cachep) {
        vfree(hash_tab);
        DEBUG_LOG(KERN_ERR "****** %s : kmem_cache_create ws_lvs_conn error\n",
                __FUNCTION__);
        return -ENOMEM;
    }

    for (idx=0; idx<hash_tab_size; idx++)
        INIT_LIST_HEAD(&hash_tab[idx]);

    for (idx=0; idx<CT_LOCKARRAY_SIZE; idx++)
        rwlock_init(&hash_lock_array[idx].l);

    return 0;
}

static void hash_del(struct hashinfo_item *cp)
{
    atomic_inc(&cp->refcnt);
    if (likely(atomic_read(&cp->refcnt) == 2)) {
        uint32_t hash, bkt;
		HASH_FCN(cp->sha1, SHA1SIZE, hash_tab_size, hash, bkt);
        _unhash(bkt, cp);
        if (likely(atomic_read(&cp->refcnt) == 1)){
            atomic_dec(&hash_count);
            kmem_cache_free(hash_cachep, cp);
            return;
        }
        _hash(bkt, cp);
    }
    atomic_dec(&cp->refcnt);
    DEBUG_LOG(KERN_INFO "%s\n", __FUNCTION__ );
}

static void hash_flush(void)
{
    int idx;
    struct hashinfo_item *cp;

flush_again:
    for (idx = 0; idx < hash_tab_size; idx++) {
        ct_write_lock_bh(idx, hash_lock_array);
        list_for_each_entry(cp, &hash_tab[idx], c_list) {
            hash_del(cp);
        }
        ct_write_unlock_bh(idx, hash_lock_array);
    }

    /* the counter may be not NULL, because maybe some conn entries
    are unhashed but still referred */
    if (atomic_read(&hash_count) != 0) {
        schedule();
        goto flush_again;
    }

    DEBUG_LOG(KERN_INFO "%s\n", __FUNCTION__ );
}

void release_hash_table_cache(void)
{
    hash_flush();

    kmem_cache_destroy(hash_cachep);
    vfree(hash_tab);

    DEBUG_LOG(KERN_INFO "%s\n", __FUNCTION__);
}

