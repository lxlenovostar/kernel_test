#include <linux/percpu.h>
#include "alloc_mem.h"

DEFINE_PER_CPU(unsigned long *, w_cache_page); 

int alloc_percpu_page(void)
{
	int cpu;

	for_each_online_cpu(cpu) {
		per_cpu(w_cache_page, cpu) = (unsigned long *)__get_free_pages(GFP_ATOMIC, 0); 
		if (!per_cpu(w_cache_page, cpu)) {
			printk(KERN_ERR "alloc cache page for write failed.");
			return -ENOMEM;
		}
	}

	return 0;
}

void free_percpu_page() {
	int cpu;
	
	for_each_online_cpu(cpu) {
		if (per_cpu(w_cache_page, cpu))
			free_page((unsigned long)per_cpu(w_cache_page, cpu));	
	}	
}
