#include <linux/module.h>
#include <net/sock.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>
#include <linux/timer.h>

#define NETLINK_USER 31

struct sock *nl_sk = NULL;
int pid = 1;	
struct timer_list message_timer;

void send_message(unsigned long data)
{
	struct nlmsghdr *nlh;
    struct sk_buff *skb_out;
    int msg_size;
    char *msg = "send your message every 10s.";
    int res;

	if (pid == 1) {
        printk(KERN_ERR "userspace app maybe not ok.");
		goto out;
	}

	msg_size = strlen(msg);
    skb_out = nlmsg_new(msg_size, 0);
    if (!skb_out) {
        printk(KERN_ERR "Failed to allocate new skb");
        return;
    }

    nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, msg_size, 0);

    NETLINK_CB(skb_out).dst_group = 0; /* not in mcast group */
    strncpy(nlmsg_data(nlh), msg, msg_size);

    res = nlmsg_unicast(nl_sk, skb_out, pid);
    if (res < 0)
        printk(KERN_INFO "Error while sending bak to user"); 

out:
	mod_timer(&message_timer, jiffies + 10*HZ);	
}

void init_message_timer(void)
{
	init_timer(&message_timer);
	message_timer.expires = jiffies + 10*HZ;
	message_timer.data = 0;
	message_timer.function = send_message;
	add_timer(&message_timer);
}

static void nl_data_ready(struct sk_buff *skb)
{
	struct nlmsghdr *nlh;
    struct sk_buff *skb_out;
    int msg_size;
    char *msg = "get your pid";
    int res;

    nlh = (struct nlmsghdr *)skb->data;
    printk(KERN_INFO "Netlink get pid:%s", (char *)nlmsg_data(nlh));

    pid = nlh->nlmsg_pid; /* pid of sending process */

    msg_size = strlen(msg);
    skb_out = nlmsg_new(msg_size, 0);
    if (!skb_out) {
        printk(KERN_ERR "Failed to allocate new skb");
        return;
    }

    nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, msg_size, 0);

    NETLINK_CB(skb_out).dst_group = 0; /* not in mcast group */
    strncpy(nlmsg_data(nlh), msg, msg_size);

    res = nlmsg_unicast(nl_sk, skb_out, pid);
    if (res < 0)
        printk(KERN_INFO "Error while sending bak to user"); 
}

static int netlink_init(void) {
	nl_sk = netlink_kernel_create(&init_net, NETLINK_USER, 0, nl_data_ready, NULL, THIS_MODULE);
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

	init_message_timer();
	return res;
}

static void __exit my_module_exit(void)
{
	printk(KERN_INFO "Goodbye:%s", THIS_MODULE->name);
	sock_release(nl_sk->sk_socket);
	del_timer_sync(&message_timer);
}

module_init(my_module_init);
module_exit(my_module_exit);
MODULE_LICENSE("GPL");
