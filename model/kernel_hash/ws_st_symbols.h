#include <linux/skbuff.h>

#define tcp_v4_rcv_addr 0xffffffff81464dd0
static int (*tcp_v4_rcv_ptr)(struct sk_buff *skb) = (int (*)(struct sk_buff *skb))tcp_v4_rcv_addr;

