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
#include <asm/system.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <linux/in.h>
#include <linux/version.h>
#include <net/ip.h>
#include <linux/uaccess.h>
#include <linux/timer.h>
#include <linux/pid.h>
#include <linux/sched.h>
//#define USE_KMALLOC 

#define PAGE_ORDER   0
#define PAGES_NUMBER 1
#define SLOT 1024

#ifndef NIPQUAD
#define NIPQUAD(addr) \
		        ((unsigned char *)&addr)[0],\
        ((unsigned char *)&addr)[1],\
        ((unsigned char *)&addr)[2],\
        ((unsigned char *)&addr)[3]
#endif

static int MAJOR_DEVICE = 30;
void * mmap_buf = 0;
unsigned long mmap_size = 4*1024*1024*20;
//unsigned long mmap_size = 4*1024;

/*manage data*/
typedef struct {
		char *buffer;
		int length;
		volatile int start;
		volatile int end;
} RingBuffer;

RingBuffer * ring_buffer;
unsigned long packet_count = 0;

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

RingBuffer *RingBuffer_create(void *start_malloc, int length)
{
		RingBuffer *buffer = (RingBuffer *)start_malloc;
		buffer->length  = length + 1;
		buffer->start = 0;
		buffer->end = 0;
		buffer->buffer = (char *)start_malloc + SLOT;
		return buffer;
}

#define RingBuffer_available_data(B) ((B)->end % (B)->length - (B)->start)
#define RingBuffer_available_space(B) ((B)->length - (B)->end - 1)
#define RingBuffer_commit_write(B, A) ((B)->end = ((B)->end + (A)) % (B)->length)
#define RingBuffer_ends_at(B) ((B)->buffer + (B)->end)

int RingBuffer_write(RingBuffer *buffer, char *data, int length)
{
		if(RingBuffer_available_data(buffer) == 0) {
			buffer->start = buffer->end = 0;
		}

		if (length > RingBuffer_available_space(buffer)){
			printk("Not enough space: %d request, %d available", RingBuffer_available_data(buffer), length);
			return -1;
		}

		void *result = memcpy(RingBuffer_ends_at(buffer), data, length);
		if (result != RingBuffer_ends_at(buffer)){
			printk("Failed to write data into buffer.");
		return -1;
		}

		RingBuffer_commit_write(buffer, length);
		return length;
}

#define RingBuffer_starts_at(B) ((B)->buffer + (B)->start)
#define RingBuffer_commit_read(B, A) ((B)->start = ((B)->start + (A)) % (B)->length)
int RingBuffer_read(RingBuffer *buffer, char *target, int amount)
{
		if (amount > RingBuffer_available_data(buffer)){
			printk("Not enough in the buffer: has %d, needs %d\n", RingBuffer_available_data(buffer), amount);
			return -1;
		}

		void *result = memcpy(target, RingBuffer_starts_at(buffer), amount);
		if (result != target){
			printk("Failed to write buffer into data.");
			return -1;
		}

		RingBuffer_commit_read(buffer, amount);

		if(buffer->end == buffer->start) {
			buffer->start = buffer->end = 0;
		}

		return amount;
}

int kernel_thread_write(void *argc)
{
		char *a = "aa";
		char fix_buffer[SLOT];
		memcpy(fix_buffer, a, strlen(a));
		memset(fix_buffer + strlen(a), '\0', sizeof(fix_buffer) - strlen(a));

		int num = 2048;
		while (num){
			RingBuffer_write(ring_buffer, fix_buffer, SLOT);
			printk("write length is %d, start is %d, end is %d\n", ring_buffer->length, ring_buffer->start, ring_buffer->end);
			--num;
			msleep(10);
		}
		return 0;
}

int kernel_thread_read(void *argc)
{
		char mem_data[64];
		int num = 2;

		while (num){
			RingBuffer_read(ring_buffer, mem_data, 2);
			mem_data[2] = '\0';
		 	printk("read length is %d, start is %d, end is %d, data is %s\n", ring_buffer->length, ring_buffer->start, ring_buffer->end, mem_data);
		 	--num;
		 	msleep(10);
		}
		return 0;
}

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
		struct iphdr *iph = ip_hdr(skb);
		struct udphdr *uh = NULL;
		__be32 saddr, daddr;
		unsigned short sport, dport;
		unsigned short ulen;
		int i;

		/* Point into the IP datagram */ 
		__skb_pull(skb, ip_hdrlen(skb));
		skb_reset_transport_header(skb);

		/* judge start and host */
		if (skb->pkt_type != PACKET_HOST){
			goto exit_func;
		}
		
		if(!pskb_may_pull(skb, sizeof(struct udphdr))){ 
			goto exit_func;
		}

		if(iph->protocol != IPPROTO_UDP){
			goto exit_func;    
		}

		uh = udp_hdr(skb);
		ulen= skb->len - 8;
		skb->data += 8;
		saddr = iph->saddr;
		daddr = iph->daddr;
		sport = uh->source;
		dport = uh->dest;
		packet_count++;

		memcpy(mmap_buf + packet_count * 1024, skb->data, ulen);

		printk("Packet %d:%d.%d.%d.%d:%d -> %d.%d.%d.%d:%d ulen=%d\n", \
						packet_count, NIPQUAD(saddr), ntohs(sport), NIPQUAD(daddr), ntohs(dport), ulen);
		for(i = 0 ; i < ulen; i++){
			printk("%02x ",  (unsigned char)*((skb->data)+i));
		}

		printk("\n\n");

exit_func:
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
		.hooknum	= NF_INET_LOCAL_IN,
#endif
		},
};

static void wsmmap_exit(void)
{
		int ret;

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


		/*
		* Initialization RingBuffer
		*/
		ring_buffer = RingBuffer_create(mmap_buf, mmap_size - SLOT - 1);

		kernel_thread(kernel_thread_write, NULL, CLONE_KERNEL);
		//kernel_thread(kernel_thread_read, NULL, CLONE_KERNEL);
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
