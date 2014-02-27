#include <linux/module.h>               // for printk()
#include <linux/mm.h>             // for struct page
#include <asm/page.h>             // for struct page
#include <asm/pgtable.h>                // for ZERO_PAGE
#include <linux/sched.h>                // for struct mm_struct

static int __init init_page_dir( void )
{
	struct task_struct *tsk;
	
	tsk = current;
	
	printk(KERN_ALERT " the first address of PGD is %p", tsk->mm->pgd);
	return	0;
}

static void __exit exit_page_dir( void )
{
	printk( KERN_ALERT "\n   Goodbye now... \n\n" );
}

MODULE_LICENSE("GPL");
module_init(init_page_dir);
module_exit(exit_page_dir);
