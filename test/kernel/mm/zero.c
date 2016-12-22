#include <linux/module.h>		// for printk()
#include <asm/pgtable.h>                // for ZERO_PAGE
#include <linux/mm.h>             // for struct page
#include <linux/page.h>             // for struct page

static int __init init_hello( void )
{
	int i;
	struct page * zero_page;
	printk( "\n   Kello, the first! \n\n" );
	i = 0x100;
	zero_page = ZERO_PAGE(&i);
	printk(" the ZERO address is %p", zero_page);
	return	0;
}

static void __exit exit_hello( void )
{
	printk( "\n   Goodbye now... \n\n" );
}

MODULE_LICENSE("GPL");
module_init(init_hello);
module_exit(exit_hello);
