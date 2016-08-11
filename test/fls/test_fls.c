#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/crypto.h>
#include <linux/err.h>
#include <linux/scatterlist.h>
#include <linux/mm.h>

static int minit(void)
{
    printk(KERN_INFO"result: %d", fls64(8));
    printk(KERN_INFO"result: %d", fls64(256));
    printk(KERN_INFO"result: %d", fls64(0));
    printk(KERN_INFO"result: %d", fls64(1));

    return 0;
}

static void mexit(void)
{
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
