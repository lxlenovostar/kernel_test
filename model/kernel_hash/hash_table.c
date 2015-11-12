#include <asm/types.h>
#include <linux/version.h>
#include <linux/mm.h>
#include "hash_lock.h"
#include "hash_table.h"
#include "debug.h"
#include "chunk.h"
#include "bitmap.h"
#include "file.h"

#define WS_SP_HASH_TABLE_BITS 20
//#define ITEM_CITE_ADD 10
//#define ITEM_CITE_FIND 10
//#define ITEM_DISK_LIMIT 10
#define ITEM_CITE_ADD   1 
#define ITEM_CITE_FIND  1
#define ITEM_DISK_LIMIT 1
unsigned long timeout_hash_del = 5*HZ;
uint32_t hash_tab_size  = (1<<WS_SP_HASH_TABLE_BITS);
uint32_t hash_tab_mask  = ((1<<WS_SP_HASH_TABLE_BITS)-1);

static struct list_head *hash_tab = NULL;

/* SLAB cache for hash item */
static struct kmem_cache * hash_cachep/* __read_mostly*/;

/* counter for hash item */
atomic_t hash_count = ATOMIC_INIT(0);
unsigned long long hash_max_count = (2UL*1024*1024*1024) / sizeof(struct hashinfo_item); /*2GB hash_table memory usage.*/


static struct _aligned_lock hash_lock_array[CT_LOCKARRAY_SIZE];

static struct timer_list print_memory;

void hash_item_expire(unsigned long data);
void bucket_clear_item(unsigned long data);

w_work_t w_work[1<<WS_SP_HASH_TABLE_BITS];

struct percpu_counter mm0;
struct percpu_counter mm1;
struct percpu_counter mm2;
struct percpu_counter mm3;

struct kmem_cache * slab_chunk1;
struct kmem_cache * slab_chunk2;
struct kmem_cache * slab_chunk3;
extern unsigned long bitmap_size;
//extern unsigned long *bitmap;
DECLARE_PER_CPU(unsigned long *, bitmap); //percpu-BITMAP
DECLARE_PER_CPU(unsigned long, bitmap_index); //percpu-BITMAP-index
DECLARE_PER_CPU(struct file *, reserve_file); 
DECLARE_PER_CPU(loff_t, loff_file); 
static struct timer_list *bucket_clear; 
//DEFINE_PER_CPU(struct timer_list *, bucket_clear); 
int cpunum = 0;

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

static inline uint32_t reset_hash(uint32_t hash, struct hashinfo_item *cp)
{
    ct_write_lock_bh(hash, hash_lock_array);
	atomic_set(&cp->refcnt, 1);
    ct_write_unlock_bh(hash, hash_lock_array);
    return 1;
}

static void alloc_data_memory(struct hashinfo_item *cp, size_t length)
{
	if (length <= CHUNKSTEP) {
		cp->data  = kmem_cache_zalloc(slab_chunk1, GFP_ATOMIC);  
		if (!cp->data) {
        	DEBUG_LOG(KERN_ERR"****** %s : malloc cp->data error\n", __FUNCTION__);
			BUG();	//TODO:	maybe other good way fix it.
		}
		cp->mem_style = 1;
		percpu_counter_add(&mm1, CHUNKSTEP);
		return;
	} else if (length <= CHUNKSTEP*2) {
		cp->data  = kmem_cache_zalloc(slab_chunk2, GFP_ATOMIC);  
		if (!cp->data) {
        	DEBUG_LOG(KERN_ERR"****** %s : malloc cp->data error\n", __FUNCTION__);
			BUG();	//TODO:	maybe other good way fix it.
		}
		cp->mem_style = 2;
		percpu_counter_add(&mm2, CHUNKSTEP*2);
		return;
	} else if (length <= CHUNKSTEP*3) {
		cp->data  = kmem_cache_zalloc(slab_chunk3, GFP_ATOMIC);  
		if (!cp->data) {
        	DEBUG_LOG(KERN_ERR"****** %s : malloc cp->data error\n", __FUNCTION__);
			BUG();	//TODO:	maybe other good way fix it.
		}
		cp->mem_style = 3;
		percpu_counter_add(&mm3, CHUNKSTEP*3);
		return;
	} else {	
		cp->data = kmalloc(cp->len, GFP_ATOMIC);
		if (!cp->data) {
        	DEBUG_LOG(KERN_ERR"****** %s : malloc cp->data error\n", __FUNCTION__);
			BUG();	//TODO:	maybe other good way fix it.
		}
		cp->mem_style = 0;
		percpu_counter_add(&mm0, cp->len);
		return;
	}
}

static void free_data_memory(struct hashinfo_item *cp) 
{
	if (cp->mem_style == 0) {
		kfree(cp->data);	
		percpu_counter_add(&mm0, -(cp->len));
	} else if (cp->mem_style == 1) {
		kmem_cache_free(slab_chunk1, cp->data);
		percpu_counter_add(&mm1, -(CHUNKSTEP));
	} else if (cp->mem_style == 2) {
		kmem_cache_free(slab_chunk2, cp->data);
		percpu_counter_add(&mm2, -(CHUNKSTEP*2));
	} else if (cp->mem_style == 3) {
		kmem_cache_free(slab_chunk3, cp->data);
		percpu_counter_add(&mm3, -(CHUNKSTEP*3));
	} else {
		//do nothing.
        DEBUG_LOG(KERN_ERR"****** %s : you can't arrive here.\n", __FUNCTION__);
		BUG();	//TODO:	maybe other good way fix it.
	}
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

	/*
     * handle the value.
     */
	cp->len = len_value;
	alloc_data_memory(cp, len_value);
	memcpy(cp->data, value, cp->len);
	
	cp->flag_cahce = 0;	

	cp->cpuid = -1;
	
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

void print_memory_usage(unsigned long data)
{
	unsigned long tmp_save, tmp_sum;	
	int slot_size = hash_tab_size * sizeof(struct list_head);
   	uint32_t hash_count_now = atomic_read(&hash_count);
	int item_size = hash_count_now * sizeof(struct hashinfo_item); 
	unsigned long long data_mem = percpu_counter_read(&mm0) + percpu_counter_read(&mm1) + percpu_counter_read(&mm2) + percpu_counter_read(&mm3);
	tmp_save = percpu_counter_read(&save_num);
	tmp_sum =  percpu_counter_read(&sum_num);


	//printk(KERN_INFO "max hash count is:%llu and max ull is:%llu, %s", hash_max_count, ULLONG_MAX, (hash_max_count>ULLONG_MAX)?"gt":"lt");

	printk(KERN_INFO "memory usage is:%dMB, data memmory is:%lluMB, item number is:%u", (item_size + slot_size)/1024/1024, data_mem/1024/1024, hash_count_now);

	if (tmp_sum > 0)
		printk(KERN_INFO "save bytes is:%lu Bytes %lu MB, all bytes is:%lu Bytes %lu MB, Cache ratio is:%lu%%", tmp_save,(tmp_save/1024/1024), tmp_sum, (tmp_sum/1024/1024), (tmp_save*100)/tmp_sum);
	
	mod_timer(&print_memory, jiffies + timeout_hash_del);
}

int initial_hash_table_cache(void)
{
    unsigned long idx;
	int cpu;
    
	hash_tab = vmalloc(hash_tab_size * sizeof(struct list_head));
    if (!hash_tab) {
        DEBUG_LOG(KERN_ERR"****** %s : vmalloc tab error\n", __FUNCTION__);
        return -ENOMEM;
    }

	bucket_clear  = vmalloc(hash_tab_size * sizeof(struct timer_list));
   	if (!bucket_clear) {
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
   
	 
	//cpunum = num_online_cpus;
	for_each_online_cpu(cpu) 	
		cpunum++;
	
	for (idx = 0; idx < hash_tab_size; idx++) {
		init_timer(bucket_clear+idx);
		(bucket_clear+idx)->expires = jiffies + timeout_hash_del;
		(bucket_clear+idx)->data = idx;
		(bucket_clear+idx)->function = bucket_clear_item;
   		add_timer_on(bucket_clear+idx, idx%cpunum);
	}
	
	percpu_counter_init(&mm0, 0);
	percpu_counter_init(&mm1, 0);
	percpu_counter_init(&mm2, 0);
	percpu_counter_init(&mm3, 0);
	
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
			free_data_memory(cp);
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
			if (cp->flag_cahce == 0 || cp->flag_cahce == 2) 
				free_data_memory(cp);
            kmem_cache_free(hash_cachep, cp);
        }
        ct_write_unlock_bh(idx, hash_lock_array);
    }

    DEBUG_LOG(KERN_INFO "%s\n", __FUNCTION__ );
}

void release_hash_table_cache(void)
{
	unsigned long idx;
	int cpu;
    hash_flush();
	
	del_timer_sync(&print_memory);
	percpu_counter_destroy(&mm0);
	percpu_counter_destroy(&mm1);
	percpu_counter_destroy(&mm2);
	percpu_counter_destroy(&mm3);
	
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

/*
 * write file and free data in the memory.
 */
static void wr_file(struct work_struct *work)
{
	struct hashinfo_item *cp, *next;
	w_work_t *_work = (w_work_t *)work;
	int num, find_index, cpu, i, ret;
	char *copy_mem;
	unsigned long mem_index = 0;
	unsigned long all_size = 0;
	unsigned long data = _work->index;

 	cpu = get_cpu();
    ct_write_lock_bh(data, hash_lock_array);
	/*
	 * sum the all bytes.
	 */
	list_for_each_entry_safe(cp, next, &hash_tab[data], c_list) {
		if (atomic_read(&cp->refcnt) >= ITEM_DISK_LIMIT && cp->flag_cahce == 0) {
			if (cp->len <= CHUNKSTEP) {
				num = 1;
			}
			else {
				num = cp->len/CHUNKSTEP + ((cp->len%CHUNKSTEP == 0) ? 0 : 1);
				//num = (cp->len + CHUNKSTEP - 1)/CHUNKSTEP;
			}
			all_size += num*CHUNKSTEP;
		}
	}

	if (all_size != 0) {
		/*
	 	* alloc memory for copy data.
	 	*/	
		copy_mem = kzalloc(all_size, GFP_ATOMIC);
    	if (!copy_mem) {
        	DEBUG_LOG(KERN_ERR"****** %s : vmalloc  copy_mem error\n", __FUNCTION__);
        	BUG(); //TODO need update it.
    	}

    	list_for_each_entry_safe(cp, next, &hash_tab[data], c_list) {
			if (atomic_read(&cp->refcnt) >= ITEM_DISK_LIMIT && cp->flag_cahce == 0) {
				if (cp->len <= CHUNKSTEP)
					num = 1;
				else
					num = cp->len/CHUNKSTEP + ((cp->len%CHUNKSTEP == 0) ? 0 : 1);

				/*
             	 * find the bitmap position.
             	 */			
				find_index = bitmap_find_next_zero_area(per_cpu(bitmap, cpu), bitmap_size, per_cpu(bitmap_index, cpu), num, 0);
				if (find_index > bitmap_size) {
					//TODO: 后续这里处理特殊处理，将一些零散的数据整合到一块。
					BUG();
				}
			
				cp->start = find_index;
				cp->cpuid = cpu;
				
				/*
             	 * set bitmap.
             	 */
				for (i = 0; i < num; ++i) {
					set_bit(cp->start + i, per_cpu(bitmap, cpu));	
				} 

				/*
			 	 * update bitmap index for next item.
			 	 */
				per_cpu(bitmap_index, cpu) += num;

				/*
			 	 * set the status of item. 2 stands for data will write to file.
			 	 */
				cp->flag_cahce = 2;

				/*
			 	 * cp->data copy.
			 	 */
				memcpy(copy_mem + mem_index, cp->data, cp->len);
				mem_index += num*CHUNKSTEP;
			}
		}
	}
    ct_write_unlock_bh(data, hash_lock_array);
	put_cpu();
	
	if (all_size == 0) 
		return;
	
	/*
     * write file.
	 */
	cpu = get_cpu();
	ret = kernel_write(per_cpu(reserve_file, cpu), copy_mem, all_size, per_cpu(loff_file, cpu));
	if (ret < 0) {
    	printk(KERN_ERR "Error writing file:%d\n", cpu);
		BUG(); //TODO need updat it.
	}
	
    //TODO: file size maybe so big?
	per_cpu(loff_file, cpu) += all_size;
	put_cpu();		

	kfree(copy_mem);
 
    ct_write_lock_bh(data, hash_lock_array);
	list_for_each_entry_safe(cp, next, &hash_tab[data], c_list) {
		if (cp->flag_cahce == 2) {
			cp->flag_cahce = 1;
			free_data_memory(cp);
		}		
	}
    ct_write_unlock_bh(data, hash_lock_array);
}

void bucket_clear_item(unsigned long data)
{
    struct hashinfo_item *cp, *next;
	int i, num;
    int flag = 0;

    ct_write_lock_bh(data, hash_lock_array);
    list_for_each_entry_safe(cp, next, &hash_tab[data], c_list) {
   		if (atomic_dec_and_test(&cp->refcnt)) {
			list_del(&cp->c_list);
			atomic_dec(&hash_count);
			if (cp->flag_cahce == 0 || cp->flag_cahce == 2) 
				free_data_memory(cp);
			if (cp->flag_cahce == 1 || cp->flag_cahce == 2) {
				if (cp->len <= CHUNKSTEP)
					num = 1;
				else
					num = cp->len/CHUNKSTEP + ((cp->len%CHUNKSTEP == 0) ? 0 : 1);
			
				for (i = 0; i < num; ++i) {
					//TODO: maybe???
					clear_bit(cp->start + i, per_cpu(bitmap, cp->cpuid));	
				} 
			}
            kmem_cache_free(hash_cachep, cp);
			DEBUG_LOG(KERN_INFO "delete it.");
			continue;
		}
	
		atomic_dec(&cp->refcnt);
		if (flag == 0 && atomic_read(&cp->refcnt) >= ITEM_DISK_LIMIT) 
			flag = 1;
	}
    ct_write_unlock_bh(data, hash_lock_array);

	/*
     * work join in workqueue.
     */
	if (flag) { 
		if (!work_pending((struct work_struct *)(w_work+data))) {
			INIT_WORK((struct work_struct *)(w_work+data), wr_file);
			(w_work+data)->index = data;
			queue_work(writeread_wq, (struct work_struct *)(w_work+data));
		} 
	}

	mod_timer((bucket_clear+data), jiffies + timeout_hash_del);

	DEBUG_LOG(KERN_INFO "%s\n", __FUNCTION__ );
}

/*
void bucket_clear_item(unsigned long data)
{
    struct hashinfo_item *cp, *next;
    
    ct_write_lock_bh(data, hash_lock_array);
    list_for_each_entry_safe(cp, next, &hash_tab[data], c_list) {
   		if (likely(atomic_read(&cp->refcnt) == 1)) {
			list_del(&cp->c_list);
			atomic_dec(&hash_count);
			free_data_memory(cp);
            kmem_cache_free(hash_cachep, cp);
			DEBUG_LOG(KERN_INFO "delete it.");
		}
		else { 
    		atomic_dec(&cp->refcnt);
		}
	}
    ct_write_unlock_bh(data, hash_lock_array);
	mod_timer((bucket_clear+data), jiffies + timeout_hash_del);

    DEBUG_LOG(KERN_INFO "%s\n", __FUNCTION__ );
}*/
