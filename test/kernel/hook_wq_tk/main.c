#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/err.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 32))
#include <net/net_namespace.h>
#endif
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <net/tcp.h>
#include <linux/interrupt.h>
#include <linux/skbuff.h>
#include <linux/kprobes.h>
#include <linux/slab.h>
#include <linux/mempool.h>
#include "ws_st_symbols.h"

static struct workqueue_struct *my_wq;

typedef struct {
	struct work_struct my_work;
	int    index;
} my_work_t;

DEFINE_PER_CPU(struct work_struct , work); 

struct reject_skb {
	struct sk_buff *skb;
	struct list_head list;
};

static DEFINE_PER_CPU(struct list_head, head_skb_list);

#define CACHE_NAME "skb_cache"
#define POOL_MINMUM 409600 
static struct kmem_cache * hash_cachep/* __read_mostly*/;
mempool_t *pool;
char cachename[8][20];
 
void my_tasklet_function(unsigned long data)
{
	struct sk_buff *skb = (struct sk_buff *)data;

	//printk(KERN_INFO "Im here end0.");
	local_bh_disable();
   	//skb_pull(skb, ip_hdrlen(skb));
	//skb_reset_transport_header(skb);
	//(*tcp_v4_rcv_ptr)(skb);
	(*ip_rcv_finish_ptr)(skb);
	local_bh_enable();
	//printk(KERN_INFO "Im here end1.");
	
	return;
}

static void handle_skb(struct work_struct *work)
{
	int cpu;
    struct reject_skb *cp, *next;
	int threshold = 40000;
	int i = 0;	
	
	struct tasklet_struct *my_tasklet;
	LIST_HEAD(hand_list);

	cpu = get_cpu();
	local_bh_disable();
    list_for_each_entry_safe(cp, next, &per_cpu(head_skb_list, cpu), list) {
		if (i >= threshold)
			break;
		else
			++i;

		list_del(&cp->list);
		list_add_tail(&cp->list, &hand_list);
	}
	local_bh_enable();
	put_cpu();
		
	my_tasklet = kmalloc(sizeof(struct tasklet_struct), GFP_ATOMIC);
    list_for_each_entry_safe(cp, next, &hand_list, list) {
		list_del(&cp->list);
		
		tasklet_init(my_tasklet, my_tasklet_function, (unsigned long)cp->skb);
		/* Schedule the Bottom Half */
		tasklet_hi_schedule(my_tasklet);
		tasklet_kill(my_tasklet);
	
		kmem_cache_free(hash_cachep, cp);	
		//mempool_free(cp, pool);
	}
	kfree(my_tasklet);
}

int jpf_ip_rcv(struct sk_buff *skb, struct net_device *dev, struct packet_type *pt, struct net_device *orig_dev)
{
	struct iphdr *iph;
	struct tcphdr *tcph;
	unsigned short sport, dport;
	__be32 saddr, daddr;
	char dsthost[16];
	char ssthost[16];
	int cpu;

	skb_linearize(skb);
	
	iph = (struct iphdr *)skb->data;
	tcph = (struct tcphdr *)(skb->data + (iph->ihl << 2));

	if (iph->protocol == IPPROTO_TCP) {
		sport = tcph->source;
		dport = tcph->dest;
		saddr = iph->saddr;
		daddr = iph->daddr;
		
		snprintf(dsthost, 16, "%pI4", &daddr);
		snprintf(ssthost, 16, "%pI4", &saddr);
		int data_len = ntohs(iph->tot_len) - (iph->ihl << 2) - (tcph->doff << 2);

		//if (!strcmp(ssthost, "192.168.27.77")) { 
		//if (strcmp(dsthost, "139.209.90.60") == 0 && ntohs(sport) == 80) {  
		//if (strcmp(dsthost, "139.209.90.60") == 0) {  
		//if (strcmp(ssthost, "45.78.40.252") == 0 && data_len > 0) {  
		if (strcmp(dsthost, "139.209.90.60") == 0 || strcmp(ssthost, "139.209.90.60") == 0) {  
				//struct reject_skb *skb_item = mempool_alloc(pool, GFP_ATOMIC);  
				struct reject_skb *skb_item = kmem_cache_zalloc(hash_cachep, GFP_ATOMIC);  
   				if (!skb_item) {
   					printk(KERN_INFO "%s\n", __FUNCTION__);
       				BUG();
   				}
				skb_item->skb = skb_copy(skb, GFP_ATOMIC);
				INIT_LIST_HEAD(&skb_item->list);   

				//SKB 进入等待队列
				cpu = get_cpu();
				list_add_tail(&skb_item->list, &per_cpu(head_skb_list, cpu));
				put_cpu();
				
				//判断是否开启工作队列
				cpu = get_cpu();
				if (!work_pending(&(per_cpu(work, cpu)))) {
					INIT_WORK(&(per_cpu(work, cpu)), handle_skb);
					queue_work(my_wq, &(per_cpu(work, cpu)));
				} 
				put_cpu();
		}
	}
	jprobe_return();
	return 0;
}

void clear_remainder_skb(void)
{
	int cpu;
	int rem = 0;

clear_again:	
	for_each_online_cpu(cpu) {
		if (list_empty(&per_cpu(head_skb_list, cpu))) 
		{
			rem++;
		}
		else {
			if (!work_pending(&(per_cpu(work, cpu)))) {
				INIT_WORK(&(per_cpu(work, cpu)), handle_skb);
				queue_work(my_wq, &(per_cpu(work, cpu)));
			} 
		} 
	}
	
	if (rem == num_online_cpus()) {
		printk(KERN_INFO "ok, we clear all skbs and we out.");
		return;
	}
	else {
		flush_workqueue(my_wq);
		schedule();
        rem = 0;
		goto clear_again;
	}

}

unsigned int
hook_local_in(unsigned int hooknum, struct sk_buff *skb,
	      const struct net_device *in, const struct net_device *out,
	      int (*okfn) (struct sk_buff *))
{
	return NF_ACCEPT;
}

static struct nf_hook_ops hook_ops[] = {
	{
	 .owner = THIS_MODULE,
	 .pf = PF_INET,
	 .priority = NF_IP_PRI_FIRST,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,25))
	 .hook = hook_local_in_p,
	 .hooknum = NF_IP_LOCAL_IN,
#else
	 .hook = hook_local_in,
	 .hooknum      = NF_INET_LOCAL_IN,
	 .hooknum = NF_INET_PRE_ROUTING,
#endif
	 },
};

struct jprobe jps_netif_receive_skb = { 
    //.entry = jpf_netif_receive_skb,
    .entry = jpf_ip_rcv,
    .kp = { 
        .symbol_name = "ip_rcv",
    },  
};

static int minit(void)
{
	int ret, cpu;
	
	for_each_online_cpu(cpu) {
		INIT_LIST_HEAD(&per_cpu(head_skb_list, cpu));
	}

	my_wq = create_workqueue("my_queue");
	if (!my_wq)
		return -1;
	
	#if ( LINUX_VERSION_CODE < KERNEL_VERSION(2,6,25) )
    	hash_cachep = kmem_cache_create(CACHE_NAME, sizeof(struct reject_skb), 0, SLAB_HWCACHE_ALIGN, NULL, NULL);
	#else
    	hash_cachep = kmem_cache_create(CACHE_NAME, sizeof(struct reject_skb), 0, SLAB_HWCACHE_ALIGN, NULL);
	#endif

    	if (!hash_cachep) {
        	printk(KERN_ERR "****** %s : kmem_cache_create  error\n",
                __FUNCTION__);
        return -ENOMEM;
    	}

	pool = mempool_create(POOL_MINMUM, mempool_alloc_slab, mempool_free_slab, hash_cachep);	
	if (!pool) {
        printk(KERN_ERR "****** %s : kmem_cache_create  error\n",
                __FUNCTION__);
        return -ENOMEM;
	}

	ret = register_jprobe(&jps_netif_receive_skb);
    if (ret < 0) {
        printk(KERN_ERR "Failed to register jprobe netif_receive_skb %s.\n", THIS_MODULE->name);
        return -1;
    }
 
	ret = nf_register_hooks(hook_ops, ARRAY_SIZE(hook_ops));
	if (ret) {
		printk("local_in hooks failed\n");
	}

	printk("Start %s.\n", THIS_MODULE->name);
	return 0;
}

static void mexit(void)
{
    unregister_jprobe(&jps_netif_receive_skb);
	nf_unregister_hooks(hook_ops, ARRAY_SIZE(hook_ops));
	
	flush_workqueue(my_wq);
	clear_remainder_skb();
	destroy_workqueue(my_wq);

	mempool_destroy(pool);
	kmem_cache_destroy(hash_cachep);	
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
