#include <linux/module.h>               // for printk()

static int  __init init_page_dir( void )
{
	printk("Hello world\n");
	return 0;
}

static void __exit exit_page_dir( void )
{
	printk("\n   Goodbye now... \n\n" );
}

MODULE_LICENSE("GPL");
module_init(init_page_dir);
module_exit(exit_page_dir);
