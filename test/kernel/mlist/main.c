#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/percpu.h>
#include <linux/vmalloc.h>

struct reject_skb {
     int cpu;
     struct list_head list;
};

void set_list(struct list_head *head)
{
	struct reject_skb *item1 = kmalloc(sizeof(struct reject_skb), GFP_ATOMIC);
	item1->cpu = 5;
	list_add(&item1->list, (head+5));
	
	struct reject_skb *item2 = kmalloc(sizeof(struct reject_skb), GFP_ATOMIC);
	item2->cpu = 2;
	list_add(&item2->list, (head+2));
	
	struct reject_skb *item7 = kmalloc(sizeof(struct reject_skb), GFP_ATOMIC);
	item7->cpu = 7;
	list_add(&item7->list, (head+7));
}

void print_list(struct list_head *head) 
{
	int cpu;
	struct reject_skb *cp, *next;
	for_each_online_cpu(cpu) {
		list_for_each_entry_safe(cp, next, (head+cpu), list) {	
			list_del(&cp->list);
			printk(KERN_INFO "item->cpu is:%d", cp->cpu);
			kfree(cp);
		}
	}	
}

static int minit(void)
{
	int cpu;

	cpu = num_online_cpus();

	printk(KERN_INFO "cpu is:%d", cpu);
		
	struct list_head *all_list = kmalloc(sizeof(struct list_head)*cpu, GFP_ATOMIC);;
	
	for_each_online_cpu(cpu) {
		INIT_LIST_HEAD((all_list + cpu));
	}

	set_list(all_list);
	print_list(all_list);

	kfree(all_list);
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
