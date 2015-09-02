#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/version.h>
#include <linux/ip.h>
#include <linux/in.h>
#include <linux/net.h>
#include <linux/tcp.h>
#include <net/inet_hashtables.h>
#include "debug.h"
#include "chunk.h"

static unsigned int nf_out(
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 13, 0)
		const struct nf_hook_ops *ops,
#else
		unsigned int hooknum,
#endif
		struct sk_buff *skb,
		const struct net_device *in,
		const struct net_device *out,
		int (*okfn)(struct sk_buff *))
{
	skb_linearize(skb);
	char *data = NULL;
	size_t data_len = 0;
	unsigned short dport;
	int i;
	
	struct iphdr *iph = (struct iphdr *)skb->data;
	struct tcphdr *tcph = (struct tcphdr *)(skb->data + (iph->ihl << 2));
	
	if (iph->protocol != IPPROTO_TCP)
		return NF_ACCEPT;

	dport = tcph->dest;

	if (likely(ntohs(dport) == 8888)) {	
		data = (char *)((unsigned char *)tcph + (tcph->doff << 2));
		data_len = ntohs(iph->tot_len) - (iph->ihl << 2) - (tcph->doff << 2);
		printk(KERN_INFO "chunk is %d, data_len is %lu, iph_tot is%d, iph is%d, tcph is%d", chunk_num, data_len, ntohs(iph->tot_len), (iph->ihl << 2), (tcph->doff<<2));
		//for (i = 0; i < data_len; ++i)
			//printk(KERN_INFO "data is:%02x", data[i]&0xff);

		/*
         * get partition point.
         */
		struct kfifo *fifo = NULL;
		spinlock_t lock = SPIN_LOCK_UNLOCKED;
		int fifo_i; 
		int fifo_part;
		int value;

		/*
		 * Fixup: alloc kfifo everytime.
         */	
		fifo = kfifo_alloc(KFIFOLEN, GFP_KERNEL, &lock);
		if (unlikely(fifo == NULL)) {
			printk(KERN_ERR "alloc kfifo failed.");
			BUG();
		}	

		if (data_len > chunk_num) {	
			calculate_partition(data, data_len, fifo);
	
			fifo_part = kfifo_len(fifo);

			printk("fifo_len is:%d", fifo_part/sizeof(int));
			for (fifo_i = 0; fifo_i < fifo_part/sizeof(value); ++fifo_i) {
				kfifo_get(fifo, (unsigned char *)&value, sizeof(value));
				printk("fifo_len is:%d, partition is:%d->\n", fifo_part/sizeof(int), (int)value);
				//printk(KERN_INFO "what i is:%d, len is:%d->", fifo_i, fifo_part/sizeof(int));
			}			
		}

		kfifo_free(fifo);	
	}

	return NF_ACCEPT;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 32)
static unsigned int nf_out_p(unsigned int hooknum,
		struct sk_buff **skb,
		const struct net_device *in,
		const struct net_device *out,
		int (*okfn) (struct sk_buff *))
{
	return nf_out(hooknum, *skb, in, out, okfn);
}
#endif

struct nf_hook_ops nf_out_ops = {
	.list		= { NULL, NULL},
	.pf		= PF_INET,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 32)
	.hook           = nf_out,
	.hooknum        = NF_INET_POST_ROUTING,
#else
	.hook           = nf_out_p,
	.hooknum        = NF_IP_POST_ROUTING,
#endif
	.priority       = NF_IP_PRI_LAST,
};
