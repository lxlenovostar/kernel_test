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

unsigned int
hook_local_in(unsigned int hooknum, struct sk_buff *skb,
	      const struct net_device *in, const struct net_device *out,
	      int (*okfn) (struct sk_buff *))
{
	struct iphdr *iph;
	struct tcphdr *tcph;
	unsigned short sport, dport;
	__be32 saddr, daddr;
	char dsthost[16];
	char *verify = "pack";
	int i;


	/*
     * TODO: this maybe need fix it.
     */
	skb_linearize(skb);
	
	iph = (struct iphdr *)skb->data;
	tcph = (struct tcphdr *)(skb->data + (iph->ihl << 2));

	if (iph->protocol == IPPROTO_TCP) {
		sport = tcph->source;
		dport = tcph->dest;
		saddr = iph->saddr;
		daddr = iph->daddr;
		
		snprintf(dsthost, 16, "%pI4", &daddr);

		/*
		 * 此段代码用于控制 SKB 的重入   
         */
		/*
		memcpy(skb->cb, "pack", 4);
		printk(KERN_INFO "len is:%d", strlen(skb->cb));
		for (i = 0; i < strlen(skb->cb); i++)
			printk(KERN_INFO "c is:%c", skb->cb[i]);
		
		if (!memcmp(skb->cb, "pack", 4))
			printk(KERN_INFO "true");
		*/
		
		//return NF_STOLEN;
	}

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
	 //.hooknum = NF_INET_PRE_ROUTING,
#endif
	 },
};



static int minit(void)
{
	int ret;

	ret = nf_register_hooks(hook_ops, ARRAY_SIZE(hook_ops));
	if (ret) {
		printk("local_in hooks failed\n");
	}
	
	printk("Start %s.\n", THIS_MODULE->name);

	return 0;
}

static void mexit(void)
{
	nf_unregister_hooks(hook_ops, ARRAY_SIZE(hook_ops));
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
