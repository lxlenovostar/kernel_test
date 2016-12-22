#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/hrtimer.h>

#define US_TO_NS(usec)      ((usec) * 1000)
#define MS_TO_US(msec)      ((msec) * 1000)
#define MS_TO_NS(msec)      ((msec) * 1000 * 1000)

DEFINE_PER_CPU(struct hrtimer, send_hr_timer);

enum hrtimer_restart hrtimer_send_skb(struct hrtimer *timer) 
{
    ktime_t ktime;        
    int cpu;
    //unsigned long delay_in_ms = 1L;                                                                                    
    unsigned long delay_in_us = 500L;                                                                                    
   
    //ktime = ktime_set(0, MS_TO_NS(delay_in_ms));                                                                         
    ktime = ktime_set(0, US_TO_NS(delay_in_us));                                                                         
    
	cpu = get_cpu();
    printk(KERN_INFO "hrtimer is on:%d", cpu);
	hrtimer_forward(&per_cpu(send_hr_timer, cpu), ktime_get(), ktime);
    put_cpu();

    return HRTIMER_RESTART;
    //return HRTIMER_NORESTART;
}


void init_hr_timer(void)  
{  
    ktime_t ktime;        
    int cpu;
    unsigned long delay_in_ms = 100L;                                                                                    
   
    ktime = ktime_set(0, MS_TO_NS(delay_in_ms));                                                                         
     
    for_each_online_cpu(cpu) {
        hrtimer_init(&per_cpu(send_hr_timer, cpu), CLOCK_MONOTONIC, HRTIMER_MODE_REL);
        per_cpu(send_hr_timer, cpu).function = &hrtimer_send_skb;
        hrtimer_start(&per_cpu(send_hr_timer, cpu), ktime, HRTIMER_MODE_REL);                                            
    }                     
}  

void del_hr_timer(void) 
{
    int cpu;

    for_each_online_cpu(cpu) {     
        /* del hrtimer. */
        hrtimer_cancel(&per_cpu(send_hr_timer, cpu));
    }
}


static int minit(void)
{
	int rc = 0;

	printk("Start %s.\n", THIS_MODULE->name);
    init_hr_timer(); 
        
	return rc;
}

static void mexit(void)
{
	printk("Exit %s.\n", THIS_MODULE->name);
    del_hr_timer();
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
