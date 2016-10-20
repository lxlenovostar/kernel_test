#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/skbuff.h>
#include <linux/version.h>
#include <net/tcp.h>
#include <linux/time.h>
#include <net/arp.h>
#include <linux/inetdevice.h>
#include "csum.h"

#define MD5LEN 16
#define SOURCE 6880
#define DEST   6880

__be32 gate_addr = 0;
//__be32 daddr = 0;
//__be32 saddr = 0;
#define GATE_IP "139.209.90.1"
#define SOU_IP "139.209.90.213"
#define DST_IP "119.184.176.146"

/*
#define SOU_IP "192.168.109.181"
#define DST_IP "192.168.109.147"
#define DST_MAC {0x00, 0x0C, 0x29, 0x72, 0x7B, 0xF3}
*/
//#define DST_MAC {0x00, 0x16, 0x31, 0xF0, 0x9B, 0x82}
//static u8 dst_mac[ETH_ALEN] = DST_MAC;

#define SOU_DEVICE "eth0"

#define MAC_IP_TAB_SIZE 2

struct mac_ip {
	__be32 ip;
	char mac[ETH_ALEN];
	unsigned long expires;
};

struct mac_ip mac_ip_table[MAC_IP_TAB_SIZE] = {{0,{0},0}};
unsigned long xmit_itv = 120 * HZ;

/*
struct net_device *ws_sp_get_dev(__be32 ip)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 32)
	struct list_head *net_iter;
	struct list_head *net_device_iter;
	struct net *net;
#endif

	struct net_device *netdev;
	struct in_ifaddr  *ifa;
	__be32 netmask;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 32)
	list_for_each(net_iter, &net_namespace_list) {
		net = list_entry(net_iter, struct net, list);
		list_for_each(net_device_iter, &net->dev_base_head) {
			netdev = list_entry(net_device_iter, 
					struct net_device, dev_list);
			if (unlikely(!netdev->ip_ptr))
				continue;
			for(ifa = ((struct in_device *)netdev->ip_ptr)->ifa_list;
				ifa; ifa = ifa->ifa_next) {
				netmask = ifa->ifa_mask;
				if ((ip & netmask) == (ifa->ifa_address & netmask)) {
					if (netdev->flags & IFF_LOOPBACK)
						continue;
					goto out;
				}

			}
		}
	}

#else
	for(netdev = dev_base; netdev; netdev = netdev->next) {
		if (netdev->ip_ptr) {
			for(ifa = ((struct in_device *)netdev->ip_ptr)->ifa_list;
				ifa; ifa = ifa->ifa_next)  {
				netmask = ifa->ifa_mask;
				if ((ip & netmask) == (ifa->ifa_address & netmask)) {
					if (netdev->flags & IFF_LOOPBACK)
						continue;
					goto out;
				}
			}
		}
	}
#endif
	netdev = NULL;
out:
	return netdev;
}
*/

static inline struct mac_ip *lookup_table(__be32 ip)
{
	int idx;
	for(idx = 0; idx < MAC_IP_TAB_SIZE; ++idx) 
	{
		if(ip == mac_ip_table[idx].ip && 
			mac_ip_table[idx].expires > jiffies)
			return &mac_ip_table[idx];
	}

	return NULL;
}

static inline void addup_table(__be32 ip, char *mac)
{
	int idx;
	for(idx = 0; idx < MAC_IP_TAB_SIZE; ++idx)
	{
		if (mac_ip_table[idx].expires < jiffies)
			mac_ip_table[idx].ip = 0;

		if (!mac_ip_table[idx].ip) {
			mac_ip_table[idx].ip = ip;
			mac_ip_table[idx].expires = jiffies + xmit_itv;
			memcpy(mac_ip_table[idx].mac, mac, ETH_ALEN);
			return;
		}
	}
}

int get_mac(char *dest, __be32 dst_ip, int rtos, struct sk_buff *skb) 
{
	struct mac_ip *tmp;
	struct neighbour *nb_entry = NULL;
	struct rtable *rt;
	struct flowi f1 = {
	    .oif = 0,
	    .nl_u = {
	        .ip4_u = {
		    .daddr = dst_ip,
		    .saddr = 0,
		    .tos = rtos,}},
	};
		
	tmp = lookup_table(dst_ip);
	if (likely(tmp)) {
		memcpy(dest, tmp->mac, ETH_ALEN);
		return 1;
	}

	/* 这个东西的用途？？
	if (net_ratelimit())
		pr_warn("look up table failed.\n");
	*/

	if (ip_route_output_key(&init_net, &rt, &f1))
		return -2;
	nb_entry = neigh_lookup(&arp_tbl, &dst_ip, rt->u.dst.dev);

	printk(KERN_ERR"fuck1");
	if(!nb_entry || !(nb_entry->nud_state & NUD_VALID)) {
		printk(KERN_ERR"fuck2");
		neigh_event_send(rt->u.dst.neighbour, NULL);
		if(nb_entry)
			neigh_release(nb_entry);
		ip_rt_put(rt);
		return -3;
	} else {
		printk(KERN_ERR"fuck3");
		memcpy(dest, nb_entry->ha, ETH_ALEN);
		neigh_release(nb_entry);
		addup_table(dst_ip, nb_entry->ha);
	}
	if (unlikely(rt)) {
		skb_dst_drop(skb);
		skb_dst_set(skb, &rt->u.dst);
	}
	return 1;
}

/*
static unsigned int inet_addr(char *str) 
{ 
	int a,b,c,d; 
    char arr[4]; 
    sscanf(str, "%d.%d.%d.%d", &a,&b,&c,&d); 
	// 网络字节序(big-endian) 
    arr[0] = a; arr[1] = b; arr[2] = c; arr[3] = d; 

    return *(unsigned int*)arr; 
} 
*/

int build_ethhdr(struct sk_buff *skb) 
{
	struct ethhdr *eth;
	struct net_device *dev;
	int err;

	struct iphdr *iph;
    iph = ip_hdr(skb);
	//dev = ws_sp_get_dev(saddr);
	dev = dev_get_by_name(&init_net, SOU_DEVICE);
	if (!dev) {
		printk(KERN_ERR"get device failed.");
		return -3;
	}

	skb->dev = dev;
	skb->pkt_type  = PACKET_OUTGOING; 
	skb->local_df  = 1;     

	eth = (struct ethhdr *)skb_push(skb, ETH_HLEN);
	skb_reset_mac_header(skb);

    skb->protocol = htons(ETH_P_IP);

	memcpy(eth->h_source, dev->dev_addr, ETH_ALEN);
	//memcpy(eth->h_dest, dst_mac, ETH_ALEN);
	
	gate_addr = in_aton(GATE_IP);
	err = get_mac(eth->h_dest, gate_addr, RT_TOS(20), skb);
	if (err != 1) {
		kfree_skb(skb);
		if (net_ratelimit())
			printk(KERN_ERR"get device mac failed when send packets.err:%d", err);
			return err;
		}

    return 0;	
}

/** 
 * build ip header. 
 */
int build_iphdr(struct sk_buff *skb)
{
	struct iphdr *iph;
	int rtos;

	skb_push(skb, sizeof(struct iphdr));
	skb_reset_network_header(skb);
    iph = ip_hdr(skb);

	rtos = RT_TOS(20);
    *((__be16 *)iph) = htons((4 << 12) | (5 << 8) | (RT_TOS(20) & 0xff));
    iph->tot_len = htons(skb->len);
	iph->frag_off = htons(IP_DF);
    iph->ttl      = 64;
    iph->protocol = IPPROTO_TCP;
    //iph->saddr    = inet_addr(SOU_IP);
    //iph->daddr    = inet_addr(DST_IP);

	//saddr =  in_aton(SOU_IP);
	//daddr = in_aton(DST_IP);
    iph->saddr    = in_aton(SOU_IP);
    iph->daddr    = in_aton(DST_IP);
	return 0;
}

/** 
 * build tcp header. Learn by tcp_send_fin()
 */
int build_tcphdr(struct sk_buff *skb)
{
	struct tcphdr *th;

    //skb_shinfo(skb)->gso_segs = 1;
	/*
	有没有必要设置这些变量 ？？
    skb_shinfo(skb)->gso_segs = 0;
    skb_shinfo(skb)->gso_size = 0;
    skb_shinfo(skb)->gso_type = 0;
	skb->csum = 0;
	*/

	skb_push(skb, ALIGN(sizeof(struct tcphdr), 4));
    skb_reset_transport_header(skb);

    /* Build TCP header and checksum it. */
    th = tcp_hdr(skb);
    th->source      = htons(SOURCE);
    th->dest        = htons(DEST);
    th->seq         = htonl(123);
	th->ack_seq     = 0;
	*(((__be16 *)th) + 6)   = htons(((sizeof(struct tcphdr) >> 2) << 12) | TCPCB_FLAG_FIN);
	th->window = htons(560);
	th->check = 0;
	th->urg_ptr = 0;

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

	/* At least 32-bit aligned.  */
    size = ALIGN(sizeof(struct ethhdr), 4) + ALIGN(sizeof(struct iphdr), 4) + ALIGN(sizeof(struct tcphdr), 4) + ALIGN(MD5LEN, 4);

	skb = alloc_skb(size, GFP_ATOMIC);
	if (skb == NULL)
		return -1;

	/* Reserve space for headers and prepare control bits. */
    skb_reserve(skb, size);

	/* build tcp payload. */
	err = copy_md5sum(skb);
	if (err != 0) {
		kfree_skb(skb);
		return err;
	}

	/* build tcp header. */
	err = build_tcphdr(skb);
	if (err != 0) {
		kfree_skb(skb);
		return err;
	}

	/* build ip header. */
	err = build_iphdr(skb);
	if (err != 0) {
		kfree_skb(skb);
		return err;
	}

	/* Calculate the checksum. */
    iph = ip_hdr(skb);
	skb->ip_summed = CHECKSUM_NONE;   
	skbcsum(skb, iph);

	/* build eth header. */
    err = build_ethhdr(skb); 
	if (err != 0) {
		return err;
	}

	err = dev_queue_xmit(skb);

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
