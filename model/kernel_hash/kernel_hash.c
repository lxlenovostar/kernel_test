#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/crypto.h>
#include <linux/err.h>
#include <linux/scatterlist.h>
#include "debug.h"
#include "nf.h"
#include "chunk.h"
	
unsigned long RM = 1;
unsigned long zero_value = 1;
int zero_num = 6;
unsigned long Q = 1;
unsigned long R = 1048583;
int chunk_num = 32;  //控制最小值

void init_hash_parameters(void)
{
	int i;
	
	// precalculate
	for (i = 0; i < 60; ++i)
		Q = (2 * Q);

	for (i = 1; i <= chunk_num - 1; i++)
		RM = (R * RM) % Q;

	for (i = 0; i < zero_num; ++i)
        zero_value = (2 * zero_value);
    zero_value = zero_value - 1;
}

static int minit(void)
{
	int err = 0;

	init_hash_parameters();

	if (0 > (err = nf_register_hook(&nf_out_ops))) {
		printk(KERN_ERR "Failed to register nf_out %s.\n", THIS_MODULE->name);
		goto err_nf_reg_out;
	}

	if (0 > (err = nf_register_hook(&nf_in_ops))) {
		printk(KERN_ERR "Failed to register nf_in %s.\n", THIS_MODULE->name);
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
	/* free the hash table contents */
	struct tcp_chunk *current_chunk, *tmp;

    int i;
	
	nf_unregister_hook(&nf_in_ops);
	nf_unregister_hook(&nf_out_ops);
   	
	DEBUG_LOG("\n");
  	HASH_ITER(hh, hash_head, current_chunk, tmp) {
    	for (i = 0; i < 20; i++) {
        	DEBUG_LOG("%02x->", (current_chunk->sha)[i]&0xff);
    	}
    	DEBUG_LOG("\n%d\n", current_chunk->id);

    	HASH_DEL(hash_head,current_chunk);   /* delete; users advances to next */
    	kfree(current_chunk);                /* optional- if you want to free  */
  	}

	printk(KERN_INFO "\nsavenum is:%lu\nExit %s.\n", save_num, THIS_MODULE->name);
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
