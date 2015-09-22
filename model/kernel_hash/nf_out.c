#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/version.h>
#include <linux/ip.h>
#include <linux/in.h>
#include <linux/net.h>
#include <linux/tcp.h>
#include <net/inet_hashtables.h>
#include "debug.h"
#include "chunk.h"
#include "sha.h"
#include "hash_table.h"

struct tcp_chunk *hash_head = NULL;
struct percpu_counter save_num;
struct percpu_counter sum_num;
rwlock_t hash_rwlock = RW_LOCK_UNLOCKED; /* Static way which get rwlock*/

void prune_hash_data(unsigned long data)
{
	/*struct free_slab *tmp;
	struct free_slab *next;
	struct list_head *tmp_head_free_slab = (struct listhead *) data;
	if (likely(tmp_head_free_slab->next != NULL)) {
		list_for_each_entry_safe_reverse(tmp, next, tmp_head_free_slab,
						 list) {
			if (likely(atomic_read(&((tmp->free_mem).users)) == 1)) {
				list_del(&tmp->list);
				kmem_cache_free(skbuff_free_cache, tmp);
				//percpu_counter_inc(&free_packets);
			}
		}
	}*/

	/*
	mod_timer(&get_cpu_var(my_timer), jiffies + 10);
	put_cpu_var(my_timer);
	*/
}

void hand_hash(uint8_t *dst, size_t len) 
{
	//struct tcp_chunk *element;
    
	/*
	unsigned hf_bkt, hf_hashv;                                              
	HASH_FCN(dst, (unsigned)strlen(dst), (hash_head)->hh.tbl->num_buckets, hf_hashv, hf_bkt); 
	DEBUG_LOG(KERN_INFO "HASH is:%u and bucket is:%u, num is:%u\n", hf_hashv, hf_hashv&CT_LOCKARRAY_MASK, CT_LOCKARRAY_MASK);
	*/
	struct hashinfo_item *item;	
	DEBUG_LOG(KERN_INFO "LOCK 1");

	item = get_hash_item(dst);
    if (item == NULL) {
        if (add_hash_info(dst) != 0) {
			DEBUG_LOG(KERN_INFO "add hash item error");
            BUG();
        }   	
		DEBUG_LOG(KERN_INFO "LOCK 3");
	}
	else {
		percpu_counter_add(&sum_num, len);
		percpu_counter_add(&save_num, len);
		DEBUG_LOG(KERN_INFO "save len is:%d\n", len);
		DEBUG_LOG(KERN_INFO "LOCK 4");
	}
	
	/*
	ct_read_lock_bh(hf_hashv, hash_lock_array);
	HASH_FIND_STR(hash_head, dst, element);  
	ct_read_unlock_bh(hf_hashv, hash_lock_array);
	DEBUG_LOG(KERN_INFO "LOCK 2");
	*/

	/*
	read_lock_bh(&hash_rwlock);
	HASH_FIND_STR(hash_head, dst, element);  
	read_unlock_bh(&hash_rwlock);
	DEBUG_LOG(KERN_INFO "LOCK 2");

    if (element == NULL) {
		element = (struct tcp_chunk*)kmalloc(sizeof(struct tcp_chunk), GFP_ATOMIC);
    	element->sha = dst;
    	element->id = len;
		
		ct_write_lock_bh(hf_hashv, hash_lock_array);
    	HASH_ADD_KEYPTR(hh, hash_head, element->sha, SHALEN, element);
		ct_write_unlock_bh(hf_hashv, hash_lock_array);
		DEBUG_LOG(KERN_INFO "LOCK 3");

		write_lock_bh(&hash_rwlock);
    	HASH_ADD_KEYPTR(hh, hash_head, element->sha, SHALEN, element);
		write_unlock_bh(&hash_rwlock);
		DEBUG_LOG(KERN_INFO "LOCK 3");

		percpu_counter_add(&sum_num, len);
    } else {
		percpu_counter_add(&sum_num, len);
		percpu_counter_add(&save_num, len);
		DEBUG_LOG(KERN_INFO "save len is:%d\n", len);
		DEBUG_LOG(KERN_INFO "LOCK 4");
	}
	*/
}

void build_hash(char *src, int start, int end, int length) 
{
	/*
     * Fixup: use slab maybe effectiver than kmalloc.
     */
	int genhash, i;
	uint8_t *dst = kmalloc(sizeof(uint8_t)*SHALEN, GFP_ATOMIC);
	memset(dst, '\0', SHALEN);

	//static uint8_t dst[SHALEN];

	DEBUG_LOG(KERN_INFO "LOCK 7");

	genhash = tcp_v4_sha1_hash_data(dst, src + start, (end - start + 1));
	if (genhash) {
		printk(KERN_ERR "%s\n", __func__);
		BUG();
	}
	/*
	DEBUG_LOG(KERN_INFO "DATA:");
	for (i = start; i <= end; i++) {
		printk("%02x:", src[i]&0xff);
	}*/
	DEBUG_LOG(KERN_INFO "SHA-1:");
	for (i = 0; i < 20; i++) {
		printk("%02x:", dst[i]&0xff);
	}

	hand_hash(dst, length);
	kfree(dst);
}

void get_partition(char *data, int length)
{
	struct kfifo *fifo = NULL;
	spinlock_t lock = SPIN_LOCK_UNLOCKED;
	int fifo_i, fifo_part, value, fifo_len; 
	int start_pos = 0;
	int end_pos = 0;

	DEBUG_LOG(KERN_INFO "LOCK 6");

	if (length > chunk_num) {	
		/*
	     * Fixup: alloc kfifo everytime.
	     */	
		fifo = kfifo_alloc(KFIFOLEN, GFP_ATOMIC, &lock);
		if (unlikely(fifo == NULL)) {
			printk(KERN_ERR "alloc kfifo failed.");
			BUG();
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
	
	DEBUG_LOG(KERN_INFO "LOCK 6.8");

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
	char *data = NULL;
	size_t data_len = 0;
	unsigned short dport;
	struct iphdr *iph = (struct iphdr *)skb->data;
	struct tcphdr *tcph = (struct tcphdr *)(skb->data + (iph->ihl << 2));

	//get_cpu();
	
	skb_linearize(skb);
	
	if (iph->protocol != IPPROTO_TCP)
		return NF_ACCEPT;

	dport = tcph->dest;

	if (likely(ntohs(dport) == 8888)) {	
		/*
		read_lock_bh(&hash_rwlock);
		if (HASH_OVERHEAD(hh, hash_head) >= MEMLIMIT) {
			//DEBUG_LOG(KERN_INFO "Memory is out");
			printk(KERN_ERR "Memory is out");
			read_unlock_bh(&hash_rwlock);
			return NF_ACCEPT;
		} else {
			//DEBUG_LOG(KERN_INFO "Memory is %lu", HASH_OVERHEAD(hh, hash_head));
			printk(KERN_INFO "Memory is:%lu, count is:%u", HASH_OVERHEAD(hh, hash_head), HASH_COUNT(hash_head));
		}
		read_unlock_bh(&hash_rwlock);
		*/

		data = (char *)((unsigned char *)tcph + (tcph->doff << 2));
		data_len = ntohs(iph->tot_len) - (iph->ihl << 2) - (tcph->doff << 2);
		DEBUG_LOG(KERN_INFO "chunk is %d, data_len is %lu, iph_tot is%d, iph is%d, tcph is%d", chunk_num, data_len, ntohs(iph->tot_len), (iph->ihl << 2), (tcph->doff<<2));
		//for (i = 0; i < data_len; ++i)
			//DEBUG_LOG(KERN_INFO "data is:%02x", data[i]&0xff);
	
		DEBUG_LOG(KERN_INFO "LOCK 5");

		get_partition(data, data_len);
		DEBUG_LOG(KERN_INFO "LOCK 5.5");
	}

	//put_cpu();
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
