/*
 *     Filename:  debug.c
 *
 *  Description:
 *
 *      Version:  1.0
 *      Created:  07/14/2014 04:03:54 PM
 *     Revision:  none
 *     Compiler:  gcc
 *
 *       Author:  Hong Jinyi (hongjy), hongjy@chinanetcenter.com
 * Organization:  chinanetcenter
 */
#include "debug.h"
#include "ws_sp_hash_table.h"

#ifdef DEBUG

u32 fsaddr = 0;
u32 fdport = 0;
u32 fdaddr = 0;
u32 fsport = 0;
int check4(u32 saddr, u32 sport, u32 daddr, u32 dport)
{
	if (fsaddr && saddr != fsaddr) return 0;
	if (fdaddr && daddr != fdaddr) return 0;
	if (fsport && sport != fsport) return 0;
	if (fdport && dport != fdport) return 0;
	return 1;
}

void print_skb(struct sk_buff *skb)
{
	struct iphdr *iph;
	struct tcphdr *tcph;
	char *buf = skb->data;
	int i;
	int len = skb->len;
	int iphl;
	int tcphl;

	struct sock *sk = skb->sk;
	struct inet_sock *inet = inet_sk(sk);

	iph = (struct iphdr *)skb->data;
	iphl = iph->ihl << 2;
	tcph = (struct tcphdr *)(skb->data + iphl);
	tcphl = tcph->doff << 2;

	if (!check4(iph->saddr, tcph->source, iph->daddr, tcph->dest))
		return;

	printk("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
	if (inet) {
		printk("inet sk\n");
		printk("%d.%d.%d.%d:%d -> %d.%d.%d.%d:%d\n",
				NIPQUAD(inet->saddr), ntohs(inet->sport),
				NIPQUAD(inet->daddr), ntohs(inet->dport));
	}
	printk("%d.%d.%d.%d:%d -> %d.%d.%d.%d:%d\n",
			NIPQUAD(iph->saddr), ntohs(tcph->source),
			NIPQUAD(iph->daddr), ntohs(tcph->dest));

	printk("Packet length: %d\n", len);
	printk("IP header length: %d\n", iphl);
	printk("TCP header length: %d\n", tcphl);
	for (i = 0; i < len && i < PRINT_LEN; ++i) {
		printk("%02x ", (unsigned char)buf[i]);
		if (i % 4 == 3) printk("\n");
	}
	if (len > PRINT_LEN) {
		printk("......\n");
	}
	printk("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n\n");
}

#else
inline void print_skb(struct sk_buff *skb) {}
#endif
