#include <linux/module.h>
#include <linux/kprobes.h>
#include <linux/kallsyms.h>
#include <linux/version.h>
#include <linux/ip.h>
#include <linux/in.h>
#include <linux/net.h>
#include <linux/tcp.h>

struct jprobe jp;

int jpf_ip_rcv(struct sk_buff *skb, struct net_device *dev, struct packet_type *pt, struct net_device *orig_dev)
{
	unsigned short sport, dport;
	__be32 saddr, daddr, seq;
	char dsthost[16];
	char ssthost[16];
	struct iphdr *iph;
	struct tcphdr *tcph;

	iph = (struct iphdr *)skb->data;
	//iph = ip_hdr(skb);
	if (iph->protocol == IPPROTO_TCP) {
		//tcph = tcp_hdr(skb); /*函数tcp_hdr只有到了TCP层才有意义*/
 		tcph = (struct tcphdr *)(skb->data + (iph->ihl << 2));	
		
		sport = tcph->source;
		dport = tcph->dest;
		saddr = iph->saddr;
		daddr = iph->daddr;

		snprintf(dsthost, 16, "%pI4", &daddr);
		snprintf(ssthost, 16, "%pI4", &saddr);
		
		//if (strcmp(ssthost, "218.6.111.34") == 0 && ntohs(dport) == 888) {  
		//if (ntohs(dport) == 888) {  
		if (strcmp(ssthost, "218.6.111.34") == 0 && strcmp(dsthost, "139.209.90.213") == 0 && ntohs(dport) == 888) {  
		//if (strcmp(ssthost, "218.6.111.34") == 0 && strcmp(dsthost, "139.209.90.213") == 0) {  
			seq = ntohl(tcph->seq);
			printk(KERN_INFO "new packet, seq is:%u", seq);
		}
   	}	

     jprobe_return();
     return 0;
}

static __init int init_jprobe_sample(void) 
{
	jp.kp.symbol_name = "ip_rcv";
	jp.entry = JPROBE_ENTRY(jpf_ip_rcv);
	register_jprobe(&jp);
	
	printk(KERN_INFO "Start %s.", THIS_MODULE->name);
	return 0;
}
module_init(init_jprobe_sample);

static __exit void cleanup_jprobe_sample(void)
{
	unregister_jprobe(&jp);
}
module_exit(cleanup_jprobe_sample);

MODULE_LICENSE("GPL");
