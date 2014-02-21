#include <linux/module.h>		// for printk()
#include <asm/pgtable.h>                // for ZERO_PAGE

static int __init init_hello( void )
{
	printk( "\n   Kello, the first! \n\n" );
	unsigned long i = 0x100;
	printk(" the ZERO address is %p", ZERO_PAGE(&i));
	return	0;
}

static void __exit exit_hello( void )
{
	printk( "\n   Goodbye now... \n\n" );
}

MODULE_LICENSE("GPL");
module_init(init_hello);
module_exit(exit_hello);
