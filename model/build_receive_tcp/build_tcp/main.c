#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/skbuff.h>
#include <net/tcp.h>
#include <linux/time.h>
#include "csum.h"

#define MD5LEN 16
#define SOURCE 6880
#define DEST   6880

#define SOU_IP "139.209.90.213"
#define DST_IP "119.184.176.146"
//#define DST_MAC {0x00, 0x50, 0x56, 0xC0, 0x00, 0x08}
#define DST_MAC {0x00, 0x16, 0x31, 0xF0, 0x9B, 0x82}
static u8 dst_mac[ETH_ALEN] = DST_MAC;
#define SOU_DEVICE "eth0"

static unsigned int inet_addr(char *str) 
{ 
	int a,b,c,d; 
    char arr[4]; 
    sscanf(str, "%d.%d.%d.%d", &a,&b,&c,&d); 
	/* 网络字节序(big-endian) */
    arr[0] = a; arr[1] = b; arr[2] = c; arr[3] = d; 

    return *(unsigned int*)arr; 
} 

int build_ethhdr(struct sk_buff *skb) 
{
	struct ethhdr *eth;
	struct net_device *dev;

	/* from ip_output
	   from sock_bindtodevice

	//return __sock_create(current->nsproxy->net_ns, family, type, protocol, res, 0);
	sk->sk_net = get_net(net);
	struct net *net = sk->sk_net;
	struct net_device *dev = dev_get_by_name(net, devname);
		
	skb->dev = dev;
    skb->protocol = htons(ETH_P_IP);
	*/

	dev = dev_get_by_name(&init_net, SOU_DEVICE);
	if (!dev)
		return 3;

	skb->dev = dev;

	eth = (struct ethhdr *)skb_push(skb, ETH_HLEN);
	skb_reset_mac_header(skb);

    skb->protocol = htons(ETH_P_IP);

	memcpy(eth->h_source, dev->dev_addr, ETH_ALEN);
	memcpy(eth->h_dest, dst_mac, ETH_ALEN);
	
    return 0;	
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
    iph->saddr    = inet_addr(SOU_IP);
    iph->daddr    = inet_addr(DST_IP);

	return 0;
}

/** 
 * build tcp header. Learn by tcp_send_fin()
 */
int build_tcphdr(struct sk_buff *skb)
{
	struct tcphdr *th;
	struct tcp_skb_cb *tcb;

	tcb = TCP_SKB_CB(skb);
    tcb->flags = (TCPCB_FLAG_ACK | TCPCB_FLAG_FIN);
    tcb->sacked = 0;
    //skb_shinfo(skb)->gso_segs = 1;
	skb->csum = 0;
    skb_shinfo(skb)->gso_segs = 0;
    skb_shinfo(skb)->gso_size = 0;
    skb_shinfo(skb)->gso_type = 0;

	skb_push(skb, ALIGN(sizeof(struct tcphdr), 4));
    skb_reset_transport_header(skb);

    /* Build TCP header and checksum it. */
    th = tcp_hdr(skb);
    th->source      = htons(SOURCE);
    th->dest        = htons(DEST);
    th->seq         = htonl(123);
	
	*(((__be16 *)th) + 6)   = htons(((sizeof(struct tcphdr) >> 2) << 12) | tcb->flags);

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
   
	printk(KERN_INFO "Start %s.", THIS_MODULE->name);

	/* The TCP header must be at least 32-bit aligned.  */
    size = ALIGN(sizeof(struct ethhdr), 4) + ALIGN(sizeof(struct iphdr), 4) + ALIGN(sizeof(struct tcphdr), 4) + ALIGN(MD5LEN, 4);

	skb = alloc_skb(size, GFP_ATOMIC);
	if (skb == NULL)
		return 1;

	/* Reserve space for headers and prepare control bits. */
    skb_reserve(skb, size);

	/* build tcp payload. */
	err = copy_md5sum(skb);
	if (err != 0)
		return err;

	/* build tcp header. */
	err = build_tcphdr(skb);
	if (err != 0)
		return err;

	/* build ip header. */
	err = build_iphdr(skb);
	if (err != 0)
		return err;

	/* Calculate the checksum. */
    iph = ip_hdr(skb);
	skbcsum(skb, iph);

	/* build eth header. */
    err = build_ethhdr(skb); 
	if (err != 0)
		return err;

	err = dev_queue_xmit(skb);
	msleep(10);
	//kfree_skb(skb);

	printk(KERN_INFO "dev_queue_xmit:%d", err);

	return err;    
}

static void mexit(void)
{
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
