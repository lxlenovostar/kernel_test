#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/percpu.h>
#include <linux/vmalloc.h>

// 10G = 10 * 1024 * 1024 * 1024 /32  number of slots.
// 335544320 / 8  = 41943040(字节) 
// 41943040 / 1024 / 1024 = 40M 
// 41943040  / sizeof(int) =  (数组项)
#define CHUNKSIZE 32
#define FILESIZE 10                 // 10G

unsigned long *percpu_ptr;

/*
#define SIZE (10 * 1024 * 1024 * 1024 /32 /8 /1024 / 8)
typedef struct {
     unsigned long x[SIZE];
} my_work_t;
*/

static int minit(void)
{
	int cpu;
	unsigned long *this;
	unsigned long file_size = (10 * 1024 * 1024 * 1024);
	unsigned long chunk_num = file_size / CHUNKSIZE;
	unsigned long bytes_num = chunk_num / sizeof(int);

	printk("Start %s.\n", THIS_MODULE->name);
	
	percpu_ptr = alloc_percpu(unsigned long);

	for_each_online_cpu(cpu) {
		this = *per_cpu_ptr(percpu_ptr, cpu);;
		//alloc memory for every percpu-value.
		this = vmalloc(bytes_num);
		if (!this) {
			printk(KERN_ERR "alloc bitmap failed.");
			return -ENOMEM;
		}	
	}

	return 0;
}

static void mexit(void)
{
	int cpu;
	unsigned long *this;

	
	if (percpu_ptr) {
		for_each_online_cpu(cpu) {
			this = *per_cpu_ptr(percpu_ptr, cpu);;
		
			if (this)
				vfree(this);	
		}
	}
	

	if (percpu_ptr)
		free_percpu(percpu_ptr);

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
