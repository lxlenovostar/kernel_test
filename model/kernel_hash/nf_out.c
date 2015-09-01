#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/version.h>
#include <linux/ip.h>
#include <linux/in.h>
#include <linux/net.h>
#include <linux/tcp.h>
#include <net/inet_hashtables.h>
#include "debug.h"

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
	struct iphdr *iph = (struct iphdr *)skb->data;
	struct tcphdr *tcph = (struct tcphdr *)(skb->data + (iph->ihl << 2));
	char *data = NULL;
	size_t data_len = 0;
	unsigned short dport;
	int i;
	
	if (iph->protocol != IPPROTO_TCP)
		return NF_ACCEPT;

	dport = tcph->dest;

	if (likely(ntohs(dport) == 8888)) {	
		data = (char *)((unsigned char *)tcph + (tcph->doff << 2));
		data_len = ntohs(iph->tot_len) - (iph->ihl << 2) - (tcph->doff << 2);
		printk(KERN_INFO "data_len is %lu, iph_tot is%lu, iph is%lu, tcph is%lu", data_len, ntohs(iph->tot_len), (iph->ihl << 2), (tcph->doff<<2));
		for (i = 0; i < data_len; ++i)
			printk(KERN_INFO "data is:%02x\n", data[i]&0xff );
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
