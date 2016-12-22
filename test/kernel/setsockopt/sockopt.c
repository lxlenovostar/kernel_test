/*
 *     Filename:  sockopt.c
 *
 *  Description:
 *
 *      Version:  1.0
 *      Created:  30/03/2015 11:35:16 AM
 *     Revision:  none
 *     Compiler:  gcc
 *
 *       Author:  lx, lxlenovostar@gmail.com
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/tcp.h>
#include <linux/string.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>


#define SOCKET_OPS_IPCHG	135
#define SOCKET_OPS_IPCHG_CHK	(SOCKET_OPS_IPCHG + 1)
#define SOCKET_OPS_MIN		SOCKET_OPS_IPCHG
#define SOCKET_OPS_MAX		(SOCKET_OPS_IPCHG_CHK + 1)

static int
get_ip(struct sock *sk, int cmd, void __user * user, int *len)
{
	u32 ret = 0;

	if (sk == NULL)
		return -1;

	if (cmd == SOCKET_OPS_IPCHG) {
		if (copy_to_user(user, &ret, sizeof (ret)))
			return -1;
		*len = sizeof (ret);
	} else if (cmd == SOCKET_OPS_IPCHG_CHK) {
		if (copy_to_user(user, "WSIPCHG", sizeof ("WSIPCHG")))
			return -1;
		*len = sizeof ("WSIPCHG");
	} else {
		return -1;
	}

	return 0;
}

static int
set_ip(struct sock *sk, int cmd, void __user * user, unsigned int len)
{
	if (sk == NULL)
		return -1;

	if (cmd == SOCKET_OPS_IPCHG) {
		u32 fakeip;

		if (copy_from_user(&fakeip, user, len))
			return -1;
		printk(KERN_INFO "set ip: %x\n", fakeip);
	} else {
		return -1;
	}

	return 0;
}

struct nf_sockopt_ops nf_opt = {
	.pf = PF_INET,
	.set_optmin = SOCKET_OPS_MIN,
	.set_optmax = SOCKET_OPS_MAX,
	.set = set_ip,
	.get_optmin = SOCKET_OPS_MIN,
	.get_optmax = SOCKET_OPS_MAX,
	.get = get_ip,
    .owner = THIS_MODULE,
};

int __init
minit(void)
{
	int err = 0;

	if (0 > (err = nf_register_sockopt(&nf_opt))) {
		printk("Failed to register sockopt %s.\n", THIS_MODULE->name);
		return err;
	}

	printk("Init %s successful.\n", THIS_MODULE->name);
	return err;
}

void
mexit(void)
{
	nf_unregister_sockopt(&nf_opt);
	printk("Exit %s.\n", THIS_MODULE->name);
}

module_init(minit);
module_exit(mexit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("hongjy");
