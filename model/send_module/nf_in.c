#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/version.h>
#include <linux/ip.h>
#include <linux/in.h>
#include <linux/net.h>
#include <linux/tcp.h>
#include <linux/kprobes.h>
#include <net/inet_hashtables.h>
#include <linux/skbuff.h>
#include <net/tcp.h>
#include <linux/kprobes.h>
#include "debug.h"
#include "chunk.h"
#include "sha.h"
#include "hash_table.h"
#include "slab_cache.h"
#include "sort.h"

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
	return NF_ACCEPT;
}


int jpf_ip_rcv(struct sk_buff *skb, struct net_device *dev, struct packet_type *pt, struct net_device *orig_dev)
//int jpf_netif_receive_skb(struct sk_buff *skb)
{
	jprobe_return();
	return 0;
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
	.hooknum        = NF_INET_PRE_ROUTING,
#else
	.hook           = nf_in_p,
	.hooknum        = NF_IP_PRE_ROUTING,
#endif
	.priority       = NF_IP_PRI_FIRST,
};

struct jprobe jps_netif_receive_skb = { 
    //.entry = jpf_netif_receive_skb,
    .entry = jpf_ip_rcv,
    .kp = { 
        //.symbol_name = "netif_receive_skb",
        //.symbol_name = "__vlan_hwaccel_rx",
        //.symbol_name = "vlan_hwaccel_do_receive",
        //.symbol_name = "vlan_gro_receive",
        //.symbol_name = "vlan_tx_tag_present",
        //.symbol_name = "__vlan_hwaccel_rx",
        .symbol_name = "ip_rcv",
        //.symbol_name = "packet_rcv",
        //.symbol_name = "igb_receive_skb",
        //.symbol_name = "napi_gro_receive",
    },  
};
