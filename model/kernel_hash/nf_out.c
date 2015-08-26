/*
 *     Filename:  nf_out.c
 *
 *  Description:
 *
 *      Version:  1.0
 *      Created:  07/10/2014 03:09:44 PM
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
#include <net/inet_hashtables.h>

#include "ws_sp_hash_table.h"
#include "debug.h"
#include "csum.h"

static int get_fakeip_from_sk(struct sock *sk)
{
	struct tcp_sock *tp = tcp_sk(sk);
	if (!tp)
		return 0;
	return tp->rcv_tstamp;
}

static inline void set_sip(struct sk_buff *skb, u32 fakeip)
{
	struct iphdr *iph = (struct iphdr *)skb->data;

	if (fakeip) {
		DEBUG_LOG("#### NF_OUT ####  set_sip: %x -> %x\n", iph->saddr, fakeip);
		iph->saddr = fakeip;
		skbcsum(skb);
	}
}

static unsigned int send_syn(struct sk_buff *skb, struct sp_tcp_hashinfo *hash_info)
{
	struct sp_tcp_hashinfo_item *tcp_item = NULL;
	struct iphdr *iph = (struct iphdr *)skb->data;
	u32 fakeip;

	if ((fakeip = get_fakeip_from_sk(skb->sk)) == 0)
		return NF_ACCEPT;

    //phase one
	tcp_item = get_tcp_hash_item_by_syn(hash_info);
	if (!tcp_item) {
		if (add_tcp_hash_info(hash_info) != 0) {
			debug_tcp_info(__func__, hash_info);
			return NF_DROP;
		}
		tcp_item = get_tcp_hash_item_by_syn(hash_info);
	}
	tcp_item->fakeip = fakeip;
	tcp_item->realip = 0;
    put_tcp_hash_info(tcp_item, 0, TIMEOUT_HASH_SYN);

    //phase two
	hash_info->saddr = fakeip;
	tcp_item = get_tcp_hash_item_by_syn(hash_info);
	if (!tcp_item) {
		if (add_tcp_hash_info(hash_info) != 0) {
			debug_tcp_info(__func__, hash_info);
			return NF_DROP;
		}
		tcp_item = get_tcp_hash_item_by_syn(hash_info);
	}
	tcp_item->fakeip = 0;
	tcp_item->realip = iph->saddr;
	put_tcp_hash_info(tcp_item, 0, TIMEOUT_HASH_SYN);

    //phase three
	set_sip(skb, fakeip);
	print_skb(skb);
	return NF_ACCEPT;
}

static unsigned int send_fin(struct sk_buff *skb, struct sp_tcp_hashinfo *hash_info)
{
	struct sp_tcp_hashinfo_item *tcp_item = NULL;
	tcp_item = get_tcp_hash_item_by_syn(hash_info);
	if (tcp_item) {
		set_sip(skb, tcp_item->fakeip);
		print_skb(skb);
		put_tcp_hash_info(tcp_item, 0, TIMEOUT_HASH_FIN);
	}
	return NF_ACCEPT;
}

static unsigned int send_normal(struct sk_buff *skb, struct sp_tcp_hashinfo *hash_info)
{
	struct sp_tcp_hashinfo_item *tcp_item = NULL;
	tcp_item = get_tcp_hash_item_by_syn(hash_info);
	if (tcp_item) {
		set_sip(skb, tcp_item->fakeip);
		print_skb(skb);
		put_tcp_hash_info(tcp_item, 0, TIMEOUT_HASH_NOR);
	}
	return NF_ACCEPT;
}

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
	struct sp_tcp_hashinfo hash_info;
	struct iphdr *iph = (struct iphdr *)skb->data;
	struct tcphdr *tcph = (struct tcphdr *)(skb->data + (iph->ihl << 2));

	if (iph->protocol != IPPROTO_TCP)
		return NF_ACCEPT;

	hash_info.daddr=iph->daddr;
	hash_info.saddr=iph->saddr;
	hash_info.dport=tcph->dest;
	hash_info.sport=tcph->source;
	hash_info.seq=tcph->seq;

	if (tcph->syn) {
		return send_syn(skb, &hash_info);
	} else if (tcph->fin) {
		return send_fin(skb, &hash_info);
	} else {
		return send_normal(skb, &hash_info);
	}
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
