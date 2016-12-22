#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/workqueue.h>
#include <linux/syscalls.h>
#include <linux/file.h>
#include <linux/vmalloc.h>
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

/*
//learn form: http://lxr.oss.org.cn/source/drivers/target/target_core_alua.c?v=3.10#L729
static void my_wq_write( struct work_struct *work)
{
	my_work_t *my_work = (my_work_t *)work;
	printk( "my_work.x %d\n", my_work->x );
	kfree( (void *)work );
	
	struct file *file = filp_open("/opt/lx", O_RDWR | O_CREAT | O_TRUNC, 0600);
    int ret;
 	char *md_buf = "hello world.";
	int md_buf_len = strlen(md_buf);

    if (IS_ERR(file)) {
    	pr_err("filp_open(%s) for failed\n", "/opt/lx");
        //return -ENODEV;
    }
    
	ret = kernel_write(file, md_buf, md_buf_len, 0);
    if (ret < 0)
    	pr_err("Error writing file: %s\n", "/opt/lx");
        
	fput(file);
         
	//return ret ? -EIO : 0;	
	return;
}
*/

#define CHUNKSZ             32
int bitmap_scnprintf_ptr(char *buf, unsigned int buflen,
    const unsigned long *maskp, int nmaskbits)
{
    int i, word, bit, len = 0;
    unsigned long val;
    const char *sep = ""; 
    int chunksz;
    u32 chunkmask;

	printk(KERN_INFO "buflen is:%u;nmaskbits is:%d", buflen, nmaskbits);
    
	chunksz = nmaskbits & (CHUNKSZ - 1); 
    if (chunksz == 0) {
		printk(KERN_INFO "Im here.");
        chunksz = CHUNKSZ;
	}
	
	printk(KERN_INFO "chunksz is:%d", chunksz);

    i = ALIGN(nmaskbits, CHUNKSZ) - CHUNKSZ;
	
	printk(KERN_INFO "ALIGN is:%d, i is:%d", ALIGN(nmaskbits, CHUNKSZ), i);

    for (; i >= 0; i -= CHUNKSZ) {
        chunkmask = ((1ULL << chunksz) - 1); 
        word = i / BITS_PER_LONG;
        bit = i % BITS_PER_LONG;
		
		printk(KERN_INFO "word is:%d, bit is:%d", word, bit);
        
		val = (maskp[word] >> bit) & chunkmask;
		
		printk(KERN_INFO "val is:%lu", val);

        len += scnprintf(buf+len, buflen-len, "%s%0*lx", sep,
            (chunksz+3)/4, val);
        chunksz = CHUNKSZ;
        sep = ",";
    }   
    return len;
}

#define FILESIZE 4ULL 
unsigned long *bitmap;

static int minit(void)
{
	unsigned long long file_size = (FILESIZE * 1024 * 1024 * 1024);
	unsigned long long chunk_num = file_size / 32;
	unsigned long long bitmap_size = chunk_num / 8;
	char dst_p[1000];

	bitmap_size = 4;
	chunk_num = bitmap_size*8;

	bitmap = vmalloc(bitmap_size);	//a bit for a chunk.
	if (!bitmap) {
			printk(KERN_ERR "alloc bitmap failed.");
			return -ENOMEM;
	}
	bitmap_zero(bitmap, chunk_num);
	//set_bit(0, bitmap);
	//set_bit(1, bitmap);
	bitmap_scnprintf_ptr(dst_p, 1000, bitmap, chunk_num);
	
	printk(KERN_INFO "Start %s", THIS_MODULE->name);
	printk(KERN_INFO "data is %s, %d", dst_p, strlen(dst_p));

	return 0;
}

static void mexit(void)
{
	vfree(bitmap);
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
