/* VincentR2D2&liyi */
/* Copyright (C) 2011  wangsu Corporation.  All Rights Reserved. */
#include <asm/types.h>
#include <linux/version.h>
#include "ws_sp_hash_table.h"
#include "ws_sp_hash.h"

#define WS_SP_HASH_TABLE_BITS 20

void inline debug_tcp_info(const char *func, struct sp_tcp_hashinfo *tcp_info)
{
	DEBUG_FUNC("====== %s : client %d.%d.%d.%d:%d, lvs %d.%d.%d.%d:%d, "
			"seq %u, ack %u\n", func,
			NIPQUAD(tcp_info->saddr), ntohs(tcp_info->sport),
			NIPQUAD(tcp_info->daddr), ntohs(tcp_info->dport),
			ntohl(tcp_info->seq), ntohl(tcp_info->ack_seq));
}

void inline init_tcp_item(struct sp_tcp_hashinfo_item *tcp_item)
{
}

void inline release_tcp_item(struct sp_tcp_hashinfo_item *tcp_item)
{
}

/* Connection hash size. Default is what was selected at compile time. */

uint32_t _ws_sp_hash_tab_size  = (1<<WS_SP_HASH_TABLE_BITS);
uint32_t _ws_sp_hash_tab_mask  = ((1<<WS_SP_HASH_TABLE_BITS)-1);

unsigned long _ws_timeout_hash_syn = 6*HZ;

/*
 * Connection hash table: for input and output packets lookups of sp
 */
static struct list_head *_ws_sp_hash_tab = NULL;

/* SLAB cache for sp hash */
static struct kmem_cache * _ws_sp_hash_cachep/* __read_mostly*/;

/* counter for current wslvs connections */
atomic_t _ws_sp_hash_count = ATOMIC_INIT(0);
uint32_t _ws_sp_hash_max_count = 100*1000*1000;
static unsigned int _ws_sp_hash_rnd = 0;


static struct ws_sp_aligned_lock _ws_sp_hash_lock_array[CT_LOCKARRAY_SIZE];

static uint32_t __ws_sp_hash_hashkey(uint32_t saddr, uint16_t port)
{
	return jhash_2words((__force u32)saddr, (__force u32)port, _ws_sp_hash_rnd)&_ws_sp_hash_tab_mask;
}

static uint32_t ws_sp_hash_hashkey(struct sp_tcp_hashinfo *key)
{
	return __ws_sp_hash_hashkey(key->saddr, key->sport);
}

static inline int is_same_tcp_info(struct sp_tcp_hashinfo *x,
		struct sp_tcp_hashinfo *y,
		int cmp_seq)
{
	if (cmp_seq && x->ack_seq != y->ack_seq)
		return 0;
	return x->daddr==y->daddr && x->dport==y->dport
		&& x->saddr==y->saddr && x->sport==y->sport;
}

static inline uint32_t ws_sp_hash(uint32_t hash, struct sp_tcp_hashinfo_item *cp)
{
	ct_write_lock_bh(hash, _ws_sp_hash_lock_array);
	list_add(&cp->c_list, &_ws_sp_hash_tab[hash]);
	atomic_inc(&cp->refcnt);
	ct_write_unlock_bh(hash, _ws_sp_hash_lock_array);
	return 1;
}

static inline uint32_t ws_sp_unhash(uint32_t hash, struct sp_tcp_hashinfo_item *cp)
{
	ct_write_lock_bh(hash, _ws_sp_hash_lock_array);
	list_del(&cp->c_list);
	atomic_dec(&cp->refcnt);
	ct_write_unlock_bh(hash, _ws_sp_hash_lock_array);
	return 1;
}

/* put back the conn without restarting its timer */
static inline void __ws_sp_hash_put(struct sp_tcp_hashinfo_item *cp)
{
	atomic_dec(&cp->refcnt);
}

static inline void ws_sp_hash_put(struct sp_tcp_hashinfo_item *cp,
		unsigned long timeout)
{
	mod_timer(&cp->timer, jiffies+timeout);
	__ws_sp_hash_put(cp);
}

static void ws_sp_hash_expire(unsigned long data)
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
			debug_tcp_info(__FUNCTION__, &(cp->tcp_info));
			kmem_cache_free(_ws_sp_hash_cachep, cp);
			return;
		}
		ws_sp_hash(hash, cp);
	}
	ws_sp_hash_put(cp, _ws_timeout_hash_syn);
	debug_tcp_info(__FUNCTION__, &cp->tcp_info);
}

static inline void ws_sp_hash_expire_now(struct sp_tcp_hashinfo_item *cp)
{
	if (del_timer(&cp->timer))
		mod_timer(&cp->timer, jiffies);
}

/*
 * Create a new connection entry and hash it into the ip_vs_conn_tab
 */
static struct sp_tcp_hashinfo_item* ws_sp_hash_new(struct sp_tcp_hashinfo *tcp_info)
{
	uint32_t hash;
	struct sp_tcp_hashinfo_item *cp;
	int hash_count = atomic_read(&_ws_sp_hash_count);
	if (hash_count > _ws_sp_hash_max_count){
		debug_tcp_info(__FUNCTION__, tcp_info);
		return NULL;
	}

	cp = kmem_cache_zalloc(_ws_sp_hash_cachep, GFP_ATOMIC);  /*is atomic needed?---by liyi*/
	if (cp == NULL) {
		debug_tcp_info(__FUNCTION__, tcp_info);
		return NULL;
	}

	INIT_LIST_HEAD(&cp->c_list);
	setup_timer(&cp->timer, ws_sp_hash_expire, (unsigned long)cp);
	cp->tcp_info = *tcp_info;
	atomic_set(&cp->refcnt, 1);
	atomic_inc(&_ws_sp_hash_count);

	hash = ws_sp_hash_hashkey(tcp_info);
	ws_sp_hash(hash, cp);

	debug_tcp_info(__FUNCTION__, tcp_info);

	return cp;
}

static void ws_sp_hash_flush(void)
{
	int idx;
	struct sp_tcp_hashinfo_item *cp;

flush_again:
	for (idx = 0; idx < _ws_sp_hash_tab_size; idx++) {
		// Lock is actually needed in this loop.
		ct_write_lock_bh(idx, _ws_sp_hash_lock_array);
		list_for_each_entry(cp, &_ws_sp_hash_tab[idx], c_list) {
			ws_sp_hash_expire_now(cp);
		}
		ct_write_unlock_bh(idx, _ws_sp_hash_lock_array);
	}

	/* the counter may be not NULL, because maybe some conn entries
	are run by slow timer handler or unhashed but still referred */
	if (atomic_read(&_ws_sp_hash_count) != 0) {
		schedule();
		goto flush_again;
	}

	DEBUG_ERROR("<<==>> %s\n", __FUNCTION__ );
}


/* Function: initial_sp_hash_table_cache */
int __init initial_sp_hash_table_cache(void)
{
	int idx;
	// Allocate the connection hash table and initialize its list heads
	_ws_sp_hash_tab = vmalloc(_ws_sp_hash_tab_size * sizeof(struct list_head));
	if (!_ws_sp_hash_tab) {
		DEBUG_ERROR("****** %s : vmalloc tab error\n", __FUNCTION__);
		return -ENOMEM;
	}

	/* Allocate ip_vs_conn slab cache */
#if ( LINUX_VERSION_CODE < KERNEL_VERSION(2,6,25) )
	_ws_sp_hash_cachep = kmem_cache_create(CACHE_NAME,
			sizeof(struct sp_tcp_hashinfo_item),
			0, SLAB_HWCACHE_ALIGN, NULL, NULL);
#else
	_ws_sp_hash_cachep = kmem_cache_create(CACHE_NAME,
			sizeof(struct sp_tcp_hashinfo_item),
			0, SLAB_HWCACHE_ALIGN, NULL);
#endif

	if (!_ws_sp_hash_cachep) {
		vfree(_ws_sp_hash_tab);
		DEBUG_ERROR("****** %s : kmem_cache_create ws_lvs_conn error\n",
				__FUNCTION__);
		return -ENOMEM;
	}

	for (idx=0; idx<_ws_sp_hash_tab_size; idx++)
		INIT_LIST_HEAD(&_ws_sp_hash_tab[idx]);

	for (idx=0; idx<CT_LOCKARRAY_SIZE; idx++)
		rwlock_init(&_ws_sp_hash_lock_array[idx].l);

	/* calculate the random value for connection hash */
	get_random_bytes(&_ws_sp_hash_rnd, sizeof(_ws_sp_hash_rnd));

	DEBUG_ERROR("<<==>> %s : table count =%d, memory=%ldKbytes \n",
			__FUNCTION__, _ws_sp_hash_tab_size,
			(long)((_ws_sp_hash_tab_size*sizeof(struct list_head))>>10));

	return 0;
}

//
// Function: release_sp_hash_table_cache
//
// Description:
//   ±¾º¯ÊýÊµÏÖcacheÊÍ·Å
//
void release_sp_hash_table_cache(void)
{
	ws_sp_hash_flush();

	kmem_cache_destroy(_ws_sp_hash_cachep);
	vfree(_ws_sp_hash_tab);

	DEBUG_ERROR("<<==>> %s  \n", __FUNCTION__);
}

void reset_sp_hash_table_cache(void)
{
	ws_sp_hash_flush();
}

struct sp_tcp_hashinfo_item *get_tcp_hash_item(struct sp_tcp_hashinfo *tcp_hash_info,
		int cmp_seq)
{
	uint32_t hash;
	struct sp_tcp_hashinfo_item *cp;
	hash=ws_sp_hash_hashkey(tcp_hash_info);

	ct_read_lock_bh(hash, _ws_sp_hash_lock_array);
	list_for_each_entry(cp, &_ws_sp_hash_tab[hash], c_list) {
		if (is_same_tcp_info(&cp->tcp_info, tcp_hash_info, cmp_seq)) {
				atomic_inc(&cp->refcnt);
				ct_read_unlock_bh(hash, _ws_sp_hash_lock_array);

				debug_tcp_info(__FUNCTION__, tcp_hash_info);
				return cp;
		}
	}
	ct_read_unlock_bh(hash, _ws_sp_hash_lock_array);
	return NULL;
}

struct sp_tcp_hashinfo_item *get_tcp_hash_item_by_syn(struct sp_tcp_hashinfo *tcp_hash_info)
{
	return get_tcp_hash_item(tcp_hash_info, 0);
}

struct sp_tcp_hashinfo_item *get_tcp_hash_item_by_ack(struct sp_tcp_hashinfo *tcp_hash_info)
{
	return get_tcp_hash_item(tcp_hash_info, 1);
}

int add_tcp_hash_info(struct sp_tcp_hashinfo *tcp_hash_info)
{
	struct sp_tcp_hashinfo_item *cp=NULL;

	cp=ws_sp_hash_new(tcp_hash_info);
	if(cp == NULL)
		return -1;
	init_tcp_item(cp);
	ws_sp_hash_put(cp, _ws_timeout_hash_syn);
	debug_tcp_info(__FUNCTION__, tcp_hash_info);
	return 0;
}

void put_tcp_hash_info(struct sp_tcp_hashinfo_item* tcp_hash_info_item,
		int delete, unsigned long timeout)
{
	if (delete>0){
		__ws_sp_hash_put(tcp_hash_info_item);
		ws_sp_hash_expire_now(tcp_hash_info_item);
	}else
		ws_sp_hash_put(tcp_hash_info_item, timeout);
}

int get_total_sp_hash_count(void)
{
	return atomic_read(&_ws_sp_hash_count);
}

void set_sp_timeout(unsigned long timeout)
{
	_ws_timeout_hash_syn = timeout*HZ;
}

unsigned long get_sp_timeout(void)
{
	return _ws_timeout_hash_syn/HZ;
}

/*VincentR2D2&liyi*/

