#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/percpu.h>

struct skb_lock_list {
    struct list_head skb_list;
    spinlock_t lock;
};

/*
 * 实验目标：将list和lock封装在一起，然后定义为per-cpu类型
 */
DEFINE_PER_CPU(struct skb_lock_list, send_list);

static int minit(void)
{
    int cpu;

    for_each_online_cpu(cpu) {
        INIT_LIST_HEAD(&(per_cpu(send_list, cpu).skb_list));
        per_cpu(send_list, cpu).lock = SPIN_LOCK_UNLOCKED;
    }
    
    printk(KERN_INFO"begin:");

	return 0;
}

static void mexit(void)
{

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
