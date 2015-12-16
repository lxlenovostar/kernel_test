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
#define ITEM_CITE_ADD   6 
#define ITEM_CITE_FIND  6
#define ITEM_DISK_LIMIT 60 
#define ITEM_VIP_LIMIT  120 
unsigned long timeout_hash_del = 10*HZ;
uint32_t hash_tab_size  = (1<<WS_SP_HASH_TABLE_BITS);
uint32_t hash_tab_mask  = ((1<<WS_SP_HASH_TABLE_BITS)-1);

static struct list_head *hash_tab = NULL;

/* SLAB cache for hash item */
static struct kmem_cache * hash_cachep/* __read_mostly*/;

/* counter for hash item */
atomic_t hash_count = ATOMIC_INIT(0);
unsigned long long hash_max_count = (2ULL*1024*1024*1024) / sizeof(struct hashinfo_item); /*2GB hash_table memory usage.*/


static struct _aligned_lock hash_lock_array[CT_LOCKARRAY_SIZE];

static struct timer_list print_memory;
struct workqueue_struct *writeread_wq; // for read/write file

void hash_item_expire(unsigned long data);
void bucket_clear_item(unsigned long data);

w_work_t w_work[1<<WS_SP_HASH_TABLE_BITS];

atomic64_t mm0;
atomic64_t mm1;
atomic64_t mm2;
atomic64_t mm3;

struct percpu_counter mmw;
struct percpu_counter mmd;


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

void alloc_data_memory(struct hashinfo_item *cp, size_t length)
{
	if (atomic_read(&cp->mem_style) >= 0) {
		return;
	}

	if (length <= CHUNKSTEP) {
		cp->data  = kmem_cache_zalloc(slab_chunk1, GFP_ATOMIC);  
		if (!cp->data) {
        	DEBUG_LOG(KERN_ERR"****** %s : malloc cp->data error\n", __FUNCTION__);
			BUG();	//TODO:	maybe other good way fix it.
		}
		atomic_set(&cp->mem_style, 1);
		atomic64_add(CHUNKSTEP, &mm1);
		return;
	} else if (length <= CHUNKSTEP*2) {
		cp->data  = kmem_cache_zalloc(slab_chunk2, GFP_ATOMIC);  
		if (!cp->data) {
        	DEBUG_LOG(KERN_ERR"****** %s : malloc cp->data error\n", __FUNCTION__);
			BUG();	//TODO:	maybe other good way fix it.
		}
		atomic_set(&cp->mem_style, 2);
		atomic64_add(CHUNKSTEP*2, &mm2);
		return;
	} else if (length <= CHUNKSTEP*3) {
		cp->data  = kmem_cache_zalloc(slab_chunk3, GFP_ATOMIC);  
		if (!cp->data) {
        	DEBUG_LOG(KERN_ERR"****** %s : malloc cp->data error\n", __FUNCTION__);
			BUG();	//TODO:	maybe other good way fix it.
		}
		atomic_set(&cp->mem_style, 3);
		atomic64_add(CHUNKSTEP*3, &mm3);
		return;
	} else {	
		cp->data = kmalloc(cp->len, GFP_ATOMIC);
		if (!cp->data) {
        	DEBUG_LOG(KERN_ERR"****** %s : malloc cp->data error\n", __FUNCTION__);
			BUG();	//TODO:	maybe other good way fix it.
		}
		atomic_set(&cp->mem_style, 0);
		atomic64_add(cp->len, &mm0);
		return;
	}
}

static void free_data_memory(struct hashinfo_item *cp) 
{
	if (atomic_read(&cp->mem_style) == 0) {
		kfree(cp->data);	
		atomic_set(&cp->mem_style, -1);
		atomic64_sub(cp->len, &mm0);
		return;
	} else if (atomic_read(&cp->mem_style) == 1) {
		kmem_cache_free(slab_chunk1, cp->data);
		atomic_set(&cp->mem_style, -1);
		atomic64_sub(CHUNKSTEP, &mm1);
		return;
	} else if (atomic_read(&cp->mem_style) == 2) {
		kmem_cache_free(slab_chunk2, cp->data);
		atomic_set(&cp->mem_style, -1);
		atomic64_sub(CHUNKSTEP*2, &mm2);
		return;
	} else if (atomic_read(&cp->mem_style) == 3) {
		kmem_cache_free(slab_chunk3, cp->data);
		atomic_set(&cp->mem_style, -1);
		atomic64_sub(CHUNKSTEP*3, &mm3);
		return;
	} else {
		//do nothing.
		int status = atomic_read(&cp->flag_cache);
		int mem_style = atomic_read(&cp->mem_style);
        printk(KERN_ERR"****** %s : you can't arrive here. mem_style is:%d, and flag_cache is:%d", __FUNCTION__, mem_style, status);
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

	cp->len = len_value;
	atomic_set(&cp->mem_style, -1);    
	atomic_set(&cp->flag_cache, 0);    
	atomic_set(&cp->flag_mem, 0);    
	cp->data_lock = SPIN_LOCK_UNLOCKED;
	alloc_data_memory(cp, cp->len);
	memcpy(cp->data, value, cp->len);
	
    rwlock_init(&cp->cache_lock);

	cp->cpuid = -1;
	cp->store_flag = 0;
	
	INIT_LIST_HEAD(&cp->c_list);
	memcpy(cp->sha1, info, SHA1SIZE);
	atomic_set(&cp->refcnt, ITEM_CITE_ADD);    
	atomic_set(&cp->share_ref, 1);    
    rwlock_init(&cp->share_lock);
 
   	/*
   	 * total hash item.
   	 */
	atomic_inc(&hash_count);

	/*
   	 * insert into the hash table.
   	 */
	HASH_FCN(cp->sha1, SHA1SIZE, hash_tab_size, hash, bkt);
	//unsigned long flags;
	//local_irq_save(flags);
   	_hash(bkt, cp);
	//local_irq_restore(flags);

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
			//write_lock_bh(&cp->share_lock);
			atomic_inc(&cp->share_ref);
			//write_unlock_bh(&cp->share_lock);
			
			//write_lock_bh(&cp->cache_lock);
			if (atomic_read(&cp->flag_cache) == 1) {
				atomic_set(&cp->flag_cache, 4); 
			}
			//write_unlock_bh(&cp->cache_lock);

			ct_read_unlock_bh(hash, hash_lock_array);
			return cp; 
        }   
    }   
    ct_read_unlock_bh(hash, hash_lock_array);
    return NULL;
}

unsigned long long old_write_mm = 0ULL;
unsigned long long old_save = 0ULL;
unsigned long long old_sum = 0ULL;
long old_skb_sum = 0L;
int time_intval = 10;

void print_memory_usage(unsigned long data)
{
	int slot_size = hash_tab_size * sizeof(struct list_head);
   	uint32_t hash_count_now = atomic_read(&hash_count);
	int item_size = hash_count_now * sizeof(struct hashinfo_item); 
	long data_mem = atomic64_read(&mm0) + atomic64_read(&mm1) + atomic64_read(&mm2) + atomic64_read(&mm3);
	unsigned long long write_mm = percpu_counter_sum(&mmw)/1024/1024;
	unsigned long long write_d_mm = percpu_counter_sum(&mmd)/1024/1024;
	long tmp_save = atomic64_read(&save_num)/1024/1024;
	long tmp_sum =  atomic64_read(&sum_num)/1024/1024;
	long tmp_skb_sum =  atomic64_read(&skb_num);
	long read_data = atomic64_read(&rdl)/1024/1024;
	long read_frequency = atomic64_read(&rdf);

	printk(KERN_INFO "\n[memory usage]");	
	printk(KERN_INFO "memory usage is:%dMB, data memmory is:%ldMB, all memory is:%ldMB, item number is:%u", (item_size + slot_size)/1024/1024, data_mem/1024/1024, ((item_size + slot_size)/1024/1024 + data_mem/1024/1024), hash_count_now);

	printk(KERN_INFO "[write file]");	
	printk(KERN_INFO "write data is:%lluMB, data miss store is:%lluMB", write_mm, write_d_mm);
	
	printk(KERN_INFO "[read file]");	
	printk(KERN_INFO "read data is:%ldMB, number of times is:%ld", read_data, read_frequency);
	
	printk(KERN_INFO "[speed]");	
	printk(KERN_INFO "save rate is:%lluMB/s sum rate is:%lluMB/s write rate is:%lluMB/s", (tmp_save - old_save)/time_intval, (tmp_sum - old_sum)/time_intval, (write_mm - old_write_mm)/time_intval);

	printk(KERN_INFO "[packets]");	
	printk(KERN_INFO "packet num is:%ld pps", (tmp_skb_sum - old_skb_sum)/time_intval);
	if (tmp_sum > 0) {
		printk(KERN_INFO "[cache ratio]");	
		printk(KERN_INFO "save bytes is:%ldMB, all bytes is:%ldMB, Cache ratio is:%ld%%", tmp_save, tmp_sum, (tmp_save*100)/tmp_sum);
	}

	old_write_mm = write_mm;
	old_save = tmp_save;
	old_sum = tmp_sum;
	old_skb_sum = tmp_skb_sum;

	mod_timer(&print_memory, jiffies + timeout_hash_del);
}

int initial_hash_table_cache(void)
{
    unsigned long idx;
	int cpu;
    
	writeread_wq = create_workqueue("kwrite_queue");
	if (!writeread_wq)
		return -1;

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
	
	atomic64_set(&mm0, 0L);
	atomic64_set(&mm1, 0L);
	atomic64_set(&mm2, 0L);
	atomic64_set(&mm3, 0L);
	
	percpu_counter_init(&mmw, 0ULL);
	percpu_counter_init(&mmd, 0ULL);
	
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
			if (atomic_read(&cp->flag_cache) == 0 || atomic_read(&cp->flag_cache) == 2 || atomic_read(&cp->flag_cache) == 3 || atomic_read(&cp->flag_cache) == 4) 
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
    hash_flush();
	
	del_timer_sync(&print_memory);

	percpu_counter_destroy(&mmw);
	percpu_counter_destroy(&mmd);
	
	for (idx = 0; idx < hash_tab_size; idx++) {
			del_timer_sync(bucket_clear + idx);
	}

	flush_workqueue(writeread_wq);
	destroy_workqueue(writeread_wq);
    
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
	unsigned long st_all_size = 0;
	unsigned long data = _work->index;

 	cpu = get_cpu();
    ct_read_lock_bh(data, hash_lock_array);
	/*
	 * sum the all bytes.
	 */
	list_for_each_entry_safe(cp, next, &hash_tab[data], c_list) {
		//read_lock_bh(&cp->share_lock);
		if (atomic_read(&cp->flag_cache) == 2 && cp->cpuid == -1 && atomic_read(&cp->share_ref) == 1) {
			//for statistics.
			if (cp->store_flag == 1) {
				cp->store_flag = 2;
			}

			if (cp->len <= CHUNKSTEP) {
				num = 1;
			}
			else {
				num = DIV_ROUND_UP(cp->len, CHUNKSTEP);
			}
			all_size += num*CHUNKSTEP;
			st_all_size += cp->len;

			/*
			 * 使用cp->cpu来标识，防止有些chunk开始不用写，后面又需要写的情况。
			 */
			cp->cpuid = -2;	
		}
		//read_unlock_bh(&cp->share_lock);
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
			//read_lock_bh(&cp->share_lock);
			/*
			 * chunk 又被引用，这种情况保持状态2，不写文件  
			 */
			if (cp->cpuid == -2 && atomic_read(&cp->flag_cache) == 2 && atomic_read(&cp->share_ref) > 1) { 
				cp->cpuid = -1;
				//read_unlock_bh(&cp->share_lock);
				continue;
			}
			
			if (cp->cpuid == -2 &&  atomic_read(&cp->flag_cache) == 2 && atomic_read(&cp->share_ref) == 1) {
				
				if (cp->len <= CHUNKSTEP)
					num = 1;
				else
					num = DIV_ROUND_UP(cp->len, CHUNKSTEP);

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
			 	 * cp->data copy.
			 	 */
				memcpy(copy_mem + mem_index, cp->data, cp->len);
				mem_index += num*CHUNKSTEP;
			}
			//read_unlock_bh(&cp->share_lock);
		}
	}
    ct_read_unlock_bh(data, hash_lock_array);
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
	percpu_counter_add(&mmw, st_all_size);

	/*
	 * free memory.
	 */
	kfree(copy_mem);
 
    ct_read_lock_bh(data, hash_lock_array);
	list_for_each_entry_safe(cp, next, &hash_tab[data], c_list) {
		//read_lock_bh(&cp->share_lock);
		//write_lock_bh(&cp->cache_lock);
		if (atomic_read(&cp->flag_cache) == 2 && atomic_read(&cp->share_ref) == 1 && cp->cpuid >= 0) {
			atomic_set(&cp->flag_cache, 1);    
			free_data_memory(cp);
		}		
		//write_unlock_bh(&cp->cache_lock);
		//read_unlock_bh(&cp->share_lock);
	}
    ct_read_unlock_bh(data, hash_lock_array);
}

void bucket_clear_item(unsigned long data)
{
    struct hashinfo_item *cp, *next;
	int i, num;
    int flag = 0;
	//int threshold = 100;

    ct_write_lock_bh(data, hash_lock_array);
    //if (ct_write_trylock_bh(data, hash_lock_array)) {
	list_for_each_entry_safe(cp, next, &hash_tab[data], c_list) {
		//read_lock_bh(&cp->share_lock);
		//write_lock_bh(&cp->cache_lock);
	
		/*	
		if (threshold == 0)
				break;
		else 
			threshold--;
		*/

   		if (atomic_read(&cp->share_ref) == 1 && atomic_read(&cp->refcnt) <= 1) {
			list_del(&cp->c_list);
			atomic_dec(&hash_count);
			if (atomic_read(&cp->flag_cache) == 0 || atomic_read(&cp->flag_cache) == 2 || atomic_read(&cp->flag_cache) == 3 || atomic_read(&cp->flag_cache) == 4) 
			//if (atomic_read(&cp->flag_cache) == 0 || atomic_read(&cp->flag_cache) == 2 || atomic_read(&cp->flag_cache) == 3) 
				free_data_memory(cp);
			/*
		 	 * decide whether the data write into file by cp->cpuid.
			 */
			if (atomic_read(&cp->flag_cache) == 1 || (atomic_read(&cp->flag_cache) == 2 && cp->cpuid >= 0) || atomic_read(&cp->flag_cache) == 4) {
			//if (atomic_read(&cp->flag_cache) == 1 || (atomic_read(&cp->flag_cache) == 2 && cp->cpuid >= 0)) {
				if (cp->len <= CHUNKSTEP)
					num = 1;
				else
					num = DIV_ROUND_UP(cp->len, CHUNKSTEP);
			
				for (i = 0; i < num; ++i) {
					clear_bit(cp->start + i, per_cpu(bitmap, cp->cpuid));	
				} 
			}

			if (cp->store_flag == 1) {
				percpu_counter_add(&mmd, cp->len);
			}

			//write_unlock_bh(&cp->cache_lock);
			//read_unlock_bh(&cp->share_lock);
            kmem_cache_free(hash_cachep, cp);
			DEBUG_LOG(KERN_INFO "delete it.");
			continue;
		}
	
		atomic_dec(&cp->refcnt);
	
		/*
         * start the status machine.
         */	
		if (atomic_read(&cp->refcnt) >= ITEM_VIP_LIMIT && atomic_read(&cp->flag_cache) == 0)
			atomic_set(&cp->flag_cache, 3); 
		
		if (atomic_read(&cp->refcnt) < ITEM_VIP_LIMIT && atomic_read(&cp->refcnt) >= ITEM_DISK_LIMIT && atomic_read(&cp->flag_cache) == 0)
			atomic_set(&cp->flag_cache, 2); 

		if (flag == 0 && atomic_read(&cp->refcnt) >= ITEM_DISK_LIMIT && atomic_read(&cp->refcnt) < ITEM_VIP_LIMIT) {
			flag = 1;
		}
			
		if (atomic_read(&cp->refcnt) >= ITEM_DISK_LIMIT) { 
			//just for statistics.
			if (cp->store_flag == 0) {
				cp->store_flag = 1;
			}
		}
		//write_unlock_bh(&cp->cache_lock);
		//read_unlock_bh(&cp->share_lock);
	}
	//}
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
