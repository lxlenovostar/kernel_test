#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/version.h>

static unsigned int nf_in(
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
	unsigned short sport, dport;
	struct iphdr *iph;
	struct tcphdr *tcph;

	iph = ip_hdr(skb);
	if (iph->protocol == IPPROTO_TCP) {
 		tcph = (struct tcphdr *)(skb->data + (iph->ihl << 2));	
		
		sport = tcph->source;
		dport = tcph->dest;
		if (ntohs(sport) == 6880 && ntohs(dport) == 6880) {  
			printk(KERN_INFO "find a new packet");		
    		return NF_DROP;
		}
	}
    return NF_ACCEPT;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 32)
static unsigned int nf_in_p(unsigned int hooknum,
		struct sk_buff **skb,
		const struct net_device *in,
		const struct net_device *out,
		int (*okfn) (struct sk_buff *))
{
	return nf_in(hooknum, *skb, in, out, okfn);
}
#endif

struct nf_hook_ops nf_in_ops = {
	.list		= { NULL, NULL},
	.pf		= PF_INET,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 32)
	.hook           = nf_in,
	//.hooknum        = NF_INET_PRE_ROUTING,
	.hooknum        = NF_INET_LOCAL_IN,
#else
	.hook           = nf_in_p,
	//.hooknum        = NF_IP_PRE_ROUTING,
	.hooknum        = NF_IP_LOCAL_IN,
#endif
	.priority       = NF_IP_PRI_FIRST,
};

static int minit(void)
{
	int err = 0;

	printk(KERN_INFO "Start %s.", THIS_MODULE->name);

	if (0 > (err = nf_register_hook(&nf_in_ops))) {
		printk(KERN_ERR "Failed to register nf_in %s.\n", THIS_MODULE->name);
		
		return err;
	}    

	return err;    
}

static void mexit(void)
{
	
	nf_unregister_hook(&nf_in_ops);
	
	printk(KERN_INFO "Exit %s.", THIS_MODULE->name);
}

module_init(minit);
module_exit(mexit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("lix");
#ifdef DEBUG
MODULE_VERSION("0.0.1.debug");
#else
MODULE_VERSION("0.0.1");
#endif
