#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/version.h>
#include <linux/ip.h>
#include <linux/in.h>
#include <linux/net.h>
#include <linux/tcp.h>
#include <linux/kprobes.h>
#include <net/inet_hashtables.h>
#include "debug.h"
#include "chunk.h"
#include "sha.h"
#include "hash_table.h"
#include "slab_cache.h"

struct tcp_chunk *hash_head = NULL;
rwlock_t hash_rwlock = RW_LOCK_UNLOCKED; /* Static way which get rwlock*/
struct percpu_counter save_num;
struct percpu_counter sum_num;

void hand_hash(char *src, size_t len, uint8_t *dst) 
{
	struct hashinfo_item *item;	

	item = get_hash_item(dst, src, len);
    if (item == NULL) {
        if (add_hash_info(dst, src, len) != 0) {
			printk(KERN_ERR "%s:add hash item error", __FUNCTION__);
        }   	
		percpu_counter_add(&sum_num, len);
	}
	else {
		if (len > (SHALEN + 2)) {
			percpu_counter_add(&save_num, (len - SHALEN - 2));
		}
		percpu_counter_add(&sum_num, len);
		DEBUG_LOG(KERN_INFO "save len is:%d\n", len);
	}
}

void build_hash(char *src, int start, int end, int length) 
{
	/*
     * Fixup: use slab maybe effectiver than kmalloc.
     */
	int genhash, i;
	//uint8_t *dst = kmalloc(sizeof(uint8_t)*SHALEN, GFP_ATOMIC);
	//memset(dst, '\0', SHALEN);
	uint8_t *dst;

	/*
     * the length of data <= 22, we can pass it.
     */
	if (length <= (SHALEN + 2))
		return;	

	dst = kmem_cache_zalloc(hash_item_data, GFP_ATOMIC);  
   	if (dst == NULL) {
   		DEBUG_LOG(KERN_ERR "%s\n", __FUNCTION__ );
       	BUG();
   	}   

	//static uint8_t dst[SHALEN];

	genhash = tcp_v4_sha1_hash_data(dst, src + start, length);
	if (genhash) {
		printk(KERN_ERR "%s:calculate SHA-1 failed.", __func__);
		return;
	}
	
	DEBUG_LOG(KERN_INFO "DATA:");
	for (i = start; i <= end; i++) {
		DEBUG_LOG("%02x:", src[i]&0xff);
	}
	DEBUG_LOG(KERN_INFO "SHA-1:");
	for (i = 0; i < 20; i++) {
		DEBUG_LOG("%02x:", dst[i]&0xff);
	}

	hand_hash(src + start, length, dst);
	kmem_cache_free(hash_item_data, dst);
}

void get_partition(char *data, int length)
{
	struct kfifo *fifo = NULL;
	spinlock_t lock = SPIN_LOCK_UNLOCKED;
	int fifo_i, fifo_part, value, fifo_len; 
	int start_pos = 0;
	int end_pos = 0;

	if (length > chunk_num) {	
		/*
	     * Fixup: alloc kfifo everytime.
	     */	
		fifo = kfifo_alloc(KFIFOLEN, GFP_ATOMIC, &lock);
		if (IS_ERR(fifo)) {
			printk(KERN_ERR "Out of memory allocating kfifo failed.");
			return;
		}
	
		calculate_partition(data, length, fifo);

		fifo_part = kfifo_len(fifo);
		fifo_len = fifo_part/sizeof(int); 

		DEBUG_LOG(KERN_INFO "length is:%d, fifo_len is:%lu", length, fifo_part/sizeof(int));
		for (fifo_i = 0; fifo_i < fifo_part/sizeof(value); ++fifo_i) {
			kfifo_get(fifo, (unsigned char *)&value, sizeof(value));

			//DEBUG_LOG(KERN_INFO "fifo_i is:%d", fifo_i);
			if (fifo_i == 0) {
				start_pos = 0;
				end_pos = value;
				DEBUG_LOG(KERN_INFO "start_pos is:%d,end_pos is:%d", start_pos, end_pos);
				build_hash(data, start_pos, end_pos, (end_pos - start_pos + 1));
			}	 
			else {
				start_pos = end_pos + 1;
				end_pos = value;
				DEBUG_LOG(KERN_INFO "start_pos is:%d,end_pos is:%d", start_pos, end_pos);
				build_hash(data, start_pos, end_pos, (end_pos - start_pos + 1));
			}
		}
		
		if (fifo_len > 0)  {
			if (end_pos != length - 1) {
				start_pos = end_pos + 1;
				end_pos = length - 1;
				DEBUG_LOG(KERN_INFO "start_pos is:%d,end_pos is:%d", start_pos, end_pos);
				build_hash(data, start_pos, end_pos, (end_pos - start_pos + 1));
			}
		} else {
			start_pos = 0;
			end_pos = length - 1;
			DEBUG_LOG(KERN_INFO "start_pos is:%d,end_pos is:%d", start_pos, end_pos);
			build_hash(data, start_pos, end_pos, (end_pos - start_pos + 1));
		}			
	
		kfifo_free(fifo);	
	}
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
		int (*okfn)(struct sk_buff *))
{
	char *data;
	size_t data_len = 0;
	unsigned short sport, dport;
	__be32 saddr, daddr;
	char dsthost[16];
	struct iphdr *iph;
	struct tcphdr *tcph;
	
	return NF_ACCEPT;
	
	skb_linearize(skb);
	iph = (struct iphdr *)skb->data;
	tcph = (struct tcphdr *)(skb->data + (iph->ihl << 2));

	if (iph->protocol != IPPROTO_TCP)
		return NF_ACCEPT;

	sport = tcph->source;
	dport = tcph->dest;
	saddr = iph->saddr;
	daddr = iph->daddr;

	//snprintf(dsthost, 16, "%pI4", &iph->saddr);
	//printk(KERN_INFO "ip is:%s", dsthost);

	//if (strcmp(dsthost, "139.209.90.60") == 0 && ntohs(sport) == 80) { 
		data = (char *)((unsigned char *)tcph + (tcph->doff << 2));
		data_len = ntohs(iph->tot_len) - (iph->ihl << 2) - (tcph->doff << 2);
		DEBUG_LOG(KERN_INFO "skb_len is %d, chunk is %d, data_len is %lu, iph_tot is%d, iph is%d, tcph is%d", skb->len, chunk_num, data_len, ntohs(iph->tot_len), (iph->ihl << 2), (tcph->doff<<2));
		//for (i = 0; i < data_len; ++i)
			//DEBUG_LOG(KERN_INFO "data is:%02x", data[i]&0xff);
	
		get_partition(data, data_len);
	//}

	return NF_ACCEPT;
}

//int jpf_ip_rcv(struct sk_buff *skb, struct net_device *dev, struct packet_type *pt, struct net_device *orig_dev)
int jpf_netif_receive_skb(struct sk_buff *skb)
{
	char *data = NULL;
	size_t data_len = 0;
	unsigned short sport, dport;
	__be32 saddr, daddr;
	char dsthost[16];
	char ssthost[16];
	struct iphdr *iph;
	struct tcphdr *tcph;
	unsigned long long reserve_mem;

	/*
	 * TODO: need configure from userspace.
     */		
	reserve_mem = global_page_state(NR_FREE_PAGES) + global_page_state(NR_FILE_PAGES);
	if (reserve_mem < (200ULL*1024*1024/4/1024))
		goto out;		

	/*
     * TODO: this maybe need fix it.
     */
	skb_linearize(skb);

	//iph = (struct iphdr *)(skb->data + 4);
	//tcph = (struct tcphdr *)((skb->data + 4) + (iph->ihl << 2));
	iph = (struct iphdr *)skb->data;
	tcph = (struct tcphdr *)(skb->data + (iph->ihl << 2));

	if (iph->protocol == IPPROTO_TCP) {
		sport = tcph->source;
		dport = tcph->dest;
		saddr = iph->saddr;
		daddr = iph->daddr;

		snprintf(dsthost, 16, "%pI4", &daddr);
		snprintf(ssthost, 16, "%pI4", &saddr);

		/*		
		if (strcmp(dsthost, "139.209.90.60") == 0 && ntohs(sport) == 80)  
			printk(KERN_INFO "ip is:%s", dsthost);
		*/

		//if (strcmp(dsthost, "139.209.90.60") == 0 || strcmp(ssthost, "139.209.90.60") == 0) { 
		if (strcmp(dsthost, "139.209.90.60") == 0 && ntohs(sport) == 80) { 
		//if (ntohs(sport) == 80) { 
		//if (strcmp(dsthost, "139.209.90.60") == 0) { 
		//if (strcmp(dsthost, "192.168.27.77") == 0) { 
			data = (char *)((unsigned char *)tcph + (tcph->doff << 2));
			data_len = ntohs(iph->tot_len) - (iph->ihl << 2) - (tcph->doff << 2);
			DEBUG_LOG(KERN_INFO "skb_len is %d, chunk is %d, data_len is %lu, iph_tot is%d, iph is%d, tcph is%d", skb->len, chunk_num, data_len, ntohs(iph->tot_len), (iph->ihl << 2), (tcph->doff<<2));
			//for (i = 0; i < data_len; ++i)
				//DEBUG_LOG(KERN_INFO "data is:%02x", data[i]&0xff);
			get_partition(data, data_len);
		}
	}
out:
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
    .entry = jpf_netif_receive_skb,
    //.entry = jpf_ip_rcv,
    .kp = { 
        .symbol_name = "netif_receive_skb",
        //.symbol_name = "__vlan_hwaccel_rx",
        //.symbol_name = "vlan_hwaccel_do_receive",
        //.symbol_name = "vlan_gro_receive",
        //.symbol_name = "vlan_tx_tag_present",
        //.symbol_name = "__vlan_hwaccel_rx",
        //.symbol_name = "ip_rcv",
        //.symbol_name = "packet_rcv",
        //.symbol_name = "igb_receive_skb",
        //.symbol_name = "napi_gro_receive",
    },  
};
