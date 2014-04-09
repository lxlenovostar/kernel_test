#include <linux/module.h>               // for printk()
#include <linux/kernel.h>               // for KERN_DEBUG  

static int  __init init_page_dir( void )
{
	printk(KERN_DEBUG "Hello world\n");
	return 0;
}

static void __exit exit_page_dir( void )
{
	printk(KERN_DEBUG "\n   Goodbye now... \n\n" );
}

MODULE_LICENSE("GPL");
module_init(init_page_dir);
module_exit(exit_page_dir);
