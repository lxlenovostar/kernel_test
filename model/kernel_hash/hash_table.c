#include <asm/types.h>
#include <linux/version.h>
#include <linux/mm.h>
#include "hash_lock.h"
#include "hash_table.h"
#include "debug.h"

#define WS_SP_HASH_TABLE_BITS 20

void inline init_tcp_item(struct hashinfo_item *tcp_item)
{
}

void inline release_tcp_item(struct hashinfo_item *tcp_item)
{
}

uint32_t hash_tab_size  = (1<<WS_SP_HASH_TABLE_BITS);
uint32_t hash_tab_mask  = ((1<<WS_SP_HASH_TABLE_BITS)-1);

unsigned long _ws_timeout_hash_syn = 6*HZ;

/*
 * Connection hash table: for input and output packets lookups of sp
 */
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
    ct_write_unlock_bh(hash, hash_lock_array);
    return 1;
}

/*
static struct hashinfo_item* hash_new(struct hashinfo *info)
{
    uint32_t hash;
    struct hashinfo_item *cp;
    int hash_count = atomic_read(&hash_count);
    if (hash_count > hash_max_count){
        DEBUG_LOG(__FUNCTION__);
        return NULL;
    }   

    cp = kmem_cache_zalloc(hash_cachep, GFP_ATOMIC);  
    if (cp == NULL) {
        DEBUG_LOG(__FUNCTION__);
        return NULL;
    }   

    INIT_LIST_HEAD(&cp->c_list);
    atomic_inc(&hash_count);

    hash = hash_hashkey(tcp_info); //hash function.
    hash(hash, cp);
    DEBUG_LOG(__FUNCTION__);
    
	return cp; 
}
*/

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

static void hash_expire(unsigned long data)
{
    struct sp_tcp_hashinfo_item *cp = (struct sp_tcp_hashinfo_item *)data;
    atomic_inc(&cp->refcnt);
    if (likely(atomic_read(&cp->refcnt) == 2)) {
        uint32_t hash = ws_sp_hash_hashkey(&cp->tcp_info);
        ws_sp_unhash(hash, cp);
        if (likely(atomic_read(&cp->refcnt) == 1)){
            if (timer_pending(&cp->timer))
                del_timer(&cp->timer);
            atomic_dec(&_ws_sp_hash_count);
            release_tcp_item(cp);
            debug_tcp_info(__FUNCTION__, &(cp->tcp_info));
            kmem_cache_free(_ws_sp_hash_cachep, cp);
            return;
        }
        ws_sp_hash(hash, cp);
    }
    ws_sp_hash_put(cp, _ws_timeout_hash_syn);
    debug_tcp_info(__FUNCTION__, &cp->tcp_info);
}

static void hash_flush(void)
{
    int idx;
    struct hashinfo_item *cp;

flush_again:
    for (idx = 0; idx < hash_tab_size; idx++) {
        ct_write_lock_bh(idx, hash_lock_array);
        list_for_each_entry(cp, &hash_tab[idx], c_list) {
            ws_sp_hash_expire_now(cp);
        }
        ct_write_unlock_bh(idx, hash_lock_array);
    }

    /* the counter may be not NULL, because maybe some conn entries
    are run by slow timer handler or unhashed but still referred */
    if (atomic_read(&_ws_sp_hash_count) != 0) {
        schedule();
        goto flush_again;
    }

    DEBUG_ERROR("<<==>> %s\n", __FUNCTION__ );
}

void release_hash_table_cache(void)
{
    hash_flush();

    kmem_cache_destroy(hash_cachep);
    vfree(hash_tab);

    DEBUG_LOG("<<==>> %s  \n", __FUNCTION__);
}

