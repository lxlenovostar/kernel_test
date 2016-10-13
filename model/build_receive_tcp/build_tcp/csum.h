#ifndef csum_INC
#define csum_INC
#include <linux/net.h>

extern void skbcsum(struct sk_buff *, struct iphdr *);

#endif
