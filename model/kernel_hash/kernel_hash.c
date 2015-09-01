#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/crypto.h>
#include <linux/err.h>
#include <linux/scatterlist.h>
#include "debug.h"
#include "nf.h"

static int minit(void)
{
	int err = 0;

	if (0 > (err = nf_register_hook(&nf_out_ops))) {
		printk("Failed to register nf_out %s.\n", THIS_MODULE->name);
		goto err_nf_reg_out;
	}

	if (0 > (err = nf_register_hook(&nf_in_ops))) {
		printk("Failed to register nf_in %s.\n", THIS_MODULE->name);
		goto err_nf_reg_in;
	}    
	goto out;	

err_nf_reg_in:
	nf_unregister_hook(&nf_in_ops);
err_nf_reg_out:
	nf_unregister_hook(&nf_out_ops);
out:
	return err;    
}

static void mexit(void)
{
	nf_unregister_hook(&nf_in_ops);
	nf_unregister_hook(&nf_out_ops);
	printk("Exit %s.\n", THIS_MODULE->name);
}

module_init(minit);
module_exit(mexit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("lix");
#ifdef DEBUG
MODULE_VERSION("1.4.1.debug");
#else
MODULE_VERSION("1.4.1");
#endif
