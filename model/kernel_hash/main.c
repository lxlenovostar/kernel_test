/*
 *     Filename:  main.c
 *
 *  Description:
 *
 *      Version:  1.0
 *      Created:  05/05/2014 05:31:59 PM
 *     Revision:  none
 *     Compiler:  gcc
 *
 *       Author:  Hong Jinyi (hongjy), hongjy@chinanetcenter.com
 * Organization:  chinanetcenter
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>

#include "ws_sp_hash_table.h"
#include "nf.h"
#include "debug.h"

int __init minit(void)
{
	int err = 0;

	if (0 != (err = initial_sp_hash_table_cache()))
		goto out;

	if (0 > (err = nf_register_hook(&nf_out_ops))) {
		printk("Failed to register nf_out %s.\n", THIS_MODULE->name);
		goto err_nf_reg_out;
	}

	if (0 > (err = nf_register_hook(&nf_in_ops))) {
		printk("Failed to register nf_in %s.\n", THIS_MODULE->name);
		goto err_nf_reg_in;
	}

	if (0 > (err = nf_register_sockopt(&nf_opt))) {
		printk("Failed to register sockopt %s.\n", THIS_MODULE->name);
		goto err_nf_reg_opt;
	}

	printk("Init %s successful.\n", THIS_MODULE->name);
	goto out;

err_nf_reg_opt:
	nf_unregister_hook(&nf_in_ops);
err_nf_reg_in:
	nf_unregister_hook(&nf_out_ops);
err_nf_reg_out:
	release_sp_hash_table_cache();
out:
	return err;
}

void mexit(void)
{
	nf_unregister_sockopt(&nf_opt);
	nf_unregister_hook(&nf_in_ops);
	nf_unregister_hook(&nf_out_ops);
	release_sp_hash_table_cache();
	printk("Exit %s.\n", THIS_MODULE->name);
}

module_init(minit);
module_exit(mexit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("hongjy");
#ifdef DEBUG
MODULE_VERSION("1.4.1.debug");
#else
MODULE_VERSION("1.4.1");
#endif
