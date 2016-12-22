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
#include "alloc.h"
	
unsigned long RM = 1;
unsigned long zero_value = 1;
int zero_num = 6;
unsigned long Q = 1;
unsigned long R = 1048583;
int chunk_num = 32;  //控制最小值
static int kprobe_in_reged = 0;
struct kmem_cache * hash_item_data; /* __read_mostly*/
//extern unsigned long *bitmap;
DECLARE_PER_CPU(unsigned long *, bitmap); //percpu-BITMAP

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

	if (0 > (err = alloc_percpu_file()))
		goto err_alloc_file;

	if (0 > (err = alloc_slab()))
		goto err_alloc_slab;

	if (0 > (err = alloc_bitmap()))
		goto err_bitmap;

	if (0 > (err = initial_hash_table_cache()))
		goto err_hash_table_cache;

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
		goto err_sha1siq_pool;
	}   

	err = register_jprobe(&jps_netif_receive_skb);
    if (err < 0) {
        printk(KERN_ERR "Failed to register jprobe netif_receive_skb %s.\n", THIS_MODULE->name);
        goto out;
    }
    kprobe_in_reged = 1; 

	goto out;

err_sha1siq_pool:
	tcp_free_sha1sig_pool();
err_nf_reg_in:
	nf_unregister_hook(&nf_in_ops);
err_nf_reg_out:
	nf_unregister_hook(&nf_out_ops);
err_hash_table_cache:
	release_hash_table_cache();
err_bitmap:
	free_bitmap();
err_alloc_slab:
	free_slab();
err_alloc_file:
	free_percpu_file();
out:
	return err;    
}

static void mexit(void)
{
	/* free the hash table contents */
	unsigned long tmp_save, tmp_sum;
	
	nf_unregister_hook(&nf_in_ops);
	nf_unregister_hook(&nf_out_ops);
	
	if (kprobe_in_reged)
        unregister_jprobe(&jps_netif_receive_skb);

	release_hash_table_cache();
	free_percpu_file();
	free_slab();
	free_bitmap();	
	tcp_free_sha1sig_pool();
	
	tmp_save = percpu_counter_sum(&save_num);
	tmp_sum =  percpu_counter_sum(&sum_num);
	percpu_counter_destroy(&save_num);
	percpu_counter_destroy(&sum_num);

	if (tmp_sum > 0)
		printk(KERN_INFO "Cache ratio is:%lu%%", (tmp_save*100)/tmp_sum);
	
	printk(KERN_INFO "savenum is:%lu; sumnum is:%lu,%lu(Mb);\nExit %s.", tmp_save, tmp_sum, (tmp_sum /1024 /1024 *8), THIS_MODULE->name);
	
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
