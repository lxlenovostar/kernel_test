#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/crypto.h>
#include <linux/err.h>
#include <linux/scatterlist.h>
#include "debug.h"
#include "nf.h"
#include "chunk.h"
#include "sha.h"
#include "hash_table.h"
	
unsigned long RM = 1;
unsigned long zero_value = 1;
int zero_num = 6;
unsigned long Q = 1;
unsigned long R = 1048583;
int chunk_num = 32;  //控制最小值
struct ws_sp_aligned_lock *hash_lock_array;

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
	percpu_counter_init(&save_num, 0);
	percpu_counter_init(&sum_num, 0);

	if (0 > (err = initial_hash_table_cache()))
		goto out;	

	printk(KERN_INFO "Start %s.", THIS_MODULE->name);

	if (0 > (err = nf_register_hook(&nf_out_ops))) {
		printk(KERN_ERR "Failed to register nf_out %s.\n", THIS_MODULE->name);
		goto err_nf_reg_out;
	}

	if (0 > (err = nf_register_hook(&nf_in_ops))) {
		printk(KERN_ERR "Failed to register nf_in %s.\n", THIS_MODULE->name);
		goto err_nf_reg_in;
	}    
	
	if (tcp_alloc_sha1sig_pool() == NULL) { 
		printk(KERN_ERR "Failed to alloc sha1 pool %s.\n", THIS_MODULE->name);
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
	unsigned long tmp_save, tmp_sum;

	nf_unregister_hook(&nf_in_ops);
	nf_unregister_hook(&nf_out_ops);
	release_hash_table_cache();	
	tcp_free_sha1sig_pool();
	
	tmp_save = percpu_counter_sum(&save_num);
	tmp_sum =  percpu_counter_sum(&sum_num);
	percpu_counter_destroy(&save_num);
	percpu_counter_destroy(&sum_num);
	printk(KERN_INFO "\nsavenum is:%lu; sumnum is:%lu,%lu(Mb); Cache ratio is:%lu%%\nExit %s.\n", tmp_save, tmp_sum, (tmp_sum /1024 /1024 *8), (tmp_save*100)/tmp_sum, THIS_MODULE->name);
	
}

module_init(minit);
module_exit(mexit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("lix");
#ifdef DEBUG
MODULE_VERSION("0.0.1.debug");
#else
MODULE_VERSION("0.0.1");
#endif
