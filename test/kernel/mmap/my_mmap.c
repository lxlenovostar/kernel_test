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

#define PAGE_ORDER   0
#define PAGES_NUMBER 1
#define PACKET_LEN 20

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
struct kmem_cache *skbuff_free_cache;
struct percpu_counter packets;
struct percpu_counter xmit_packets;
struct percpu_counter free_packets;

struct free_slab {
	struct sk_buff free_mem;
	int free_index;
	struct list_head list;
};
static DEFINE_PER_CPU(struct list_head, head_free_slab);
static DEFINE_PER_CPU(struct timer_list, my_timer);
struct timeval tv;
long int begin;
long int end;

/*
 * The dest addr.
 */
//const char *dest_addr = "192.168.204.130";
const char *dest_addr = "192.168.99.2";
__be32 dest = 0;
//#define DST_MAC {0x00, 0x0c, 0x29, 0x45, 0x0a, 0x46}
#define DST_MAC {0x00, 0x16, 0x31, 0xf0, 0x9e, 0x9e}
static u8 dst_mac[ETH_ALEN] = DST_MAC;
//char *dest_addr = "192.168.204.130";
//#define DST_MAC {0x00, 0x0c, 0x29, 0x45, 0x0a, 0x46}

static int MAJOR_DEVICE = 50;
static int MAJOR_DEVICE_SEND = 51;
void *mmap_buf = 0;
void *mmap_buf_send = 0;
void *mmap_buf_pend = 0;
#define SLOT 1500
unsigned long mmap_size = 4096 + 1024 * SLOT;
#define  BITMAP_SIZE ((mmap_size-4096)/SLOT)

struct net_device *
ws_sp_get_dev(__be32 ip)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 32)
	struct list_head *net_iter;
	struct list_head *net_device_iter;
	struct net *net;
#endif

	struct net_device *netdev;
	struct in_ifaddr *ifa;
	__be32 netmask;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 32)
	list_for_each(net_iter, &net_namespace_list) {
		net = list_entry(net_iter, struct net, list);
		list_for_each(net_device_iter, &net->dev_base_head) {
			netdev = list_entry(net_device_iter,
					    struct net_device, dev_list);
			if (unlikely(!netdev->ip_ptr))
				continue;
			for (ifa =
			     ((struct in_device *) netdev->ip_ptr)->ifa_list;
			     ifa; ifa = ifa->ifa_next) {
				netmask = ifa->ifa_mask;
				if ((ip & netmask) ==
				    (ifa->ifa_address & netmask)) {
					if (netdev->flags & IFF_LOOPBACK)
						continue;
					goto out;
				}

			}
		}
	}

#else
	for (netdev = dev_base; netdev; netdev = netdev->next) {
		if (netdev->ip_ptr) {
			for (ifa =
			     ((struct in_device *) netdev->ip_ptr)->ifa_list;
			     ifa; ifa = ifa->ifa_next) {
				netmask = ifa->ifa_mask;
				if ((ip & netmask) ==
				    (ifa->ifa_address & netmask)) {
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

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36)
#define LIN_IOCTL_NAME	.ioctl
int
ws_ioctl(struct inode *inode, struct file *file, u_int cmd, u_long data)
#else
#define LIN_IOCTL_NAME	.unlocked_ioctl
long
ws_ioctl(struct file *file, u_int cmd, u_long data)
#endif
{
	return 0;
}

int
mmap_alloc(void)
{
	struct page *page;
	mmap_size = PAGE_ALIGN(mmap_size);
	mmap_buf = kmalloc(mmap_size, GFP_ATOMIC);
	mmap_buf_send = kmalloc(mmap_size, GFP_ATOMIC);

	printk("kmalloc mmap_buf=%p, mmap_buf_send=%p, mmap_size=%ld\n",
	       (void *) mmap_buf, (void *) mmap_buf_send, mmap_size);
	if (!mmap_buf) {
		printk("kmalloc failed!\n");
		return -1;
	}

	if (!mmap_buf_send) {
		printk("kmalloc failed!\n");
		return -1;
	}
	for (page = virt_to_page(mmap_buf);
	     page < virt_to_page(mmap_buf + mmap_size); page++) {
		SetPageReserved(page);
	}

	for (page = virt_to_page(mmap_buf_send);
	     page < virt_to_page(mmap_buf_send + mmap_size); page++) {
		SetPageReserved(page);
	}

	return 0;
}

int
mmap_free(void)
{
	struct page *page;

	for (page = virt_to_page(mmap_buf);
	     page < virt_to_page(mmap_buf + mmap_size); page++) {
		ClearPageReserved(page);
	}

	for (page = virt_to_page(mmap_buf_send);
	     page < virt_to_page(mmap_buf_send + mmap_size); page++) {
		ClearPageReserved(page);
	}

	kfree((void *) mmap_buf);
	kfree((void *) mmap_buf_send);
	mmap_buf = NULL;
	mmap_buf_send = NULL;

	return 0;
}

static int
ws_mmap(struct file *f, struct vm_area_struct *vma)
{
	unsigned long pfn;
	unsigned long start = vma->vm_start;
	unsigned long size = PAGE_ALIGN(vma->vm_end - vma->vm_start);
	if (size > mmap_size || !mmap_buf) {
		return -EINVAL;
	}

	pfn = virt_to_phys(mmap_buf) >> PAGE_SHIFT;
	return remap_pfn_range(vma, start, pfn, size, PAGE_SHARED);
}

static int
ws_mmap_send(struct file *f, struct vm_area_struct *vma)
{
	int ret;
	unsigned long pfn;
	unsigned long start = vma->vm_start;
	unsigned long size = PAGE_ALIGN(vma->vm_end - vma->vm_start);
	if (size > mmap_size || !mmap_buf_send) {
		return -EINVAL;
	}

	pfn = virt_to_phys(mmap_buf_send) >> PAGE_SHIFT;
	return remap_pfn_range(vma, start, pfn, size, PAGE_SHARED);
}

static const struct file_operations ws_fops = {
	.owner = THIS_MODULE,
	//.write = ws_write,
	//.read = ws_read,
	.mmap = ws_mmap,
	//.ioctl = ws_ioctl,
	//.open = ws_open,
	//.release = ws_release,
};

static const struct file_operations ws_fops_send = {
	.owner = THIS_MODULE,
	.mmap = ws_mmap_send,
};

unsigned int hook_local_in(unsigned int hooknum, struct sk_buff *skb,
			   const struct net_device *in,
			   const struct net_device *out,
			   int (*okfn) (struct sk_buff *));
#if ( LINUX_VERSION_CODE < KERNEL_VERSION(2,6,25) )
unsigned int
hook_local_in_p(unsigned int hooknum, struct sk_buff **pskb,
		const struct net_device *in, const struct net_device *out,
		int (*okfn) (struct sk_buff *))
{
	return hook_local_in(hooknum, *pskb, in, out, okfn);
}
#endif

void
my_function(unsigned long data)
{
	struct free_slab *tmp;
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
	}
	mod_timer(&get_cpu_var(my_timer), jiffies + 10);
	put_cpu_var(my_timer);
}

unsigned int
hook_local_in(unsigned int hooknum, struct sk_buff *skb,
	      const struct net_device *in, const struct net_device *out,
	      int (*okfn) (struct sk_buff *))
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
	int index, next_index;
	int send_len, test_len;
	char in_buf[PACKET_LEN];
	struct net_device *dev;
	int eth_len, udph_len, iph_len, len;
	//int send_index = 0;
	int send_index = smp_processor_id();
	//int send_index = get_cpu();
	//put_cpu();
	index = send_index;

	//char out_buf[NUM];
	send_len = PACKET_LEN;
	test_len = PACKET_LEN;
	//memset(in_buf, '0', test_len);

	if (iph->protocol != IPPROTO_TCP) {
		return NF_ACCEPT;
	}
	th = (struct tcphdr *) (skb->data + iph->ihl * 4);
	saddr = iph->saddr;
	daddr = iph->daddr;
	sport = th->source;
	dport = th->dest;

	if (likely(ntohs(dport) == 80)) {
		//percpu_counter_inc(&packets);
		//memcpy(mmap_buf, in_buf, test_len);
		memcpy(mmap_buf + 4096 + index * SLOT, in_buf, test_len);
		eth_len = sizeof (struct ethhdr);
		iph_len = sizeof (struct iphdr);
		udph_len = sizeof (struct udphdr);
		len = eth_len + iph_len + udph_len;
		/*
		 * build a new sk_buff
		 */
		struct free_slab *ptr = kmem_cache_zalloc(skbuff_free_cache,
							    GFP_ATOMIC &
							    ~__GFP_DMA);
		if (unlikely(!ptr)){
			return NF_DROP;
		}
		
		struct sk_buff *send_skb = (struct sk_buff*)(&(ptr->free_mem));
		atomic_set(&send_skb->users, 2);

		send_skb->head = mmap_buf_send + 4096 + send_index * SLOT;
		send_skb->data = send_skb->head;

		send_skb->ip_summed = CHECKSUM_NONE;
		skb_reset_tail_pointer(send_skb);
		send_skb->end = send_skb->tail + len + send_len;

		struct skb_shared_info *shinfo;
		shinfo = skb_shinfo(send_skb);
		shinfo->nr_frags = 0;
		shinfo->gso_size = 0;
		shinfo->gso_segs = 0;
		shinfo->gso_type = 0;
		shinfo->ip6_frag_id = 0;
		shinfo->tx_flags.flags = 0;
		atomic_set(&shinfo->dataref, 1);
		skb_frag_list_init(send_skb);
		//memset(&shinfo->hwtstamps, 0, sizeof (shinfo->hwtstamps));

		skb_reserve(send_skb, len + send_len);
		skb_push(send_skb, send_len + udph_len);
		skb_reset_transport_header(send_skb);

		udph = udp_hdr(send_skb);
		udph->source = dport;
		udph->dest = htons(ntohs(dport) + 1);
		udph->len = htons(udph_len);
		/*udph->check =
		    csum_tcpudp_magic(daddr, in_aton(dest_addr), udph_len,
				      IPPROTO_UDP, csum_partial(udph, udph_len,
								0));
		*/udph->check =
		    csum_tcpudp_magic(daddr, dest, udph_len,
				      IPPROTO_UDP, 0);
		
		skb_push(send_skb, sizeof (struct iphdr));
		skb_reset_network_header(send_skb);
		send_iph = ip_hdr(send_skb);

		put_unaligned(0x45, (unsigned char *) send_iph);

		send_iph->tos = 0;
		//put_unaligned(htons(iph_len) + htons(udph_len),&(send_iph->tot_len));
		//send_iph->tot_len = htons(iph_len + udph_len);
		send_iph->tot_len = htons(skb->len);
		send_iph->id = 0;
		send_iph->frag_off = 0;
		send_iph->ttl = 64;
		send_iph->protocol = IPPROTO_UDP;
		//put_unaligned(daddr, &(send_iph->saddr));
		//put_unaligned(dest, &(send_iph->daddr));
		send_iph->daddr = daddr;
		//send_iph->saddr = in_aton(dest_addr);
		//send_iph->saddr = dest;
		send_iph->saddr = 0;
		//send_iph->check =
		//    ip_fast_csum((unsigned char *) send_iph, send_iph->ihl);
		
		dev = skb->dev;
		/*dev = ws_sp_get_dev(daddr);
		if (unlikely(!dev)) {
			return NF_DROP;
		}*/

		eth = (struct ethhdr *) skb_push(send_skb, ETH_HLEN);
		skb_reset_mac_header(send_skb);
		eth->h_proto = htons(ETH_P_IP);
		send_skb->protocol = eth->h_proto;
		memcpy(eth->h_source, dev->dev_addr, ETH_ALEN);
		memcpy(eth->h_dest, dst_mac, ETH_ALEN);
		send_skb->dev = dev;
		dev_queue_xmit(send_skb);
		if (unlikely(atomic_read(&(send_skb->users)) == 1)){
			kmem_cache_free(skbuff_free_cache, ptr);
			//percpu_counter_inc(&free_packets);
		}
		else{
			ptr->free_index = send_index;
			list_add(&ptr->list, &get_cpu_var(head_free_slab));
			put_cpu_var(head_free_slab);
		}
		return NF_DROP;
	}

	return NF_ACCEPT;
}

static struct nf_hook_ops hook_ops[] = {
	{
	 .owner = THIS_MODULE,
	 .pf = PF_INET,
	 .priority = NF_IP_PRI_FIRST,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,25))
	 .hook = hook_local_in_p,
	 .hooknum = NF_IP_LOCAL_IN,
#else
	 .hook = hook_local_in,
	 //.hooknum      = NF_INET_LOCAL_IN,
	 .hooknum = NF_INET_PRE_ROUTING,
#endif
	 },
};

static void
wsmmap_exit(void)
{
	int ret, cpu, packet_num;
	struct timer_list *this;

	do_gettimeofday(&tv);
	end = tv.tv_sec;

	kfree(mmap_buf_pend);

	for_each_online_cpu(cpu) {
		this = &per_cpu(my_timer, cpu);
		printk("del timer other cpu id is %d\n", cpu);
		del_timer_sync(this);
	}

	packet_num = percpu_counter_sum(&packets);
	printk("rece all packets is %d and flow is %d\n", packet_num,
	       packet_num * 42 * 8 / 1024 / 1024 / (end - begin));
	
	packet_num = percpu_counter_sum(&xmit_packets);
	printk("send all packets is %d\n", packet_num);
	
	packet_num = percpu_counter_sum(&free_packets);
	printk("send all packets is %d\n", packet_num);

	percpu_counter_destroy(&packets);
	percpu_counter_destroy(&free_packets);
	percpu_counter_destroy(&xmit_packets);

	unregister_chrdev(MAJOR_DEVICE, "wsmmap");
	unregister_chrdev(MAJOR_DEVICE_SEND, "wsmmapsend");

	nf_unregister_hooks(hook_ops, ARRAY_SIZE(hook_ops));

	kmem_cache_destroy(skbuff_free_cache);

	ret = mmap_free();
	if (ret) {
		printk("wsmmap: mmap free failed!\n");
		goto free_failed;
	}
	return;

      free_failed:
	msleep(3000);
	mmap_free();
}

int
wsmmap_init(void)
{
	int ret, cpu;
	struct timer_list *this;
	struct list_head *this_list;

	ret = mmap_alloc();
	if (ret) {
		printk("wsmmap: mmap alloc failed!\n");
		goto alloc_failed;
	}

	ret = nf_register_hooks(hook_ops, ARRAY_SIZE(hook_ops));
	if (ret) {
		printk("wsmmap: local_in hooks failed\n");
		goto nf_failed;
	}

	ret = register_chrdev(MAJOR_DEVICE, "wsmmap", &ws_fops);
	if (ret) {
		printk("wsmmap: chrdev register failed\n");
		goto chr_failed;
	}

	ret = register_chrdev(MAJOR_DEVICE_SEND, "wsmmapsend", &ws_fops_send);
	if (ret) {
		printk("wsmmap: chrdev register failed\n");
		goto chr_failed;
	}

	skbuff_free_cache =
	    kmem_cache_create("skbuff_free_cache_temp",
			      sizeof (struct free_slab), 0,
			      SLAB_HWCACHE_ALIGN | SLAB_PANIC, NULL);
	if (skbuff_free_cache == NULL) {
		printk("alloc slab is failed.\n");
		return 1;
	}

	bitmap_zero(mmap_buf, BITMAP_SIZE);
	bitmap_zero(mmap_buf_send, BITMAP_SIZE);
	mmap_buf_pend = kmalloc(1024, GFP_ATOMIC);
	bitmap_zero(mmap_buf_pend, BITMAP_SIZE);

	for_each_online_cpu(cpu) {
		this_list = &per_cpu(head_free_slab, cpu);
		INIT_LIST_HEAD(this_list);
	}
	
	for_each_online_cpu(cpu) {
		this = &per_cpu(my_timer, cpu);
		this_list = &per_cpu(head_free_slab, cpu);
		setup_timer(this, my_function, (unsigned long) this_list);
		this->expires = jiffies + (6 + cpu) * HZ;
		add_timer_on(this, cpu);
	}
	
	percpu_counter_init(&packets, 0);
	percpu_counter_init(&xmit_packets, 0);
	percpu_counter_init(&free_packets, 0);

	printk("insmod module wsmmap successfully!\n");
	do_gettimeofday(&tv);
	begin = tv.tv_sec;
	dest = in_aton(dest_addr);

	return 0;

      chr_failed:
	nf_unregister_hooks(hook_ops, ARRAY_SIZE(hook_ops));
      nf_failed:
	mmap_free();
      alloc_failed:
	return ret;
}

module_init(wsmmap_init);
module_exit(wsmmap_exit);

MODULE_LICENSE("GPL");
MODULE_VERSION("1.0.0");
MODULE_DESCRIPTION("wskmmap");
MODULE_AUTHOR("wssys");
