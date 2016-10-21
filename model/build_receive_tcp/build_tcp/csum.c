#include <linux/version.h>
#include <linux/net.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <net/tcp.h>

void skbcsum(struct sk_buff *skb, struct iphdr *iph)
{
	struct tcphdr *tcph;
	int iphl;
	int tcphl;
	int tcplen;

	iphl = iph->ihl << 2;
	tcph = (struct tcphdr *)((char *)iph + iphl);
	tcphl = tcph->doff << 2;

	/*
     From ip_send_check.
     */
	iph->check	= 0;
	iph->check	= ip_fast_csum((unsigned char *)iph, iph->ihl);

	tcph->check	= 0;
	tcplen		= ntohs(iph->tot_len) - (iph->ihl << 2);
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
