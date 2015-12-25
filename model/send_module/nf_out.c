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
#include "debug.h"
#include "hash_table.h"
#include "sha.h"
#include "chunk.h"
#include "slab_cache.h"

atomic64_t sum_num;
atomic64_t save_num;
atomic64_t skb_num;

int hand_hash(char *src, size_t len, uint8_t *dst, size_t *new_len) 
{
	struct hashinfo_item *item;	

	item = get_hash_item(dst);
    if (item == NULL) {
        if (unlikely(add_hash_info(dst, src, len) != 0)) {
			printk(KERN_ERR "%s:add hash item error", __FUNCTION__);
        }   	
		atomic64_add(len, &sum_num);
		return 0;
	}
	else {
		atomic64_add(len, &sum_num);
		atomic64_add((len - (SHALEN + 2)), &save_num);
		//atomic64_add(len, &save_num);
		*new_len -= len;
		DEBUG_LOG(KERN_INFO "save len is:%d\n", len);
		return 1;
	}
}

void build_hash(char *src, int start, int length, struct list_head *head, size_t *new_len) 
{
	int genhash, i;
	uint8_t *dst;
	struct replace_item *item;

	/*
     * the length of data <= 22, we can pass it.
     */
	if (length <= (SHALEN + 2))
		return;	

	dst = kmem_cache_zalloc(sha_data, GFP_ATOMIC);  
   	if (dst == NULL) {
   		DEBUG_LOG(KERN_ERR "%s\n", __FUNCTION__ );
       	BUG();
   	}   

	genhash = tcp_v4_sha1_hash_data(dst, src + start, length);
	if (genhash) {
		printk(KERN_ERR "%s:calculate SHA-1 failed.", __func__);
		return;
	}
	
	DEBUG_LOG(KERN_INFO "DATA:");
	for (i = start; i <= (start + length - 1); i++) {
		DEBUG_LOG("%02x:", src[i]&0xff);
	}
	DEBUG_LOG(KERN_INFO "SHA-1:");
	for (i = 0; i < 20; i++) {
		DEBUG_LOG("%02x:", dst[i]&0xff);
	}

	if (hand_hash(src + start, length, dst, new_len)) {
		item = kmem_cache_zalloc(replace_data, GFP_ATOMIC);  
   		if (item == NULL) {
   			DEBUG_LOG(KERN_ERR "%s\n", __FUNCTION__ );
       		BUG();
   		}
	
		INIT_LIST_HEAD(&item->c_list);
		item->start = start;
		item->end = (start + length - 1);
		memcpy(item->sha1, dst, SHALEN);
  		list_add_tail(&item->c_list, head); 
	}

	kmem_cache_free(sha_data, dst);
}



void get_partition(char *data, int length, struct list_head *head, size_t *new_len)
{
	struct kfifo *fifo = NULL;
	spinlock_t lock = SPIN_LOCK_UNLOCKED;
	int fifo_i, fifo_part, value, fifo_len; 
	int start_pos = 0;
	int end_pos = 0;

	if (length > chunk_num) {	
		/*
	     * Fixup: alloc kfifo everytime, maybe list is a good way.
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
				build_hash(data, start_pos, (end_pos - start_pos + 1), head, new_len);
			}	 
			else {
				start_pos = end_pos + 1;
				end_pos = value;
				DEBUG_LOG(KERN_INFO "start_pos is:%d,end_pos is:%d", start_pos, end_pos);
				build_hash(data, start_pos, (end_pos - start_pos + 1), head, new_len);
			}
		}
		
		if (fifo_len > 0)  {
			if (end_pos != length - 1) {
				start_pos = end_pos + 1;
				end_pos = length - 1;
				DEBUG_LOG(KERN_INFO "start_pos is:%d,end_pos is:%d", start_pos, end_pos);
				build_hash(data, start_pos, (end_pos - start_pos + 1), head, new_len);
			}
		} else {
			start_pos = 0;
			end_pos = length - 1;
			DEBUG_LOG(KERN_INFO "start_pos is:%d,end_pos is:%d", start_pos, end_pos);
			build_hash(data, start_pos, (end_pos - start_pos + 1), head, new_len);
		}			
	
		kfifo_free(fifo);	
	}
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
		int (*okfn) (struct sk_buff *))
{
	unsigned short sport, dport;
	__be32 saddr, daddr;
	char dsthost[16];
	char ssthost[16];
	struct iphdr *iph;
	struct tcphdr *tcph;
	unsigned long long reserve_mem;
	char *data = NULL;
	size_t data_len = 0;
	struct replace_item *cp;
	struct replace_item *next;
	size_t new_len = 0;
	
	LIST_HEAD(hand_list);
	/*
	 * TODO: need configure from userspace.
     */		
	reserve_mem = global_page_state(NR_FREE_PAGES) + global_page_state(NR_FILE_PAGES);
	if (reserve_mem < (400ULL*1024*1024/4/1024))
		return NF_ACCEPT;

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
			
		//if (strcmp(dsthost, "139.209.90.60") == 0) {  
		if (!strcmp(dsthost, "192.168.109.1")) {  
			data = (char *)((unsigned char *)tcph + (tcph->doff << 2));
			data_len = ntohs(iph->tot_len) - (iph->ihl << 2) - (tcph->doff << 2);
			new_len = data_len;
			get_partition(data, data_len, &hand_list, &new_len);
			atomic64_inc(&skb_num);
   	
			if (!list_empty(&hand_list))
				printk(KERN_INFO "begin is:%zu", new_len);
			list_for_each_entry_safe(cp, next, &hand_list, c_list) {
				printk(KERN_INFO "start is:%d, end is:%d, tax is:%d, len is:%zu, total len is:%d", cp->start, cp->end, cp->end - cp->start + 1, data_len, ntohs(iph->tot_len)+14);
				list_del(&cp->c_list);
				kmem_cache_free(replace_data, cp);
			}
		}
	}

	return NF_ACCEPT;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 32)
static unsigned int nf_out_p(unsigned int hooknum,
		struct sk_buff **skb
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
