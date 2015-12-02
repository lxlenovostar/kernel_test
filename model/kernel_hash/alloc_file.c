#include <linux/percpu.h>
#include <linux/syscalls.h>
#include <linux/file.h>
#include "alloc_file.h"
#include "debug.h"

DEFINE_PER_CPU(struct file *, reserve_file); 
DEFINE_PER_CPU(loff_t, loff_file); 

int alloc_percpu_file(void)
{
	int cpu;
	char *filename;
	char str_cpu[4];
		
	for_each_online_cpu(cpu) {
		//TODO: need configure file in the userspace.
		char name[200] = "/opt/wspackfile";
		memset(str_cpu, '\0', 4);
		sprintf(str_cpu, "%d", cpu);
		filename = strcat(name, str_cpu);
		//printk("cpu is:%s;file is:%s\n", str_cpu, filename);	
	
		//TODO: if the file not exist, the machine will carsh. the description is in Evernote.	
		per_cpu(reserve_file, cpu) = filp_open(filename, O_RDWR, 0);
		if (IS_ERR(per_cpu(reserve_file, cpu))) {
			printk(KERN_ERR "filp_open(%s) for failed\n", filename);
       		return -ENODEV;
   		}
	}

	return 0;
}

void free_percpu_file() {
	int cpu;
	
	for_each_online_cpu(cpu) {
		if (!IS_ERR(per_cpu(reserve_file, cpu)))
			fput(per_cpu(reserve_file, cpu));
	}
		
}
