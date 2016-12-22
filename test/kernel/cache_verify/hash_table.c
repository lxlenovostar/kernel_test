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
#define ITEM_DISK_LIMIT 1000 
#define ITEM_VIP_LIMIT 60 
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

struct percpu_counter mm0;
struct percpu_counter mm1;
struct percpu_counter mm2;
struct percpu_counter mm3;
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
	cp->cpuid = -1;
	INIT_LIST_HEAD(&cp->c_list);
	memcpy(cp->sha1, info, SHA1SIZE);
	atomic_set(&cp->refcnt, ITEM_CITE_ADD);    
	atomic_set(&cp->data_refcnt, 1);    
    
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

struct hashinfo_item *get_hash_item(uint8_t *info, char *value, size_t len_value)
{
    uint32_t hash, bkt;
    struct hashinfo_item *cp;

	HASH_FCN(info, SHA1SIZE, hash_tab_size, hash, bkt);
    ct_read_lock_bh(hash, hash_lock_array);
    list_for_each_entry(cp, &hash_tab[bkt], c_list) {
		if (memcmp(cp->sha1, info, SHA1SIZE) == 0) {
			/*
			 * check the SHA1 and value.
			 */
			if (len_value != cp->len ||  memcmp(cp->data, value, len_value) != 0) {
				int i;
				printk("\nnew data len is:%d\n", len_value);
				for (i = 0; i < len_value; ++i)
					printk("%02x:", value[i]&0xff);
				printk("\n");
				
				printk("old data len is:%d\n", cp->len);
				for (i = 0; i < cp->len; ++i)
					printk("%02x:", (cp->data)[i]&0xff);
				printk("\n");
			}			

   			DEBUG_LOG(KERN_INFO "find it:%s\n", __FUNCTION__ );
            atomic_add(ITEM_CITE_FIND, &cp->refcnt);
			atomic_inc(&cp->data_refcnt);    
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
int time_intval = 10;

void print_memory_usage(unsigned long data)
{
	unsigned long long tmp_save, tmp_sum;	
	int slot_size = hash_tab_size * sizeof(struct list_head);
   	uint32_t hash_count_now = atomic_read(&hash_count);
	int item_size = hash_count_now * sizeof(struct hashinfo_item); 
	unsigned long long data_mem = percpu_counter_sum(&mm0) + percpu_counter_sum(&mm1) + percpu_counter_sum(&mm2) + percpu_counter_sum(&mm3);
	//unsigned long long write_mm = percpu_counter_sum(&mmw)/1024/1024;
	unsigned long long write_mm = percpu_counter_sum(&mmw);
	unsigned long long write_d_mm = percpu_counter_sum(&mmd)/1024/1024;
	tmp_save = percpu_counter_sum(&save_num)/1024/1024;
	tmp_sum =  percpu_counter_sum(&sum_num)/1024/1024;

	/*
	printk(KERN_INFO "\n[memory usage]");	
	printk(KERN_INFO "memory usage is:%dMB, data memmory is:%lluMB, all memory is:%lluMB, item number is:%u", (item_size + slot_size)/1024/1024, data_mem/1024/1024, ((item_size + slot_size)/1024/1024 + data_mem/1024/1024), hash_count_now);

	printk(KERN_INFO "[write file]");	
	printk(KERN_INFO "write data is:%lluBytes,%lluMB, data miss store is:%lluMB", write_mm, write_mm/1024/1024, write_d_mm);
	
	printk(KERN_INFO "[speed]");	
	printk(KERN_INFO "save rate is:%lluMB/s sum rate is:%lluMB/s write rate is:%lluMB/s", (tmp_save - old_save)/time_intval, (tmp_sum - old_sum)/time_intval, (write_mm/1024/1024 - old_write_mm/1024/1024)/time_intval);

	if (tmp_sum > 0) {
		printk(KERN_INFO "[cache ratio]");	
		printk(KERN_INFO "save bytes is:%lluMB, all bytes is:%lluMB, Cache ratio is:%llu%%", tmp_save, tmp_sum, (tmp_save*100)/tmp_sum);
	}
	*/

	old_write_mm = write_mm;
	old_save = tmp_save;
	old_sum = tmp_sum;

	mod_timer(&print_memory, jiffies + timeout_hash_del);
}

int initial_hash_table_cache(void)
{
    unsigned long idx;
	int cpu;
    
	writeread_wq = create_workqueue("wr_queue");
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
	
	percpu_counter_init(&mm0, 0ULL);
	percpu_counter_init(&mm1, 0ULL);
	percpu_counter_init(&mm2, 0ULL);
	percpu_counter_init(&mm3, 0ULL);
	
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

	percpu_counter_destroy(&mm0);
	percpu_counter_destroy(&mm1);
	percpu_counter_destroy(&mm2);
	percpu_counter_destroy(&mm3);

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
	int cpu, ret, offset, i, _offset;
	char *copy_mem;
	char *tmp_mem;
	unsigned long mem_index = 0;
	unsigned long all_size = 0;
	unsigned long st_all_size = 0;
	unsigned long data = _work->index;
	int temp_refcnt = 0;
	char tmp_str[3*CHUNKSTEP];

 	cpu = get_cpu();
    ct_write_lock_bh(data, hash_lock_array);
	/*
	 * sum the all bytes.
	 */
	list_for_each_entry_safe(cp, next, &hash_tab[data], c_list) {
		temp_refcnt = atomic_read(&cp->data_refcnt);
		while (atomic_read(&cp->data_refcnt) >= ITEM_DISK_LIMIT) {
			//all_size += ((num+1)*3)*CHUNKSTEP;
			all_size += (cp->len + CHUNKSTEP)*3;
			st_all_size += cp->len;

			atomic_dec(&cp->data_refcnt);
		}
		atomic_set(&cp->data_refcnt, temp_refcnt);
    }
		if (all_size > 0) {
			/*
	 		* alloc memory for copy data.
	 		*/	
			copy_mem = kzalloc(all_size, GFP_ATOMIC);
    		if (!copy_mem) {
        		DEBUG_LOG(KERN_ERR"****** %s : vmalloc  copy_mem error\n", __FUNCTION__);
        		BUG(); //TODO need update it.
    		}

    		list_for_each_entry_safe(cp, next, &hash_tab[data], c_list) {
				while (atomic_read(&cp->data_refcnt) >= ITEM_DISK_LIMIT) {
					//tmp_mem = kzalloc(num*3*CHUNKSTEP, GFP_ATOMIC);
					/*tmp_mem = kzalloc(cp->len*3, GFP_ATOMIC);
    				if (!tmp_mem) {
        				DEBUG_LOG(KERN_ERR"****** %s : vmalloc  copy_mem error\n", __FUNCTION__);
        				BUG(); //TODO need update it.
					}

					offset = 0;
					for (i = 0; i < cp->len; i++) {
						sprintf(tmp_mem + offset, "%02x:", (cp->data)[i]&0xff);
						offset += 3;
					}
					
					//printk(KERN_INFO "data is:%s", tmp_mem);

					memcpy(copy_mem + mem_index, tmp_mem, strlen(tmp_mem));
					mem_index += strlen(tmp_mem);
			
					kfree(tmp_mem);
					*/
	
					for (i = 0; i < cp->len; i++) {
						//memcpy(copy_mem + mem_index, tmp_mem, strlen(tmp_mem));
						sprintf(copy_mem + mem_index, "%02x:", (cp->data)[i]&0xff);
						mem_index += 3;
					}

					{
						memset(tmp_str, '\0', CHUNKSTEP);
						
						//memcpy(tmp_str, "\n", strlen("\n"));
						strcat(tmp_str, " ");
						offset = strlen(tmp_str);
						_offset = 0;
						for (i = 0; i < SHA1SIZE; i++) {
							sprintf(tmp_str + offset + _offset, "%02x:", (cp->sha1)[i]&0xff);
							_offset += 3;
						}
						offset = strlen(tmp_str);
						sprintf(tmp_str + offset, " %d\n", cp->len);
					
						//printk(KERN_INFO "len is:%d, info is:%s", strlen(tmp_str), tmp_str);
	
						memcpy(copy_mem + mem_index, tmp_str, strlen(tmp_str));
						//mem_index += CHUNKSTEP*3;
						mem_index += strlen(tmp_str);
					}

					atomic_dec(&cp->data_refcnt);
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
	//ret = kernel_write(per_cpu(reserve_file, cpu), copy_mem, all_size, per_cpu(loff_file, cpu));
	ret = kernel_write(per_cpu(reserve_file, cpu), copy_mem, mem_index, per_cpu(loff_file, cpu));
	if (ret < 0) {
    	printk(KERN_ERR "Error writing file:%d\n", cpu);
		BUG(); //TODO need updat it.
	}
	
	percpu_counter_add(&mmw, st_all_size);

    //TODO: file size maybe so big?
	per_cpu(loff_file, cpu) += mem_index;
	put_cpu();		

	/*
	 * free memory.
	 */
	kfree(copy_mem);
}

void bucket_clear_item(unsigned long data)
{
    struct hashinfo_item *cp, *next;
    int flag = 0;

    ct_write_lock_bh(data, hash_lock_array);
    list_for_each_entry_safe(cp, next, &hash_tab[data], c_list) {
   		if (atomic_dec_and_test(&cp->refcnt)) {
			list_del(&cp->c_list);
			atomic_dec(&hash_count);
			free_data_memory(cp);
            kmem_cache_free(hash_cachep, cp);
			DEBUG_LOG(KERN_INFO "delete it.");
			continue;
		}

		atomic_dec(&cp->refcnt);
		if (flag == 0 && atomic_read(&cp->data_refcnt) >= ITEM_DISK_LIMIT) 
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
