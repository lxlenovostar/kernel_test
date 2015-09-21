#include <asm/types.h>
#include <linux/version.h>
#include "hash_table.h"
#include "hash_lock.h"

#define WS_SP_HASH_TABLE_BITS 20

void inline init_tcp_item(struct sp_tcp_hashinfo_item *tcp_item)
{
}

void inline release_tcp_item(struct sp_tcp_hashinfo_item *tcp_item)
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
static unsigned int hash_rnd = 0;


static struct aligned_lock hash_lock_array[CT_LOCKARRAY_SIZE];

static struct hashinfo_item* hash_new(struct sp_tcp_hashinfo *tcp_info)
{
    uint32_t hash;
    struct sp_tcp_hashinfo_item *cp;
    int hash_count = atomic_read(&hash_count);
    if (hash_count > hash_max_count){
        debug_tcp_info(__FUNCTION__, tcp_info);
        return NULL;
    }   

    cp = kmem_cache_zalloc(hash_cachep, GFP_ATOMIC);  /*is atomic needed?---by liyi*/
    if (cp == NULL) {
        debug_tcp_info(__FUNCTION__, tcp_info);
        return NULL;
    }   

    INIT_LIST_HEAD(&cp->c_list);
    setup_timer(&cp->timer, hash_expire, (unsigned long)cp);
    cp->tcp_info = *tcp_info;
    atomic_set(&cp->refcnt, 1); 
    atomic_inc(&hash_count);

    hash = hash_hashkey(tcp_info);
    hash(hash, cp);

    debug_tcp_info(__FUNCTION__, tcp_info);

    return cp; 
}
