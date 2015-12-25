#include <asm/types.h>
#include <linux/version.h>
#include <linux/mm.h>
#include "hash_lock.h"
#include "hash_table.h"
#include "debug.h"
#include "chunk.h"

#define TIMEINTVAL 10
#define WS_SP_HASH_TABLE_BITS 20
//#define ITEM_CITE_ADD 10
//#define ITEM_CITE_FIND 10
//#define ITEM_DISK_LIMIT 10
#define ITEM_CITE_ADD   6 
#define ITEM_CITE_FIND  6
unsigned long timeout_hash_del = 10*HZ;
/*#define ITEM_CITE_ADD   2 
#define ITEM_CITE_FIND  2
unsigned long timeout_hash_del = 30*HZ;*/
uint32_t hash_tab_size  = (1<<WS_SP_HASH_TABLE_BITS);
uint32_t hash_tab_mask  = ((1<<WS_SP_HASH_TABLE_BITS)-1);

static struct list_head *hash_tab = NULL;

/* SLAB cache for hash item */
static struct kmem_cache * hash_cachep;

/* counter for hash item */
atomic_t hash_count = ATOMIC_INIT(0);
unsigned long long hash_max_count = (2ULL*1024*1024*1024) / sizeof(struct hashinfo_item); /*2GB hash_table memory usage.*/

static struct _aligned_lock hash_lock_array[CT_LOCKARRAY_SIZE];
static struct timer_list print_memory;
void hash_item_expire(unsigned long data);
void bucket_clear_item(unsigned long data);
static struct timer_list *bucket_clear; 

static inline uint32_t _hash(uint32_t hash, struct hashinfo_item *cp)
{
    ct_write_lock_bh(hash, hash_lock_array);
    list_add(&cp->c_list, &hash_tab[hash]);
    ct_write_unlock_bh(hash, hash_lock_array);
    return 1;
}

static inline uint32_t _unhash(uint32_t hash, struct hashinfo_item *cp)
{
    ct_write_lock_bh(hash, hash_lock_array);
    list_del(&cp->c_list);
    ct_write_unlock_bh(hash, hash_lock_array);
    return 1;
}

static struct hashinfo_item* hash_new_item(uint8_t *info, char *value, size_t len_value)
{
   	uint32_t hash, bkt;
   	struct hashinfo_item *cp;
   	int hash_count_now = atomic_read(&hash_count);
   	if (hash_count_now > hash_max_count){
   		DEBUG_LOG(KERN_INFO "%s\n", __FUNCTION__ );
       	return NULL;
   	}   

   	/*
   	 * initial the hash item.
   	 */
   	cp = kmem_cache_zalloc(hash_cachep, GFP_ATOMIC);  
   	if (cp == NULL) {
   		DEBUG_LOG(KERN_INFO "%s\n", __FUNCTION__ );
       	return NULL;
   	}   

	cp->len = len_value;
	
	INIT_LIST_HEAD(&cp->c_list);
	memcpy(cp->sha1, info, SHA1SIZE);
	atomic_set(&cp->refcnt, ITEM_CITE_ADD);    
 
   	/*
   	 * total hash item.
   	 */
	atomic_inc(&hash_count);

	/*
   	 * insert into the hash table.
   	 */
	HASH_FCN(cp->sha1, SHA1SIZE, hash_tab_size, hash, bkt);
   	_hash(bkt, cp);

   	DEBUG_LOG(KERN_INFO "%s", __FUNCTION__ );
	return cp; 
}

int add_hash_info(uint8_t *info, char *value, size_t len_value)
{
    struct hashinfo_item *cp = NULL;

    cp = hash_new_item(info, value, len_value);
    if(cp == NULL)
        return -1; 
    return 0;
}

struct hashinfo_item *get_hash_item(uint8_t *info)
{
    uint32_t hash, bkt;
    struct hashinfo_item *cp;

	HASH_FCN(info, SHA1SIZE, hash_tab_size, hash, bkt);
    ct_read_lock_bh(hash, hash_lock_array);
    list_for_each_entry(cp, &hash_tab[bkt], c_list) {
		if (memcmp(cp->sha1, info, SHA1SIZE) == 0) {
   			DEBUG_LOG(KERN_INFO "find it:%s\n", __FUNCTION__ );
            atomic_add(ITEM_CITE_FIND, &cp->refcnt);
			ct_read_unlock_bh(hash, hash_lock_array);
			return cp; 
        }   
    }   
    ct_read_unlock_bh(hash, hash_lock_array);
    return NULL;
}

unsigned long long old_save = 0ULL;
unsigned long long old_sum = 0ULL;
long old_skb_sum = 0L;

void print_memory_usage(unsigned long data)
{
   	uint32_t hash_count_now = atomic_read(&hash_count);
	int item_size = hash_count_now * sizeof(struct hashinfo_item); 
	long tmp_save = atomic64_read(&save_num);
	long tmp_sum =  atomic64_read(&sum_num);
	long tmp_skb_sum =  atomic64_read(&skb_num);

	printk(KERN_INFO "\n[memory usage]");	
	printk(KERN_INFO "hash item is:%dMB, uesd memory for something is:%lluMB, all memory is:%lluMB, item number is:%u", item_size/1024/1024, used_mem/1024/1024, (item_size + used_mem)/1024/1024, hash_count_now);
	
	printk(KERN_INFO "[speed]");	
	printk(KERN_INFO "save rate is:%lluMB/s sum rate is:%lluMB/s", (tmp_save - old_save)/1024/1024/TIMEINTVAL, (tmp_sum - old_sum)/1024/1024/TIMEINTVAL);

	printk(KERN_INFO "[packets]");	
	printk(KERN_INFO "packet num is:%ld pps", (tmp_skb_sum - old_skb_sum)/TIMEINTVAL);
	if (tmp_sum > 0) {
		printk(KERN_INFO "[cache ratio]");	
		printk(KERN_INFO "save bytes is:%ldKB(%ldMB), all bytes is:%ldKB(%ldMB), Cache ratio is:%ld%%", tmp_save/1024, tmp_save/1024/1024, tmp_sum/1024, tmp_sum/1024/1024, (tmp_save*100)/tmp_sum);
	}

	old_save = tmp_save;
	old_sum = tmp_sum;
	old_skb_sum = tmp_skb_sum;

	mod_timer(&print_memory, jiffies + timeout_hash_del);
}

int initial_hash_table_cache(void)
{
    unsigned long idx;
	int cpunum;

	hash_tab = vmalloc(hash_tab_size * sizeof(struct list_head));
    if (!hash_tab) {
        DEBUG_LOG(KERN_ERR"****** %s : vmalloc tab error\n", __FUNCTION__);
        return -ENOMEM;
    }

	used_mem += hash_tab_size * sizeof(struct list_head);

	bucket_clear  = vmalloc(hash_tab_size * sizeof(struct timer_list));
   	if (!bucket_clear) {
       	DEBUG_LOG(KERN_ERR"****** %s : vmalloc tab error\n", __FUNCTION__);
       	return -ENOMEM;
	}

	used_mem += hash_tab_size * sizeof(struct timer_list);

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
        DEBUG_LOG(KERN_ERR "****** %s : kmem_cache_create  error\n",
                __FUNCTION__);
        return -ENOMEM;
    }

    for (idx = 0; idx < hash_tab_size; idx++)
        INIT_LIST_HEAD(&hash_tab[idx]);

    for (idx = 0; idx < CT_LOCKARRAY_SIZE; idx++)
        rwlock_init(&hash_lock_array[idx].l);

	init_timer(&print_memory);
	print_memory.expires = jiffies + 10*HZ;
	print_memory.data = 0;
	print_memory.function = print_memory_usage;
    add_timer(&print_memory);
   
	 
	cpunum = num_online_cpus();
	
	for (idx = 0; idx < hash_tab_size; idx++) {
		init_timer(bucket_clear+idx);
		(bucket_clear+idx)->expires = jiffies + timeout_hash_del;
		(bucket_clear+idx)->data = idx;
		(bucket_clear+idx)->function = bucket_clear_item;
   		add_timer_on(bucket_clear+idx, idx%cpunum);
	}
	
	return 0;
}

static void hash_del_item(struct hashinfo_item *cp)
{
    atomic_inc(&cp->refcnt);
    if (likely(atomic_read(&cp->refcnt) == 2)) {
        uint32_t hash, bkt;
		HASH_FCN(cp->sha1, SHA1SIZE, hash_tab_size, hash, bkt);
        _unhash(bkt, cp);
        if (likely(atomic_read(&cp->refcnt) == 1)){
            //if (timer_pending(&cp->timer))
            //	del_timer(&cp->timer);
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
    unsigned long idx;
    struct hashinfo_item *cp, *next;
    
	for (idx = 0; idx < hash_tab_size; idx++) {
        ct_write_lock_bh(idx, hash_lock_array);
        list_for_each_entry_safe(cp, next, &hash_tab[idx], c_list) {
    		list_del(&cp->c_list);
			atomic_dec(&hash_count);
            kmem_cache_free(hash_cachep, cp);
        }
        ct_write_unlock_bh(idx, hash_lock_array);
    }

    DEBUG_LOG(KERN_INFO "%s\n", __FUNCTION__ );
}

void release_hash_table_cache(void)
{
	unsigned long idx;
    hash_flush();
	
	del_timer_sync(&print_memory);
	
	for (idx = 0; idx < hash_tab_size; idx++) {
			del_timer_sync(bucket_clear + idx);
	}
    
	kmem_cache_destroy(hash_cachep);
    vfree(hash_tab);

    DEBUG_LOG(KERN_INFO "%s\n", __FUNCTION__);
}

/*
 * this for every item timer, we don't use it now.
 */
void hash_item_expire(unsigned long data)
{
	struct hashinfo_item *cp = (struct hashinfo_item *)data;
    DEBUG_LOG(KERN_INFO "count is:%d", atomic_read(&hash_count));
	if (likely(atomic_read(&cp->refcnt) == 1)) {
		/*
         * delete it.
         */
		hash_del_item(cp);
	}
	else {
		/*
         * mod timer and desc refcnt.
         */
    	atomic_dec(&cp->refcnt);
		//mod_timer(&cp->timer, jiffies + timeout_hash_del);
	}
}

void bucket_clear_item(unsigned long data)
{
    struct hashinfo_item *cp, *next;

    ct_write_lock_bh(data, hash_lock_array);
	list_for_each_entry_safe(cp, next, &hash_tab[data], c_list) {
   		if (atomic_read(&cp->refcnt) <= 1) {
			list_del(&cp->c_list);
			atomic_dec(&hash_count);

            kmem_cache_free(hash_cachep, cp);
			DEBUG_LOG(KERN_INFO "delete it.");
			continue;
		}
	
		atomic_dec(&cp->refcnt);
	}
    ct_write_unlock_bh(data, hash_lock_array);

	mod_timer((bucket_clear+data), jiffies + timeout_hash_del);
	DEBUG_LOG(KERN_INFO "%s\n", __FUNCTION__ );
}
