#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

#define MD5_LEN 16

/*
	build tcp header.
 */
int build_tcp(struct sk_buff *skb)
{
	return 0;	
}

static int minit(void)
{
	int err = 0;
	struct sk_buff *skb;

	skb = alloc_skb(sizeof(struct ethhdr) + sizeof(struct iphdr) + sizeof(struct tcphdr) + MD5_LEN, GFP_ATOMIC);
	if (skb == NULL)
		return 1;

	err = build_tcp(skb);

	return err;    
}

static void mexit(void)
{
	
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
