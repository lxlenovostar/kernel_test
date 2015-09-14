#ifndef debug_INC
#define debug_INC

#include <linux/ip.h>
#include <linux/in.h>
#include <linux/net.h>
#include <linux/tcp.h>

//#define DEBUG

#ifdef DEBUG
#define DEBUG_LOG(format, args...) printk(format, ## args)
#else
#define DEBUG_LOG(format, args...)
#endif

#define PRINT_LEN	64

extern void printT(struct sk_buff *skb);

#endif
