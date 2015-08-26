/*
 *     Filename:  nf_in.c
 *
 *  Description:
 *
 *      Version:  1.0
 *      Created:  07/10/2014 03:08:41 PM
 *     Revision:  none
 *     Compiler:  gcc
 *
 *       Author:  Hong Jinyi (hongjy), hongjy@chinanetcenter.com
 * Organization:  chinanetcenter
 */
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/version.h>
#include <linux/ip.h>
#include <linux/in.h>
#include <linux/net.h>
#include <linux/tcp.h>
#include <linux/netdevice.h>
#include <linux/inetdevice.h>
#include <net/inet_hashtables.h>

#include "ws_sp_hash_table.h"
#include "debug.h"
#include "csum.h"

static u32 get_realip_byskb(struct sk_buff *skb)
{
	struct iphdr *iph = (struct iphdr *)skb->data;
	struct tcphdr *tcph = (struct tcphdr *)(skb->data + (iph->ihl << 2));
	struct sp_tcp_hashinfo hash_info;
	struct sp_tcp_hashinfo_item *tcp_item;
	u32 ret = 0;

	hash_info.saddr=iph->daddr;
	hash_info.daddr=iph->saddr;
	hash_info.sport=tcph->dest;
	hash_info.dport=tcph->source;
	hash_info.seq=tcph->seq;

	tcp_item = get_tcp_hash_item_by_syn(&hash_info);
	if (tcp_item == NULL)
		return 0;
	ret = tcp_item->realip;
	DEBUG_LOG("**** NF_IN ****  fakeip %x  ->  realip %x\n", iph->daddr, ret);
	put_tcp_hash_info(tcp_item, 0, TIMEOUT_HASH_NOR);
        return ret;
}

static unsigned int nf_in(
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 13, 0)
		const struct nf_hook_ops *ops,
#else
		unsigned int hooknum,
#endif
		struct sk_buff *skb,
		const struct net_device *in,
		const struct net_device *out,
		int (*okfn) (struct sk_buff *))
{
	struct iphdr *iph = (struct iphdr *)skb->data;
	u32 realip;

	if (iph->protocol != IPPROTO_TCP)
		return NF_ACCEPT;

	if (0 == (realip = get_realip_byskb(skb))) return NF_ACCEPT;

	iph->daddr = realip;
	skbcsum(skb);
	print_skb(skb);

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
	.hooknum        = NF_INET_PRE_ROUTING,
#else
	.hook           = nf_in_p,
	.hooknum        = NF_IP_PRE_ROUTING,
#endif
	.priority       = NF_IP_PRI_FIRST,
};

