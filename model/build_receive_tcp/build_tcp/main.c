#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/skbuff.h>
#include <net/tcp.h>
#include "csum.h"

#define MD5LEN 16
#define SOURCE 6880
#define DEST   6881

static unsigned int inet_addr(char *str) 
{ 
	int a,b,c,d; 
    char arr[4]; 
    sscanf(str,"%d.%d.%d.%d",&a,&b,&c,&d); 
	/* 网络字节序(big-endian)*/
    arr[0] = a; arr[1] = b; arr[2] = c; arr[3] = d; 

    return *(unsigned int*)arr; 
} 

/** 
 * build ip header. 
 */
int build_iphdr(struct sk_buff *skb)
{
	struct iphdr *iph;
	skb_push(skb, ALIGN(sizeof(struct iphdr), 4));

	skb_reset_network_header(skb);
    iph = ip_hdr(skb);

    *((__be16 *)iph) = htons((4 << 12) | (5 << 8) | (0 & 0xff));
    iph->tot_len = htons(skb->len);

    iph->ttl      = 64;
    iph->protocol = IPPROTO_TCP;
    iph->saddr    = inet_addr("127.0.0.1");
    iph->daddr    = inet_addr("127.0.0.1");

	return 0;
}

/** 
 * build tcp header. Learn by tcp_send_fin()
 */
int build_tcphdr(struct sk_buff *skb)
{
	struct tcphdr *th;

	skb->csum = 0;
    TCP_SKB_CB(skb)->flags = (TCPCB_FLAG_ACK | TCPCB_FLAG_FIN);
    TCP_SKB_CB(skb)->sacked = 0;
    skb_shinfo(skb)->gso_segs = 1;
    skb_shinfo(skb)->gso_size = 0;
    skb_shinfo(skb)->gso_type = 0;

	skb_push(skb, ALIGN(sizeof(struct tcphdr), 4));
    skb_reset_transport_header(skb);

    /* Build TCP header and checksum it. */
    th = tcp_hdr(skb);
    th->source      = htons(SOURCE);
    th->dest        = htons(DEST);
    th->seq         = htonl(123);
	
	return 0;	
}

int copy_md5sum(struct sk_buff *skb)
{
	uint8_t md5_result[MD5LEN] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};

	if (skb_tailroom(skb) < MD5LEN)
		return 2;
	else {
		memcpy(skb_push(skb, MD5LEN), md5_result, MD5LEN);			
	}

	return 0;
}

static int minit(void)
{
	int err = 0;
	struct sk_buff *skb;
	struct iphdr *iph;
	int size;
   
	/* The TCP header must be at least 32-bit aligned.  */
    size = ALIGN(sizeof(struct ethhdr), 4) + ALIGN(sizeof(struct iphdr), 4) + ALIGN(sizeof(struct tcphdr), 4) + ALIGN(MD5LEN, 4);

	skb = alloc_skb(size, GFP_ATOMIC);
	if (skb == NULL)
		return 1;

	/* Reserve space for headers and prepare control bits. */
    skb_reserve(skb, size);

	/* build tcp payload. */
	err = copy_md5sum(skb);

	/* build tcp header. */
	err = build_tcphdr(skb);

	/* build ip header. */
	err = build_iphdr(skb);

	/* Calculate the checksum. */
    iph = ip_hdr(skb);
	skbcsum(skb, iph);
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
