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
#define NUM 3
#define COPYNUM 140

#ifndef NIPQUAD
#define NIPQUAD(addr) \
		        ((unsigned char *)&addr)[0],\
        ((unsigned char *)&addr)[1],\
        ((unsigned char *)&addr)[2],\
        ((unsigned char *)&addr)[3]
#endif

spinlock_t lock = SPIN_LOCK_UNLOCKED;
spinlock_t rece_lock = SPIN_LOCK_UNLOCKED;
spinlock_t send_lock = SPIN_LOCK_UNLOCKED;
struct timer_list my_timer;
//static struct kmem_cache *skbuff_head_cache __read_mostly;
struct kmem_cache *skbuff_head_cache;
struct kmem_cache *skbuff_free_cache;

struct free_slab{
	struct sk_buff *free_mem;
	struct list_head list;
};
struct free_slab *tmp_slab;
struct free_slab *next_slab;
struct list_head head_free_slab;
struct list_head *pos;

/*
 * The dest addr.
 */
//char *dest_addr = "192.168.99.1";
//#define DST_MAC {0x00, 0x16, 0x31, 0xf0, 0x9d, 0xc4}
//char *dest_addr = "192.168.99.2";
//#define DST_MAC {0x00, 0x16, 0x31, 0xf0, 0x9e, 0x9e}
//char *dest_addr = "192.168.99.2";
//#define DST_MAC {0x00, 0x16, 0x31, 0xf0, 0x9e, 0x9e}
char *dest_addr = "192.168.204.130";
#define DST_MAC {0x00, 0x0c, 0x29, 0x45, 0x0a, 0x46}
#define SOU_MAC {0x00, 0x0c, 0x29, 0xdf, 0xdf, 0xb0}

static int MAJOR_DEVICE = 30;
void * mmap_buf = 0;
//unsigned long mmap_size = 4096+4096*4096;
unsigned long mmap_size = 4096+100*512;
#define  BITMAP_SIZE ((mmap_size-4096)/512)

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
		mmap_buf  = kmalloc(mmap_size, GFP_ATOMIC);
		printk("kmalloc mmap_buf=%p  mmap_size=%ld\n", (void *)mmap_buf, mmap_size);
		if (!mmap_buf) {
			printk("kmalloc failed!\n");
			return -1;
		}
		struct page *page;
		for (page = virt_to_page(mmap_buf); page < virt_to_page(mmap_buf + mmap_size); page++) {
			SetPageReserved(page);
		}
		
#endif

		return 0;
}


int mmap_free(void)
{
		struct page *page;
		for (page = virt_to_page(mmap_buf); page < virt_to_page(mmap_buf + mmap_size); page++) {
			ClearPageReserved(page);
		}
		kfree((void *)mmap_buf);
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

//#ifdef USE_KMALLOC
		pfn  = virt_to_phys(mmap_buf) >> PAGE_SHIFT;
		return remap_pfn_range(vma, start,  pfn, size, PAGE_SHARED);
//#else
		/* loop over all pages, map it page individually */
		/*while (size > 0) {
		pfn = vmalloc_to_pfn(ptmp);
		if ((ret = remap_pfn_range(vma, start, pfn, PAGE_SIZE, PAGE_SHARED)) < 0) {
			return ret;
		}
		start += PAGE_SIZE;
		ptmp += PAGE_SIZE;
		size -= PAGE_SIZE;
		}*/
//#endif
		//return 0;
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
		//char in_buf[100];
		int  index;
		int send_len;
		//char out_buf[NUM];

		//memset(in_buf, '0', 100);
	
		if (iph->protocol != IPPROTO_TCP) {
			//return NF_DROP;
			return NF_ACCEPT;
		}
		th = (struct tcphdr*)(skb->data + iph->ihl*4);
		ulen = ntohs(iph->tot_len);
		saddr = iph->saddr;
		daddr = iph->daddr;
		sport = th->source;
		dport = th->dest;
		
		if (ntohs(dport) == 80) {
			//memcpy(mmap_buf, in_buf, COPYNUM);
			//memcpy(out_buf, mmap_buf, NUM)		
					
			spin_lock(&rece_lock);
			index = find_first_zero_bit(mmap_buf, BITMAP_SIZE);
			if (index == BITMAP_SIZE){
				printk("receive mmap memory is full");
				//BUG();
			}
			printk("index zero is %d\n", index);
			memcpy(mmap_buf+ 4096 + index * 512, skb->data, skb->len);
			bitmap_set(mmap_buf, index, 1);
			spin_unlock(&rece_lock);



			/*
			* send packet 
			*/
			//spin_lock(&lock);
			int eth_len, udph_len, iph_len, len;
			eth_len = sizeof(struct ethhdr);
			iph_len = sizeof(struct iphdr);
			udph_len  = sizeof(struct udphdr);
			len	= eth_len + iph_len + udph_len;
			send_len = skb->len;
			/*
			 * build a new sk_buff
			 */
			struct sk_buff *send_skb = kmem_cache_alloc(skbuff_head_cache, GFP_ATOMIC & ~__GFP_DMA);
			
			if (!send_skb) {
				//spin_unlock(&lock);
				return NF_DROP;
			}
			
			//printk("what2\n");
			memset(send_skb, 0, offsetof(struct sk_buff, tail));
			atomic_set(&send_skb->users, 2);
			send_skb->cloned = 0;

			send_skb->head = mmap_buf + 1024;
			send_skb->data = mmap_buf + 1024;
		
			//printk("what3\n");
			skb_reset_tail_pointer(send_skb);
			send_skb->end = send_skb->tail + len + send_len;
			kmemcheck_annotate_bitfield(send_skb, flags1);
			kmemcheck_annotate_bitfield(send_skb, flags2);
			//printk("what4\n");

			send_skb->ip_summed = CHECKSUM_NONE;	
			
			struct skb_shared_info *shinfo;
			shinfo = skb_shinfo(send_skb);
			atomic_set(&shinfo->dataref, 1);
			shinfo->nr_frags  = 0;
			shinfo->gso_size = 0;
			shinfo->gso_segs = 0;
			shinfo->gso_type = 0;
			shinfo->ip6_frag_id = 0;
			shinfo->tx_flags.flags = 0;
			skb_frag_list_init(send_skb);
			memset(&shinfo->hwtstamps, 0, sizeof(shinfo->hwtstamps));

			//printk("what5\n");
			skb_reserve(send_skb, len + send_len);
			
			/*
 			 * instead of copy data
 			 */
			skb_push(send_skb, send_len);
			memcpy(send_skb->data, mmap_buf+ 4096 + index * 512, skb->len);
			bitmap_clear(mmap_buf, index, 1);
			printk("index %d is clear\n", index);
	
			skb_push(send_skb, sizeof(struct udphdr));
			skb_reset_transport_header(send_skb);
		
			/*
 			 * just instead of copy datas.
 			 */	
			//send_skb->tail = send_skb->end;
			
			//printk("what6\n");
			udph = udp_hdr(send_skb);
			udph->source = dport;
			udph->dest = htons(ntohs(dport) + 1);
			udph->len = htons(udph_len);
			udph->check = 0;
			udph->check = csum_tcpudp_magic(daddr, in_aton(dest_addr), udph_len, IPPROTO_UDP, csum_partial(udph, udph_len, 0));
				
			//if (udph->check == 0)
			//	udph->check = CSUM_MANGLED_0;

			skb_push(send_skb, sizeof(struct iphdr));
			skb_reset_network_header(send_skb);
			send_iph = ip_hdr(send_skb);

			put_unaligned(0x45, (unsigned char *)send_iph);
			send_iph->tos = 0;
			put_unaligned(htons(iph_len) + htons(udph_len), &(send_iph->tot_len));
			//printk("what7\n");
			send_iph->id = 0;
			send_iph->frag_off = 0;
			send_iph->ttl = 64;
			send_iph->protocol = IPPROTO_UDP;
			send_iph->check = 0;
			put_unaligned(daddr, &(send_iph->saddr));
			put_unaligned(in_aton(dest_addr), &(send_iph->daddr));
			send_iph->check    = ip_fast_csum((unsigned char *)send_iph, send_iph->ihl);
			  
			struct net_device *dev = ws_sp_get_dev(daddr);
			if (!dev){
				printk("NULL\n");
				return NF_DROP;
			}
			//printk("what8\n");
			eth = (struct ethhdr *)skb_push(send_skb, ETH_HLEN);
			skb_reset_mac_header(send_skb);
			send_skb->protocol = eth->h_proto = htons(ETH_P_IP);
			memcpy(eth->h_source, dev->dev_addr, ETH_ALEN);
			u8 dst_mac[ETH_ALEN] = DST_MAC;
			memcpy(eth->h_dest, dst_mac, ETH_ALEN);
			send_skb->dev = dev;
			//printk("what9\n");
			dev_queue_xmit(send_skb);
			//printk("what10\n");
			if (atomic_read(&(send_skb->users)) == 1){
				//printk("what10.5\n");
				kmem_cache_free(skbuff_head_cache, send_skb);
			}
			else
			{
				//printk("what10.6 users is %d\n", atomic_read(&send_skb->users));
				struct free_slab *ptr = kmem_cache_alloc(skbuff_free_cache, GFP_ATOMIC & ~__GFP_DMA);
				ptr->free_mem = send_skb;
				spin_lock(&lock);
				list_add(&ptr->list, &head_free_slab);
				spin_unlock(&lock);
			}

			//kfree(send_skb);
			//printk("what11\n");
			return NF_DROP;
			//return NF_ACCEPT;
		}

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

void my_function(unsigned long data)
{
	//printk("HELLO WORLD\n");
	list_for_each_entry_safe_reverse(tmp_slab, next_slab, &head_free_slab, list)
	{
		if (atomic_read(&((tmp_slab->free_mem)->users)) == 1){
			//printk("what2 is %d\n", atomic_read(&(tmp_slab->free_mem)->users));
			//struct free_slab *tmp_free = list_entry(tmp_slab->list, struct free_slab, list);
			list_del(&tmp_slab->list);
			struct free_slab *tmp_free = tmp_slab;
			//printk("what2.0\n");
			struct sk_buff *tmp_buff = tmp_slab->free_mem;
			//printk("what2.1\n");
			kmem_cache_free(skbuff_head_cache, tmp_buff);
			//printk("what2.2\n");
			tmp_buff = NULL;
			kmem_cache_free(skbuff_free_cache, tmp_free);
			//printk("what2.3\n");
			tmp_free = NULL;
		}
		
	}
	mod_timer(&my_timer, jiffies + 5*HZ);
}

static void wsmmap_exit(void)
{
		int ret;
				
		struct timeval tv;
		do_gettimeofday(&tv);

		del_timer(&my_timer);
		
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
		if (skbuff_head_cache == NULL){
			printk("alloc slab is failed.\n");
			return 1;
		}
		
		skbuff_free_cache = kmem_cache_create("skbuff_free_cache_temp", sizeof(struct free_slab), 0, SLAB_HWCACHE_ALIGN|SLAB_PANIC, NULL);
		if (skbuff_free_cache == NULL){
			printk("alloc slab is failed.\n");
			return 1;
		}
		

		/*
 		 * install a timer which free the slab.
 		 */		
		setup_timer(&my_timer, my_function, 0);
		my_timer.expires = jiffies + 10*HZ;
		add_timer(&my_timer);

		INIT_LIST_HEAD(&head_free_slab);
					

		bitmap_zero(mmap_buf, BITMAP_SIZE);	
		/*printk("size is %d\n", BITMAP_SIZE);	
		bitmap_set(mmap_buf, 0, 1);
		bitmap_set(mmap_buf, 1, 1);
		bitmap_set(mmap_buf, 2, 1);
		bitmap_set(mmap_buf, 3, 1);
		bitmap_set(mmap_buf, 4, 1);
		bitmap_set(mmap_buf, 5, 1);
		bitmap_set(mmap_buf, 6, 1);
		bitmap_set(mmap_buf, 7, 1);
		int index = find_first_bit(mmap_buf, BITMAP_SIZE);	
		printk("index set is %d\n", index);*/
		//bitmap_clear(mmai
		//int index = find_first_zero_bit(mmap_buf, BITMAP_SIZE);
		//printk("index zero is %d\n", index);
		

		printk("insmod module wsmmap successfully!\n");	

		/*
		int i; 
		for (i = 0; i < mmap_size; i += PAGE_SIZE){
		memset(mmap_buf + i, 'a', PAGE_SIZE);
		memset(mmap_buf + i + PAGE_SIZE - 1, '\0', 1);
		}*/
		
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
