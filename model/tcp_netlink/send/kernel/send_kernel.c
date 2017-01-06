#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/timer.h>
#include <net/sock.h>
#include <linux/netlink.h>

#define PING_PONG_TYPE (0x10 + 3)  
#define MD5_TYPE (0x10 + 4)  

struct sock *nl_sk = NULL;
int pid = 1;	

static void nl_data_ready(struct sk_buff *skb)
{
	struct nlmsghdr *s_nlh;
	struct nlmsghdr *r_nlh;
    struct sk_buff *skb_out;
    int msg_size;
    char *msg = "get your pid";
    int res;
	int type;
	
	r_nlh = (struct nlmsghdr *)skb->data; 
    type = r_nlh->nlmsg_type;

	if (type == PING_PONG_TYPE) {
    	printk(KERN_INFO "Netlink received msg payload:%s", (char *)NLMSG_DATA(r_nlh));

    	pid = r_nlh->nlmsg_pid; /*pid of sending process */

    	msg_size = strlen(msg);
    	skb_out = nlmsg_new(msg_size, 0);
    	if (!skb_out) {
        	printk(KERN_ERR "Failed to allocate new skb");
    	}

    	s_nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, msg_size, 0);

    	NETLINK_CB(skb_out).dst_group = 0; /* not in mcast group */
    	strncpy(nlmsg_data(s_nlh), msg, msg_size);

    	res = nlmsg_unicast(nl_sk, skb_out, pid);
    	if (res < 0) {
        	printk(KERN_ERR "Error while sending back to user"); 
		}

		return;
	} else if (type == MD5_TYPE) {
    	printk(KERN_INFO "Netlink received MD5 payload:%s", (char *)NLMSG_DATA(r_nlh));
		return;
	} else {
        printk(KERN_ERR "%s: expect %#x or %#x got %#x", __func__, PING_PONG_TYPE, MD5_TYPE, type);
        return;
	}
}

static int netlink_init(void) {
	nl_sk = netlink_kernel_create(&init_net, NETLINK_USERSOCK, 0, nl_data_ready, NULL, THIS_MODULE);
    if (!nl_sk) {
        printk(KERN_ALERT "Error creating socket.");
        return -1;
    }

	return 0;
}

static int __init my_module_init(void)
{
	int res = 0;
	printk(KERN_INFO "Initializing Netlink Socket:%s", THIS_MODULE->name);

	res = netlink_init();
	if (res != 0)
		printk(KERN_ERR "Initializing Netlink Error");

	return res;
}

static void __exit my_module_exit(void)
{
	printk(KERN_INFO "Goodbye:%s", THIS_MODULE->name);
	sock_release(nl_sk->sk_socket);
}

module_init(my_module_init);
module_exit(my_module_exit);
MODULE_LICENSE("GPL");
