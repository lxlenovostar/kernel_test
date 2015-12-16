#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/version.h>
#include <linux/ip.h>
#include <linux/in.h>
#include <linux/net.h>
#include <linux/tcp.h>
#include <linux/kprobes.h>
#include <net/inet_hashtables.h>
#include <linux/skbuff.h>
#include <net/tcp.h>
#include <linux/kprobes.h>
#include "ws_st_symbols.h"
#include "debug.h"
#include "chunk.h"
#include "sha.h"
#include "hash_table.h"
#include "slab_cache.h"
#include "alloc_mem.h"

struct percpu_counter save_num;
struct percpu_counter sum_num;
struct percpu_counter skb_num;
struct percpu_counter rdl;
struct percpu_counter rdf;
DEFINE_PER_CPU(struct work_struct , work); 
struct kmem_cache *sha_data; 
struct kmem_cache *reskb_cachep;
struct kmem_cache *listhead_cachep;
struct kmem_cache *readskb_cachep;
struct kmem_cache *tasklet_cachep;
DEFINE_PER_CPU(struct list_head, skb_list);
DECLARE_PER_CPU(struct file *, reserve_file); 

void tasklet_resend(unsigned long data)
{
	struct sk_buff *skb = (struct sk_buff *)data;

	//printk(KERN_INFO "Im here end0.");
	local_bh_disable();
    //skb_pull(skb, ip_hdrlen(skb));
	//skb_reset_transport_header(skb);
	//(*tcp_v4_rcv_ptr)(skb);
	(*ip_rcv_finish_ptr)(skb);
	//kfree_skb(skb);
	local_bh_enable();
	//printk(KERN_INFO "Im here end1.");
	
	return;
}

/*
读文件的处理流程：

prehandle_skb(skb) {

	for i in range(len(skb)):
	1.	检测替换部分，通过函数 get_data_fromhash(SHA-1)获得数据部分
	2.  在函数 get_data_fromhash 中将share_ref 加1 。使此chunk在状态机中停止流动。
    3.  读取数据到指定位置之后，将share_ref 减1
    4.  将没有在内存中的chunk，建立链表read_list方便后续读取

   	for i in range(len(read_list)):
   	1. 将数据从文件读取
   	2. 状态位置改为4 

	for i in range(len(skb))
    	填充数据
	
	for in range(len(skb))
   		刷新hash表 get_partition()
}
*/

void hand_hash(char *src, size_t len, uint8_t *dst, struct list_head *head) 
{
	struct hashinfo_item *item;	
	struct read_skb *r_skb;
	//unsigned long flags;

	//local_irq_save(flags);
	item = get_hash_item(dst);
	//local_irq_restore(flags);
    if (item == NULL) {
		//local_irq_save(flags);
        if (unlikely(add_hash_info(dst, src, len) != 0)) {
			printk(KERN_ERR "%s:add hash item error", __FUNCTION__);
        }   	
		//local_irq_restore(flags);
		//percpu_counter_add(&sum_num, len);
	}
	else {
		//percpu_counter_add(&save_num, (len - SHALEN - 2));
		//percpu_counter_add(&sum_num, len);
		DEBUG_LOG(KERN_INFO "save len is:%d\n", len);

		if (atomic_read(&item->flag_cache) != 4) {
			/*
			 * if the data is in the memory, we do nothing.
			 */
			atomic_dec(&item->share_ref);
			/*write_lock_bh(&item->share_lock);
			atomic_dec(&item->share_ref);
			write_unlock_bh(&item->share_lock);
			*/
		} else {
			r_skb = kmem_cache_zalloc(readskb_cachep, GFP_ATOMIC);  
   			if (r_skb == NULL) {
   				DEBUG_LOG(KERN_ERR "%s\n", __FUNCTION__ );
       			BUG();
   			}

			if (item->cpuid >= num_online_cpus()) {
   				DEBUG_LOG(KERN_ERR "%s\n", __FUNCTION__ );
       			BUG();
			}	
			   
			r_skb->item = item;
			list_add(&r_skb->list, (head+item->cpuid));
		}
	}
}

void build_hash(char *src, int start, int end, int length, struct list_head *head) 
{
	/*
     * Fixup: use slab maybe effectiver than kmalloc.
     */
	int genhash, i;
	uint8_t *dst;

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

	genhash = tcp_v4_sha1_hash_data(dst, src + start, (end - start + 1));
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

	hand_hash(src + start, length, dst, head);
	kmem_cache_free(sha_data, dst);
}

void get_partition(char *data, int length, struct list_head *head)
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
				build_hash(data, start_pos, end_pos, (end_pos - start_pos + 1), head);
			}	 
			else {
				start_pos = end_pos + 1;
				end_pos = value;
				DEBUG_LOG(KERN_INFO "start_pos is:%d,end_pos is:%d", start_pos, end_pos);
				build_hash(data, start_pos, end_pos, (end_pos - start_pos + 1), head);
			}
		}
		
		if (fifo_len > 0)  {
			if (end_pos != length - 1) {
				start_pos = end_pos + 1;
				end_pos = length - 1;
				DEBUG_LOG(KERN_INFO "start_pos is:%d,end_pos is:%d", start_pos, end_pos);
				build_hash(data, start_pos, end_pos, (end_pos - start_pos + 1), head);
			}
		} else {
			start_pos = 0;
			end_pos = length - 1;
			DEBUG_LOG(KERN_INFO "start_pos is:%d,end_pos is:%d", start_pos, end_pos);
			build_hash(data, start_pos, end_pos, (end_pos - start_pos + 1), head);
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
	return NF_ACCEPT;
}

static void read_data(struct list_head *all_head)
{
    struct read_skb *r_cp, *r_next;
	int cpu;
	struct hashinfo_item *item;
	char buffer[512];
	char *tmp_data = NULL;
	ssize_t ret;

	/* 
	 * read the file.
	 */
    for_each_online_cpu(cpu) {
        list_for_each_entry_safe(r_cp, r_next, (all_head+cpu), list) {   
			/*//避免读文件，探测性能瓶颈
			item = r_cp->item; 
			
			spin_lock(&item->data_lock);
			alloc_data_memory(item, item->len);
			spin_unlock(&item->data_lock);
			
			list_del(&r_cp->list);
            atomic_dec(&item->share_ref);
			kmem_cache_free(readskb_cachep, r_cp); 
           	
			continue;
			*/	
			item = r_cp->item; 
		
			// alloc memory space for data.
			spin_lock(&item->data_lock);
			alloc_data_memory(item, item->len);
			spin_unlock(&item->data_lock);

			//memory space for temporary read file.
			if (unlikely(item->len > 512)) {
				tmp_data = kzalloc(item->len, GFP_ATOMIC);	
			} else {
				memset(buffer, '\0', 512);
			}
			
			//read the file.
			if (unlikely(item->len > 512)) {
				ret = kernel_read(per_cpu(reserve_file, cpu), (item->start)*CHUNKSTEP, tmp_data, item->len);
   			} else {
   				ret = kernel_read(per_cpu(reserve_file, cpu), (item->start)*CHUNKSTEP, buffer, item->len);
			}
		
			if (ret < 0) {
   				printk(KERN_ERR "read file error! err message is:%zd, start is:%lu, len is:%d",ret, (item->start)*CHUNKSTEP, item->len);
       			BUG(); //TODO: need update it.
   			}

			//percpu_counter_add(&rdl, item->len);
			//percpu_counter_inc(&rdf);

			// memcpy temporary space to data.
			spin_lock(&item->data_lock);
			if (unlikely(item->len > 512)) {
				memcpy(item->data, tmp_data, item->len);
			} else {
				memcpy(item->data, buffer, item->len);
			}
			spin_unlock(&item->data_lock);
		
			if (unlikely(item->len > 512)) 
				kfree(tmp_data);

			list_del(&r_cp->list);
			
			/*
			 * TODO: this maybe not lock. just atomic.
			 */
			//write_lock_bh(&item->share_lock);
            atomic_dec(&item->share_ref);
            //write_unlock_bh(&item->share_lock);
           	
			kmem_cache_free(readskb_cachep, r_cp); 
        }   
    }   
}

static void resend_skb(struct list_head *head)
{
    struct reject_skb *cp, *next;
	struct tasklet_struct *tmp_tasklet;
	
	tmp_tasklet = kmem_cache_zalloc(tasklet_cachep, GFP_ATOMIC);  
    list_for_each_entry_safe(cp, next, head, list) {
		list_del(&cp->list);
		
		tasklet_init(tmp_tasklet, tasklet_resend, (unsigned long)cp->skb);
		/* Schedule the Bottom Half */
		tasklet_hi_schedule(tmp_tasklet);
		tasklet_kill(tmp_tasklet);
		kmem_cache_free(reskb_cachep, cp);
	}
	kmem_cache_free(tasklet_cachep, tmp_tasklet);
}

static void handle_skb(struct work_struct *work)
{
	int cpu;
    struct reject_skb *cp, *next;
	int threshold = 10000;
	int i = 0;	
	char *data = NULL;
	size_t data_len = 0;
	struct iphdr *iph;
	struct tcphdr *tcph;
	struct list_head *all_list;  

	LIST_HEAD(hand_list);
	/*
	 * get the skb
	 */
	cpu = get_cpu();
	local_bh_disable();
    list_for_each_entry_safe(cp, next, &per_cpu(skb_list, cpu), list) {
		if (i >= threshold)
			break;
		else
			++i;

		list_del(&cp->list);
		list_add_tail(&cp->list, &hand_list);
	}
	local_bh_enable();
	put_cpu();

	if (list_empty(&hand_list))
		return;
    
	/*
	 * alloc the head list.
	 */
	//struct list_head *all_list  = NULL;  
	all_list  = kmem_cache_zalloc(listhead_cachep, GFP_ATOMIC);  
   	if (all_list == NULL) {
   		DEBUG_LOG(KERN_INFO "%s\n", __FUNCTION__ );
       	BUG();	//TODO
	}

	for_each_online_cpu(cpu) {
    	INIT_LIST_HEAD((all_list + cpu));
	}
 
	/*
	 * 1.make data into hashmap.
	 * 2.read the data from memory
     * 3.store the data which need read from file.
	 */
    list_for_each_entry(cp, &hand_list, list) {
		iph = (struct iphdr *)(cp->skb)->data;
		tcph = (struct tcphdr *)((cp->skb)->data + (iph->ihl << 2));

		data = (char *)((unsigned char *)tcph + (tcph->doff << 2));
		data_len = ntohs(iph->tot_len) - (iph->ihl << 2) - (tcph->doff << 2);
		DEBUG_LOG(KERN_INFO "skb_len is %d, chunk is %d, data_len is %lu, iph_tot is%d, iph is%d, tcph is%d", (cp->skb)->len, chunk_num, data_len, ntohs(iph->tot_len), (iph->ihl << 2), (tcph->doff<<2));
			//for (i = 0; i < data_len; ++i)
				//DEBUG_LOG(KERN_INFO "data is:%02x", data[i]&0xff);
		get_partition(data, data_len, all_list);
	}   

	/* 
	 * read the file.
	 */
	read_data(all_list);	
	kmem_cache_free(listhead_cachep, all_list);
	
	/*
	 * resend skb into kernel.
	 */	
	resend_skb(&hand_list);
}

int jpf_ip_rcv(struct sk_buff *skb, struct net_device *dev, struct packet_type *pt, struct net_device *orig_dev)
//int jpf_netif_receive_skb(struct sk_buff *skb)
{
	unsigned short sport, dport;
	__be32 saddr, daddr;
	char dsthost[16];
	char ssthost[16];
	struct iphdr *iph;
	struct tcphdr *tcph;
	unsigned long long reserve_mem;
	int cpu;
	struct reject_skb *skb_item;  
	char *data = NULL;
	size_t data_len = 0;

	/*
	 * TODO: need configure from userspace.
     */		
	reserve_mem = global_page_state(NR_FREE_PAGES) + global_page_state(NR_FILE_PAGES);
	if (reserve_mem < (400ULL*1024*1024/4/1024))
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
		
		//if (strcmp(dsthost, "139.209.90.60") == 0) {  
		//if (strcmp(dsthost, "139.209.90.60") == 0 && ntohs(sport) == 80) {  
		//if (strcmp(dsthost, "139.209.90.60") == 0 || strcmp(ssthost, "139.209.90.60") == 0) {  
		if (strcmp(dsthost, "139.209.90.213") != 0 && strcmp(ssthost, "139.209.90.213") != 0) {  
		//if (strcmp(ssthost, "192.168.27.77") == 0) {  
			/*
			//case 1: 			
			data = (char *)((unsigned char *)tcph + (tcph->doff << 2));
			data_len = ntohs(iph->tot_len) - (iph->ihl << 2) - (tcph->doff << 2);
			DEBUG_LOG(KERN_INFO "skb_len is %d, chunk is %d, data_len is %lu, iph_tot is%d, iph is%d, tcph is%d", (cp->skb)->len, chunk_num, data_len, ntohs(iph->tot_len), (iph->ihl << 2), (tcph->doff<<2));
			//for (i = 0; i < data_len; ++i)
				//DEBUG_LOG(KERN_INFO "data is:%02x", data[i]&0xff);
			get_partition(data, data_len, NULL);
			percpu_counter_inc(&skb_num);
			*/

			//case2
			skb_item = kmem_cache_zalloc(reskb_cachep, GFP_ATOMIC);  
   			if (!skb_item) {
   				printk(KERN_INFO "%s\n", __FUNCTION__);
       			BUG();
   			}
			skb_item->skb = skb_copy(skb, GFP_ATOMIC);
			INIT_LIST_HEAD(&skb_item->list);   

			//SKB 进入等待队列
			cpu = get_cpu();
			list_add_tail(&skb_item->list, &per_cpu(skb_list, cpu));
			put_cpu();
		
			percpu_counter_inc(&skb_num);

			//判断是否开启工作队列
			cpu = get_cpu();
			if (!work_pending(&(per_cpu(work, cpu)))) {
				INIT_WORK(&(per_cpu(work, cpu)), handle_skb);
				queue_work(skb_wq, &(per_cpu(work, cpu)));
			} 
			put_cpu();
		}
	}
out:
	jprobe_return();
	return 0;
}

void clear_remainder_skb(void)
{
	int cpu;
	int rem = 0;

clear_again:	
	for_each_online_cpu(cpu) {
		if (list_empty(&per_cpu(skb_list, cpu))) 
		{
			rem++;
		}
		else {
			if (!work_pending(&(per_cpu(work, cpu)))) {
				INIT_WORK(&(per_cpu(work, cpu)), handle_skb);
				queue_work(skb_wq, &(per_cpu(work, cpu)));
			} 
		} 
	}
	
	if (rem == num_online_cpus()) {
		printk(KERN_INFO "ok, we clear all skbs and we out.");
		return;
	}
	else {
		flush_workqueue(skb_wq);
		schedule();
        rem = 0;
		goto clear_again;
	}

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
    //.entry = jpf_netif_receive_skb,
    .entry = jpf_ip_rcv,
    .kp = { 
        //.symbol_name = "netif_receive_skb",
        //.symbol_name = "__vlan_hwaccel_rx",
        //.symbol_name = "vlan_hwaccel_do_receive",
        //.symbol_name = "vlan_gro_receive",
        //.symbol_name = "vlan_tx_tag_present",
        //.symbol_name = "__vlan_hwaccel_rx",
        .symbol_name = "ip_rcv",
        //.symbol_name = "packet_rcv",
        //.symbol_name = "igb_receive_skb",
        //.symbol_name = "napi_gro_receive",
    },  
};
