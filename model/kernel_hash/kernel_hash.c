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
	int cpu, err = 0;
	struct timer_list *this;

	init_hash_parameters();
	percpu_counter_init(&save_num, 0);
	percpu_counter_init(&sum_num, 0);
	
	for_each_online_cpu(cpu) {
		this = &per_cpu(my_timer, cpu);
		setup_timer(this, prune, (unsigned long) this_list);
		this->expires = jiffies + (6 + cpu) * HZ;
		add_timer_on(this, cpu);
	}
	
	printk(KERN_INFO "\nStart %s.\n", THIS_MODULE->name);

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
	struct tcp_chunk *current_chunk, *tmp;
	uint8_t *sha;
    int i, cpu;
	unsigned long tmp_save, tmp_sum;
	struct timer_list *this;

	percpu_counter_destroy(&save_num);
	percpu_counter_destroy(&sum_num);

	for_each_online_cpu(cpu) {
		this = &per_cpu(my_timer, cpu);
		printk("del timer cpu id is %d\n", cpu);
		del_timer_sync(this);
	}
	
	nf_unregister_hook(&nf_in_ops);
	nf_unregister_hook(&nf_out_ops);
	tcp_free_sha1sig_pool();
   	
	DEBUG_LOG("\n");
  	HASH_ITER(hh, hash_head, current_chunk, tmp) {
    	for (i = 0; i < 20; i++) {
        	DEBUG_LOG("%02x->", (current_chunk->sha)[i]&0xff);
    	}
    	DEBUG_LOG("\n%d\n", current_chunk->id);

    	HASH_DEL(hash_head,current_chunk);   /* delete; users advances to next */
    	
		sha = current_chunk->sha;
		kfree(current_chunk);                
    	kfree(sha);                
  	}

	/*	
	tmp_save = get_cpu_var(save_num);
	put_cpu_var(save_num);
	tmp_sum = get_cpu_var(sum_num);
	put_cpu_var(sum_num);
	printk(KERN_INFO "\nsavenum is:%lu; sumnum is:%lu\nExit %s.\n", tmp_save, tmp_sum, THIS_MODULE->name);
	*/
	
	tmp_save = percpu_counter_sum(&save_num);
	tmp_sum =  percpu_counter_sum(&sum_num);
	printk(KERN_INFO "\nsavenum is:%lu; sumnum is:%lu\nExit %s.\n", tmp_save, tmp_sum, THIS_MODULE->name);
	
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
