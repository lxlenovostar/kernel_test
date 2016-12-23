#include <linux/module.h>
#include <net/sock.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>

/*
 struct netlink_skb_parms
 {
	struct ucred            creds;          // Skb credentials      
    __u32                   pid;
    __u32                   groups;
    __u32                   dst_pid;
    __u32                   dst_groups;
    kernel_cap_t            eff_cap;
 };

 struct nlmsghdr
 {
    __u32       nlmsg_len;  // Length of message including header 
    __u16       nlmsg_type; // Message content 
    __u16       nlmsg_flags;    // Additional flags 
    __u32       nlmsg_seq;  // Sequence number 
    __u32       nlmsg_pid;  // Sending process port ID 
 };

*/

#define MY_MSG_TYPE (0x10 + 2)  // + 2 is arbitrary. same value for kern/usr

struct sock *nl_sk = NULL;
DEFINE_MUTEX(my_mutex);

static int 
nl_data_ready(struct sk_buff *skb, struct nlmsghdr *r_nlh)
{
	struct nlmsghdr *s_nlh;
    int pid;
    struct sk_buff *skb_out;
    int msg_size;
    char *msg = "Hello from kernel";
    int res;
	int type;

    printk(KERN_INFO "Entering: %s", __FUNCTION__);

    type = r_nlh->nlmsg_type;
    if (type != MY_MSG_TYPE) {
        printk(KERN_ERR "%s: expect %#x got %#x", __func__, MY_MSG_TYPE, type);
        return -EINVAL;
    }

    //nlh = (struct nlmsghdr *)skb->data;
    printk(KERN_INFO "Netlink received msg payload:%s", (char *)NLMSG_DATA(r_nlh));

    pid = r_nlh->nlmsg_pid; /*pid of sending process */

    msg_size = strlen(msg);
	/*
     nlmsg_new - Allocate a new netlink message
     * @payload: size of the message payload
     * @flags: the type of memory to allocate.
     */
    skb_out = nlmsg_new(msg_size, 0);
    if (!skb_out) {
        printk(KERN_ERR "Failed to allocate new skb");
        return -ENOMEM;
    }

	/**
      * nlmsg_put - Add a new netlink message to an skb
 	  * @skb: socket buffer to store message in
   	  * @pid: netlink process id
   	  * @seq: sequence number of message
   	  * @type: message type
   	  * @payload: length of message payload
   	  * @flags: message flags
   	  *
   	  * Returns NULL if the tailroom of the skb is insufficient to store
   	  * the message header and payload.
    */
    s_nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, msg_size, 0);

	/*
     #define NETLINK_CB(skb)     (*(struct netlink_skb_parms*)&((skb)->cb))
     */
    NETLINK_CB(skb_out).dst_group = 0; /* not in mcast group */
    strncpy(nlmsg_data(s_nlh), msg, msg_size);

	/**
 	  * nlmsg_unicast - unicast a netlink message
 	  * @sk: netlink socket to spread message to
 	  * @skb: netlink message as socket buffer
 	  * @pid: netlink pid of the destination socket
 	 */
    res = nlmsg_unicast(nl_sk, skb_out, pid);
    if (res < 0) {
        printk(KERN_INFO "Error while sending bak to user"); 
		return res;
	}

	return 0;
}

static void
nl_rcv_msg(struct sk_buff *skb)
{
    mutex_lock(&my_mutex);
    netlink_rcv_skb(skb, &nl_data_ready);
    mutex_unlock(&my_mutex);
}

static int __init my_module_init(void)
{
	printk(KERN_INFO "Initializing Netlink Socket:%s", THIS_MODULE->name);

	nl_sk = netlink_kernel_create(&init_net, NETLINK_USERSOCK, 0, nl_rcv_msg, NULL, THIS_MODULE);
    if (!nl_sk) {
        printk(KERN_ALERT "Error creating socket.");
        return -1;
    }

	return 0;
}

static void __exit my_module_exit(void)
{
	printk(KERN_INFO "Goodbye:%s", THIS_MODULE->name);
	sock_release(nl_sk->sk_socket);
}

module_init(my_module_init);
module_exit(my_module_exit);
MODULE_LICENSE("GPL");
