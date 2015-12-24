#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/crypto.h>
#include <linux/err.h>
#include <linux/scatterlist.h>
#include <linux/kprobes.h>
#include <linux/limits.h> 
#include "debug.h"
#include "nf.h"
#include "chunk.h"
#include "sha.h"
#include "hash_table.h"
#include "bitmap.h"
#include "slab_cache.h"
#include "alloc_file.h"
	
unsigned long RM = 1;
unsigned long zero_value = 1;
int zero_num = 6;
unsigned long Q = 1;
unsigned long R = 1048583;
int chunk_num = 32;  //控制最小值
unsigned long long used_mem = 0ULL;

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

	if (0 > (err = alloc_slab()))
		goto err_alloc_slab;

	if (0 > (err = initial_hash_table_cache()))
		goto err_hash_table_cache;

	if (0 > (err = nf_register_hook(&nf_out_ops))) {
		printk(KERN_ERR "Failed to register nf_out %s.\n", THIS_MODULE->name);
		goto err_nf_reg_out;
	}
	
	if (tcp_alloc_sha1sig_pool() == NULL) { 
		printk(KERN_ERR "Failed to alloc sha1 pool %s.\n", THIS_MODULE->name);
		goto err_sha1siq_pool;
	}   

	goto out;

err_sha1siq_pool:
	tcp_free_sha1sig_pool();
err_nf_reg_out:
	nf_unregister_hook(&nf_out_ops);
err_hash_table_cache:
	release_hash_table_cache();
err_alloc_slab:
	free_slab();
out:
	return err;    
}

static void mexit(void)
{
	/* free the hash table contents */
	long tmp_save, tmp_sum;
	
	nf_unregister_hook(&nf_out_ops);

	release_hash_table_cache();
	free_slab();
	tcp_free_sha1sig_pool();
	
	tmp_save = atomic64_read(&save_num);
	tmp_sum =  atomic64_read(&sum_num);

	if (tmp_sum > 0)
		printk(KERN_INFO "Cache ratio is:%ld%%", (tmp_save*100)/tmp_sum);
	
	printk(KERN_INFO "savenum is:%ld; sumnum is:%ld,%ld(Mb);\nExit %s.", tmp_save, tmp_sum, (tmp_sum /1024 /1024 *8), THIS_MODULE->name);
	
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
