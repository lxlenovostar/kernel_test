#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/workqueue.h>
#include <linux/syscalls.h>
#include <linux/file.h>
#include <linux/vmalloc.h>

struct reject_skb {
	struct list_head list;
	struct sk_buff *skb;
	struct list_head head_rep;
};

struct replace_item 
{
	struct list_head c_list;
	
	int start;
	int end;
	uint8_t sha1[20]; 
};

static int minit(void)
{
	struct replace_item *cp;
	printk(KERN_INFO "Start %s", THIS_MODULE->name);

	struct reject_skb *head = kmalloc(sizeof(struct reject_skb), GFP_ATOMIC);
	INIT_LIST_HEAD(&head->list);
	INIT_LIST_HEAD(&head->head_rep);

	struct replace_item *one = kmalloc(sizeof(struct replace_item), GFP_ATOMIC);
	INIT_LIST_HEAD(&one->c_list);
	one->start = 0;
	one->end = 5;	
	list_add_tail(&one->c_list, &head->head_rep);
	
	struct replace_item *two = kmalloc(sizeof(struct replace_item), GFP_ATOMIC);
	INIT_LIST_HEAD(&two->c_list);
	two->start = 11;
	two->end = 16;	
	list_add_tail(&two->c_list, &head->head_rep);
	
    list_for_each_entry(cp, &head->head_rep, c_list) {
		printk(KERN_INFO "start is:%d, end is:%d", cp->start, cp->end);
	}

	kfree(head);
	kfree(one);
	kfree(two);

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
