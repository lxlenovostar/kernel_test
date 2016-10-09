#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

#define MD5_LEN 16

/** 
 * build tcp header. Learn by tcp_send_fin()
 */
int build_tcp(struct sk_buff *skb)
{
	skb->csum = 0;
    TCP_SKB_CB(skb)->flags = (TCPCB_FLAG_ACK | TCPCB_FLAG_FIN);
    TCP_SKB_CB(skb)->sacked = 0;
    skb_shinfo(skb)->gso_segs = 1;
    skb_shinfo(skb)->gso_size = 0;
    skb_shinfo(skb)->gso_type = 0;
 
    /* FIN eats a sequence byte, write_seq advanced by tcp_queue_skb(). */
    TCP_SKB_CB(skb)->seq = 123;
    TCP_SKB_CB(skb)->end_seq = TCP_SKB_CB(skb)->seq + 1;

	return 0;	
}

int copy_md5sum(struct sk_buff *skb)
{
	uint8_t md5_result[MD5LEN] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};

	if (skb_tail_rome(skb) < MD5_LEN)
		return 2;

	return 0;
}

static int minit(void)
{
	int err = 0;
	struct sk_buff *skb;
	int size;
   
	/* The TCP header must be at least 32-bit aligned.  */
    size = ALIGN(sizeof(struct ethhdr) + sizeof(struct iphdr) + sizeof(struct tcphdr) + MD5_LEN, 4);

	skb = alloc_skb(size, GFP_ATOMIC);
	if (skb == NULL)
		return 1;

	/* Reserve space for headers and prepare control bits. */
    skb_reserve(skb, size);

	err = copy_md5(skb);

	err = build_tcp(skb);

	return err;    
}

static void mexit(void)
{
	
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
