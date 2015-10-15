#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>

extern struct nf_hook_ops nf_in_ops;
extern struct nf_hook_ops nf_out_ops;
extern struct jprobe jps_netif_receive_skb;
