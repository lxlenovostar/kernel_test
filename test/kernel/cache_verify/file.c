#include <linux/syscalls.h>
#include <linux/file.h>

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

//learn form: http://lxr.oss.org.cn/source/drivers/staging/vt6655/device_main.c?v=3.10#L2759
static void my_wq_read( struct work_struct *work)
{
	my_work_t *my_work = (my_work_t *)work;
	printk( "my_work.x %d\n", my_work->x );
	kfree( (void *)work );

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
        printk("Config_FileOperation:open file fail?\n");
        //return -1;
  	}
 
    if (kernel_read(file, 0, buffer, 1024) < 0) {
    	printk("read file error?\n");
        result = -1;
        goto error1;
    }
 
	printk( "content is:%s\n", buffer);
 
error1:
         kfree(buffer);
         fput(file);

	return;
}*/
