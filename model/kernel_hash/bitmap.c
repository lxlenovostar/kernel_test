#include <linux/percpu.h>
#include <linux/vmalloc.h>
#include "bitmap.h"

int alloc_bitmap() {
	int cpu;
	unsigned long *this;
	unsigned long file_size = (FILESIZE * 1024 * 1024 * 1024);
	unsigned long chunk_num = file_size / CHUNKSIZE;
	
	percpu_ptr = alloc_percpu(unsigned long);

	for_each_online_cpu(cpu) {
		this = *per_cpu_ptr(percpu_ptr, cpu);
		//alloc memory for every percpu-value.
		this = vmalloc(chunk_num);
		if (!this) {
			printk(KERN_ERR "alloc bitmap failed.");
			return -ENOMEM;
		}	
	}

	return 0;	
}

void free_bitmap() {
	int cpu;
	unsigned long *this;
	
	if (percpu_ptr) {
		for_each_online_cpu(cpu) {
			this = *per_cpu_ptr(percpu_ptr, cpu);
		
			if (this)
				vfree(this);	
		}
	}
	
	if (percpu_ptr)
		free_percpu(percpu_ptr);
}
