#include <linux/kernel.h>
#include <linux/module.h>

#include <net/sock.h>
#include <net/netlink.h>

#define MY_MSG_TYPE (0x10 + 2)  // + 2 is arbitrary. same value for kern/usr

static struct sock *my_nl_sock;

DEFINE_MUTEX(my_mutex);

static int
my_rcv_msg(struct sk_buff *skb, struct nlmsghdr *nlh)
{
    int type;
    char *data;

    type = nlh->nlmsg_type;
    if (type != MY_MSG_TYPE) {
        printk("%s: expect %#x got %#x\n", __func__, MY_MSG_TYPE, type);
        return -EINVAL;
    }

    data = NLMSG_DATA(nlh);
    printk("%s: %02x %02x %02x %02x %02x %02x %02x %02x\n", __func__,
            data[0], data[1], data[2], data[3],
            data[4], data[5], data[6], data[7]);
    return 0;
}

static void
my_nl_rcv_msg(struct sk_buff *skb)
{
    mutex_lock(&my_mutex);
    netlink_rcv_skb(skb, &my_rcv_msg);
    mutex_unlock(&my_mutex);
}

static int
my_init(void)
{
    my_nl_sock = netlink_kernel_create(&init_net, NETLINK_USERSOCK, 0,
            my_nl_rcv_msg, NULL, THIS_MODULE);
    if (!my_nl_sock) {
        printk(KERN_ERR "%s: receive handler registration failed\n", __func__);
        return -ENOMEM;
    }

    return 0;
}

static void
my_exit(void)
{
    if (my_nl_sock) {
        netlink_kernel_release(my_nl_sock);
    }
}

module_init(my_init);
module_exit(my_exit);
