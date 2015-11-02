#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/crypto.h>
#include <linux/err.h>
#include <linux/scatterlist.h>
#include <linux/mm.h>

static struct timer_list print_memory;

void print_memory_usage(unsigned long data)
{
    printk(KERN_INFO "sleep");
	msleep(10);
    mod_timer(&print_memory, jiffies + 1*HZ);
}

static int minit(void)
{
	int rc = 0;

	printk("Start %s.\n", THIS_MODULE->name);
   
    init_timer(&print_memory);
    print_memory.expires = jiffies + 1*HZ;
    print_memory.data = 0;
    print_memory.function = print_memory_usage;
    add_timer(&print_memory);
        
	return rc;
}

static void mexit(void)
{
	del_timer_sync(&print_memory);
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
