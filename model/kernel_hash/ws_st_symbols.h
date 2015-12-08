#include <linux/skbuff.h>

#define ip_rcv_finish_addr 0xffffffff814435e0
static int (*ip_rcv_finish_ptr)(struct sk_buff *skb) = (int (*)(struct sk_buff *skb))ip_rcv_finish_addr;

