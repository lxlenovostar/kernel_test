#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/cdev.h>
#include <linux/version.h>
#include <linux/vmalloc.h>
#include <linux/ctype.h>
#include <linux/pagemap.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/proc_fs.h>
#include <asm/io.h>
#include <asm/atomic.h>
#include <asm/unaligned.h>
#include <linux/delay.h>
#include <asm/system.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <linux/tcp.h>
#include <linux/in.h>
#include <linux/version.h>
#include <net/ip.h>
#include <net/checksum.h>
#include <linux/inetdevice.h>
#include <linux/netdevice.h>
#include <net/ip.h>
#include <net/tcp.h>
#include <net/udp.h>
#include <net/icmp.h>
#include <net/arp.h>
#include <linux/if_arp.h>
#include <linux/if_ether.h>
#include <net/route.h>
#include <net/neighbour.h>
#include <net/netevent.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 32))
#include <net/net_namespace.h>
#endif
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/uaccess.h>
#include <linux/timer.h>
#include <linux/pid.h>
#include <linux/sched.h>
//#define USE_KMALLOC 

#define PAGE_ORDER   0
#define PAGES_NUMBER 1
#define SLOT 1024
#define NUM 2

#ifndef NIPQUAD
#define NIPQUAD(addr) \
		        ((unsigned char *)&addr)[0],\
        ((unsigned char *)&addr)[1],\
        ((unsigned char *)&addr)[2],\
        ((unsigned char *)&addr)[3]
#endif

spinlock_t lock = SPIN_LOCK_UNLOCKED;
static struct kmem_cache *skbuff_head_cache __read_mostly;
/*
 * The dest addr.
 */
char *dest_addr = "192.168.109.176";
//#define DST_MAC {0x00, 0x0c, 0x29, 0xdc, 0x2d, 0xf5}
#define DST_MAC {0x00, 0x0c, 0x29, 0x71, 0x18, 0xdd}

static int MAJOR_DEVICE = 30;
void * mmap_buf = 0;
unsigned long mmap_size = 4*1024;
//unsigned long mmap_size = 4*1024;

atomic_t packet_count;
atomic_t drop_count;
long int ts_begin = 0;

static int ws_open(struct inode *inode, struct file *file)
{
	return 0;    
}


#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36)	
#define LIN_IOCTL_NAME	.ioctl
int ws_ioctl(struct inode *inode, struct file *file, u_int cmd, u_long data)
#else
#define LIN_IOCTL_NAME	.unlocked_ioctl
long ws_ioctl(struct file *file, u_int cmd, u_long data)
#endif
{
    //todo
    return 0;    
}


int mmap_alloc(void)
{
		int i;    
		mmap_size = PAGE_ALIGN(mmap_size);

#ifdef USE_KMALLOC //for kmalloc
		mmap_buf = kzalloc(mmap_size, GFP_KERNEL);
		printk("kmalloc mmap_buf=%p\n", (void *)mmap_buf);
		if (!mmap_buf) {
			printk("kmalloc failed!\n");
			return -1;
		}
		for (page = virt_to_page(mmap_buf); page < virt_to_page(mmap_buf + mmap_size); page++) {
			SetPageReserved(page);
		}
#else //for vmalloc
		mmap_buf  = vmalloc(mmap_size);
		printk("vmalloc mmap_buf=%p  mmap_size=%ld\n", (void *)mmap_buf, mmap_size);
		if (!mmap_buf ) {
			printk("vmalloc failed!\n");
			return -1;
		}
		for (i = 0; i < mmap_size; i += PAGE_SIZE) {
			SetPageReserved(vmalloc_to_page(mmap_buf + i));
		}
#endif

		return 0;
}


int mmap_free(void)
{
#ifdef USE_KMALLOC
		struct page *page;
		for (page = virt_to_page(mmap_buf); page < virt_to_page(mmap_buf + mmap_size); page++) {
			ClearPageReserved(page);
		}
		kfree((void *)mmap_buf);
#else
		int i;
		for (i = 0; i < mmap_size; i += PAGE_SIZE) {
			ClearPageReserved(vmalloc_to_page(mmap_buf + i));
		}
		vfree((void *)mmap_buf);
#endif
		mmap_buf = NULL;
		return 0;
}


static int ws_mmap(struct file *f, struct vm_area_struct *vma)
{
		int ret;
		unsigned long pfn;
		unsigned long start = vma->vm_start;
		unsigned long size = PAGE_ALIGN(vma->vm_end - vma->vm_start);
		void * ptmp = mmap_buf;
		if (size > mmap_size || !mmap_buf) {
			return -EINVAL;
		}

#ifdef USE_KMALLOC
		pfn  = virt_to_phys(mmap_buf) >> PAGE_SHIFT;
		return remap_pfn_range(vma, start,  pfn, size, PAGE_SHARED);
#else
		/* loop over all pages, map it page individually */
		while (size > 0) {
		pfn = vmalloc_to_pfn(ptmp);
		if ((ret = remap_pfn_range(vma, start, pfn, PAGE_SIZE, PAGE_SHARED)) < 0) {
			return ret;
		}
		start += PAGE_SIZE;
		ptmp += PAGE_SIZE;
		size -= PAGE_SIZE;
		}
#endif
		return 0;
}


static int ws_release(struct inode *inode, struct file *file)
{
    return 0;
}


static const struct file_operations ws_fops ={    
		.owner = THIS_MODULE,
		//.write = ws_write,
		//.read = ws_read,
		.mmap = ws_mmap,
		//.ioctl = ws_ioctl,
		//.open = ws_open,
		//.release = ws_release,
};

unsigned int hook_local_in(unsigned int hooknum, struct sk_buff *skb, const struct net_device *in, 
								const struct net_device *out,int (*okfn)(struct sk_buff *));
#if ( LINUX_VERSION_CODE < KERNEL_VERSION(2,6,25) )
unsigned int hook_local_in_p(unsigned int hooknum, struct sk_buff **pskb, const struct net_device *in, 
								const struct net_device *out,int (*okfn)(struct sk_buff *))
{
	return hook_local_in(hooknum, *pskb, in, out, okfn);
}
#endif


unsigned int hook_local_in(unsigned int hooknum, struct sk_buff *skb, const struct net_device *in, const struct net_device *out,int (*okfn)(struct sk_buff *))
{
		/*
		 *   The IPv4 header is represented by the iphdr structure.
		 */
		struct iphdr *iph = ip_hdr(skb);
		/*
		 *  The UDP header
		 */
		struct udphdr *udph = NULL;
		struct iphdr *send_iph = NULL;
		struct ethhdr *eth = NULL;
		struct tcphdr *th = NULL;
		__be32 saddr, daddr;
		unsigned short sport, dport;
		unsigned short ulen;
		char in_buf[NUM];
		char out_buf[NUM];

		memset(in_buf, '0', NUM);

		if (iph->protocol != IPPROTO_TCP) {
			return NF_DROP;
		}

		th = (struct tcphdr*)(skb->data + iph->ihl*4);
		ulen = ntohs(iph->tot_len);
		saddr = iph->saddr;
		daddr = iph->daddr;
		sport = th->source;
		dport = th->dest;
		
		/*
		if (atomic_read(&packet_count) == 0) {
				struct timeval tv;
				do_gettimeofday(&tv);
				ts_begin = tv.tv_sec;
				//printk("second begin is %ld\n", ts_begin);
			}
		atomic_inc(&packet_count);
		*/
		if (ntohs(dport) == 80) {
			//memcpy(mmap_buf, in_buf, NUM);
			//memcpy(out_buf, mmap_buf, NUM);
			//atomic_inc(&drop_count);

			/*
			* send packet 
			*/
			spin_lock(&lock);
			int eth_len, udph_len, iph_len, len;
			eth_len = sizeof(struct ethhdr);
			iph_len = sizeof(struct iphdr);
			udph_len  = sizeof(struct udphdr);
			len	= eth_len + iph_len + udph_len;
			
			/*
			 * build a new sk_buff
			 */
			struct sk_buff *send_skb = kmem_cache_alloc_node(skbuff_head_cache, GFP_ATOMIC & ~__GFP_DMA, NUMA_NO_NODE);
			
			if (!send_skb) {
				return NF_DROP;
			}
			
			memset(send_skb, 0, offsetof(struct sk_buff, tail));
			atomic_set(&send_skb->users, 2);
			send_skb->head = mmap_buf + 1024;
			send_skb->data = mmap_buf + 1024;
			skb_reset_tail_pointer(skb);
			send_skb->end = skb->tail + len + NUM;
			kmemcheck_annotate_bitfield(skb, flags1);
			kmemcheck_annotate_bitfield(skb, flags2);

			//send_skb->ip_summed = CHECKSUM_PARTIAL;	
			
			struct skb_shared_info *shinfo;
			shinfo = skb_shinfo(skb);
			atomic_set(&shinfo->dataref, 2);
			shinfo->nr_frags  = 0;
			shinfo->gso_size = 0;
			shinfo->gso_segs = 0;
			shinfo->gso_type = 0;
			shinfo->ip6_frag_id = 0;
			shinfo->tx_flags.flags = 0;
			skb_frag_list_init(skb);
			memset(&shinfo->hwtstamps, 0, sizeof(shinfo->hwtstamps));

			printk("mmap_buf + 1024 is %p\n", mmap_buf + 1024);	
			skb_reserve(send_skb, len);
			printk("data %p, len is %d\n", send_skb->data, len);	
			skb_push(send_skb, sizeof(struct udphdr));
			printk("udp data %p\n", send_skb->data);	
			skb_reset_transport_header(send_skb);
		
			/*
 			 * just instead of copy datas.
 			 */	
			//send_skb->tail = send_skb->end;
			
			udph = udp_hdr(send_skb);
			udph->source = dport;
			udph->dest = htons(ntohs(dport) + 1);
			udph->len = htons(udph_len);
			udph->check = 0;
			udph->check = csum_tcpudp_magic(daddr, in_aton(dest_addr), udph_len, IPPROTO_UDP, csum_partial(udph, udph_len, 0));
				
			if (udph->check == 0)
				udph->check = CSUM_MANGLED_0;

			skb_push(send_skb, sizeof(struct iphdr));
			printk("ip data %p\n", send_skb->data);	
			skb_reset_network_header(send_skb);
			send_iph = ip_hdr(send_skb);

			// iph->version = 4; iph->ihl = 5; 
			put_unaligned(0x45, (unsigned char *)send_iph);
			send_iph->tos = 0;
			put_unaligned(htons(iph_len) + htons(udph_len), &(send_iph->tot_len));
			//send_iph->id       = htons(atomic_inc_return(&ip_ident));
			send_iph->id = 0;
			send_iph->frag_off = 0;
			send_iph->ttl = 64;
			send_iph->protocol = IPPROTO_UDP;
			send_iph->check = 0;
			put_unaligned(daddr, &(send_iph->saddr));
			put_unaligned(in_aton(dest_addr), &(send_iph->daddr));
			send_iph->check    = ip_fast_csum((unsigned char *)send_iph, send_iph->ihl);
			  
			struct net_device *dev = skb->dev;
			eth = (struct ethhdr *)skb_push(send_skb, ETH_HLEN);
			printk("eth data %p\n", send_skb->data);	
			skb_reset_mac_header(send_skb);
			send_skb->protocol = eth->h_proto = htons(ETH_P_IP);
			//printk("dev_addr is %p, len is %d", dev->dev_addr, ETH_ALEN);
			printk("h_source is %p, dev_addr is %p, len is %d", eth->h_source, dev->dev_addr, ETH_ALEN);
			memcpy(eth->h_source, dev->dev_addr, ETH_ALEN);
			u8 dst_mac[ETH_ALEN] = DST_MAC;
			memcpy(eth->h_dest, dst_mac, ETH_ALEN);
			send_skb->dev = dev;
			int result = dev_queue_xmit(send_skb);
			printk("result is %d\n", result);
			spin_unlock(&lock);
			return NF_DROP;
		}
		/*	
		printk("Packet %d:%d.%d.%d.%d:%d -> %d.%d.%d.%d:%d ulen=%d\n", \
						packet_count, NIPQUAD(saddr), ntohs(sport), NIPQUAD(daddr), ntohs(dport), ulen);
		for(i = 0 ; i < ulen; i++){
			printk("%02x ",  (unsigned char)*((skb->data)+i));
		}

		printk("what\n\n");*/

		return NF_ACCEPT;
}

static struct nf_hook_ops hook_ops[] = {
{
		.owner      	= THIS_MODULE,
		.pf         	= PF_INET,
		.priority  	= NF_IP_PRI_FIRST,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,25))
		.hook       	= hook_local_in_p,
		.hooknum	= NF_IP_LOCAL_IN,
#else
		.hook   	= hook_local_in,
		//.hooknum	= NF_INET_LOCAL_IN,
		.hooknum	= NF_INET_PRE_ROUTING,
#endif
		},
};

static void wsmmap_exit(void)
{
		int ret;
				
		/* time and speed */
		struct timeval tv;
		do_gettimeofday(&tv);
		long int t_second = tv.tv_sec;
		printk("cost time  is %ld , packet_count is %d, all packet speed is %d pkt//sec, port hook speed is %d pkt//sec\n", (t_second - ts_begin), atomic_read(&packet_count), atomic_read(&drop_count)/(t_second - ts_begin), atomic_read(&packet_count)/(t_second - ts_begin));

		ret = mmap_free( );
		if(ret) {
			printk("wsmmap: mmap free failed!\n");
			goto free_failed;        
		}

		unregister_chrdev(MAJOR_DEVICE,"wsmmap");  

		nf_unregister_hooks(hook_ops, ARRAY_SIZE(hook_ops));  

		printk("rmmod wsmmap module!\n");
		return ;

free_failed:
	msleep(3000);
	mmap_free( );  
}

int wsmmap_init(void)
{
		int ret;
		atomic_set(&packet_count, 0);
		atomic_set(&drop_count, 0);
		
		ret = mmap_alloc( );
		if(ret) {
			printk("wsmmap: mmap alloc failed!\n");
			goto alloc_failed;
		}

		
		ret = nf_register_hooks(hook_ops, ARRAY_SIZE(hook_ops));
		if (ret) {
			printk("wsmmap: local_in hooks failed\n");
			goto nf_failed;
		}
		

		ret = register_chrdev(MAJOR_DEVICE, "wsmmap", &ws_fops);
		if(ret) {
			printk("wsmmap: chrdev register failed\n");
			goto chr_failed;
		}  	

		skbuff_head_cache = kmem_cache_create("skbuff_head_cache_temp", sizeof(struct sk_buff), 0, SLAB_HWCACHE_ALIGN|SLAB_PANIC, NULL);
		/*
		int i; 
		for (i = 0; i < mmap_size; i += PAGE_SIZE){
		memset(mmap_buf + i, 'a', PAGE_SIZE);
		memset(mmap_buf + i + PAGE_SIZE - 1, '\0', 1);
		}*/
		printk("insmod module wsmmap successfully!\n");	
		return 0;

chr_failed:
	nf_unregister_hooks(hook_ops, ARRAY_SIZE(hook_ops));  
nf_failed:
	mmap_free( );
alloc_failed:    
    return ret;     
}


module_init(wsmmap_init);
module_exit(wsmmap_exit);

MODULE_LICENSE("GPL");
MODULE_VERSION("1.0.0");
MODULE_DESCRIPTION("wskmmap");
MODULE_AUTHOR("wssys");
