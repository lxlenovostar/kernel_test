#include <linux/init.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/workqueue.h>
#include <linux/syscalls.h>
#include <linux/file.h>

#define CHUNKSTEP 32
#define CACHE_SLAB_CHUNK1 "slab_chunk_1"
struct kmem_cache *slab_chunk1;
 
static struct workqueue_struct *my_wq;
typedef struct {
	struct work_struct my_work;
	int    x;
} my_work_t;

my_work_t *work, *work2;

ssize_t kernel_write(struct file *file, const char *buf, size_t count,
                             loff_t pos)
{
	mm_segment_t old_fs;
    ssize_t res;
 
    old_fs = get_fs();
    set_fs(get_ds());
    /* The cast to a user pointer is valid due to the set_fs() */
    res = vfs_write(file, (__force const char __user *)buf, count, &pos);
    set_fs(old_fs);
 
    return res;
}


//learn form: http://lxr.oss.org.cn/source/drivers/staging/vt6655/device_main.c?v=3.10#L2759
static void my_wq_read( struct work_struct *work)
{
	my_work_t *my_work = (my_work_t *)work;
	printk( "my_work.x %d\n", my_work->x );
	kfree( (void *)work );

	char *tmp_read = kmem_cache_zalloc(slab_chunk1, GFP_ATOMIC);
    unsigned char *buffer = kzalloc(1024, GFP_KERNEL);
    struct file *file;
    int result=0;
 
    if (!buffer) {
    	printk("allocate mem for file fail?\n");
        //return -1;
    }

   file = filp_open("/opt/lx1", O_RDONLY, 0);
   if (IS_ERR(file)) {
   		kfree(buffer);
		 kmem_cache_free(slab_chunk1, tmp_read);
        printk("Config_FileOperation:open file fail?\n");
        //return -1;
  	}
	 
    //if (kernel_read(file, 0, buffer, 1024) < 0) {
    if (kernel_read(file, 0, tmp_read, 1024) < 0) {
    	printk("read file error?\n");
        result = -1;
        goto error1;
    }
 
	//printk( "content is:%s\n", buffer);
	printk(KERN_INFO "content is:%s", tmp_read);
 
error1:
         kfree(buffer);
		 kmem_cache_free(slab_chunk1, tmp_read);
         fput(file);

	return;
}

//learn form: http://lxr.oss.org.cn/source/drivers/target/target_core_alua.c?v=3.10#L729
static void my_wq_write( struct work_struct *work)
{
	my_work_t *my_work = (my_work_t *)work;
	printk( "my_work.x %d\n", my_work->x );
	kfree( (void *)work );
	
	struct file *file = filp_open("/opt/lx1", O_RDWR | O_CREAT | O_TRUNC, 0600);
    int ret;
 	char *md_buf = "1hello world1.";
	int md_buf_len = strlen(md_buf);

    if (IS_ERR(file)) {
    	pr_err("filp_open(%s) for failed\n", "/opt/lx1");
        //return -ENODEV;
    }
    
	ret = kernel_write(file, md_buf, md_buf_len, 0);
    if (ret < 0)
    	pr_err("Error writing file: %s\n", "/opt/lx1");
        
	fput(file);
         
	//return ret ? -EIO : 0;	
	return;
}


static int minit(void)
{
	printk("Start %s.\n", THIS_MODULE->name);

	#if ( LINUX_VERSION_CODE < KERNEL_VERSION(2,6,25) )
    slab_chunk1 = kmem_cache_create(CACHE_SLAB_CHUNK1, 
            CHUNKSTEP,
            0, SLAB_HWCACHE_ALIGN, NULL, NULL);
    #else
    slab_chunk1 = kmem_cache_create(CACHE_SLAB_CHUNK1,
            CHUNKSTEP,
            0, SLAB_HWCACHE_ALIGN, NULL);
    #endif

    if (!slab_chunk1) {
        printk(KERN_ERR "****** %s : kmem_cache_create for chunk1  error\n",
                __FUNCTION__);
        return -ENOMEM;
    }   

	my_wq = create_workqueue("my_queue");
 	if (my_wq) {
    	/* Queue some work (item 1) */
    	work = (my_work_t *)kmalloc(sizeof(my_work_t), GFP_KERNEL);
    	if (work) {
			INIT_WORK((struct work_struct *)work, my_wq_read);
      		work->x = 1;
			queue_work(my_wq, (struct work_struct *)work);
    	}

    	/* Queue some additional work (item 2) */
    	work2 = (my_work_t *)kmalloc(sizeof(my_work_t), GFP_KERNEL);
    	if (work2) {
			INIT_WORK((struct work_struct *)work2, my_wq_write);
			work2->x = 2;
			queue_work(my_wq, (struct work_struct *)work2);
		}
	} else {
		return -1;
	}

	return 0;
}

static void mexit(void)
{
	flush_workqueue( my_wq );
	destroy_workqueue( my_wq );

	if (slab_chunk1)
        kmem_cache_destroy(slab_chunk1);

	printk("Exit %s.\n", THIS_MODULE->name);
}

module_init(minit);
module_exit(mexit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("lix");
#ifdef DEBUG
MODULE_VERSION("1.4.1.debug");
#else
MODULE_VERSION("1.4.1");
#endif
