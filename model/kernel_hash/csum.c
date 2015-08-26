/*
 *     Filename:  csum.c
 *
 *  Description:
 *
 *      Version:  1.0
 *      Created:  07/17/2014 03:41:04 PM
 *     Revision:  none
 *     Compiler:  gcc
 *
 *       Author:  Hong Jinyi (hongjy), hongjy@chinanetcenter.com
 * Organization:  chinanetcenter
 */
#include <linux/version.h>
#include <linux/net.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <net/tcp.h>

void skbcsum(struct sk_buff *skb)
{
	struct tcphdr *tcph;
	struct iphdr *iph;
	int iphl;
	int tcphl;
	int tcplen;

	iph = (struct iphdr *)skb->data;
	iphl = iph->ihl << 2;
	tcph = (struct tcphdr *)(skb->data + iphl);
	tcphl = tcph->doff << 2;

	iph->check	= 0;
	iph->check	= ip_fast_csum((unsigned char *)iph, iph->ihl);

	tcph->check	= 0;
	tcplen		= skb->len - (iph->ihl << 2);
	if (skb->ip_summed == CHECKSUM_PARTIAL) {
		tcph->check = ~csum_tcpudp_magic(iph->saddr, iph->daddr,
				tcplen, IPPROTO_TCP, 0);
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 32)
		skb->csum = offsetof(struct tcphdr, check);
#else
		skb->csum_start	= skb_transport_header(skb) - skb->head;
		skb->csum_offset = offsetof(struct tcphdr, check);
#endif
	}
	else {
		skb->csum = 0;
		skb->csum = skb_checksum(skb, iph->ihl << 2, tcplen, 0);
		tcph->check = csum_tcpudp_magic(iph->saddr, iph->daddr,
				tcplen, IPPROTO_TCP, skb->csum);
	}
}
