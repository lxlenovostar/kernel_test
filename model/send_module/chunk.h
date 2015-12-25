#include <linux/kfifo.h>
#include <linux/percpu_counter.h>
#include "hash_table.h"

#define CHUNKSTEP 32
#define SHALEN 20
#define KFIFOLEN (1024*sizeof(int))
#define MEMLIMIT (2*100*1024*1024)  /* Memory limit is 200M. */

extern unsigned long RM;
extern unsigned long zero_value;
extern int zero_num;
extern unsigned long Q;
extern unsigned long R;
//extern int R = 10;
extern int chunk_num;  //控制最小值
extern atomic64_t sum_num;
extern atomic64_t save_num;
extern atomic64_t skb_num;
extern unsigned long long used_mem;

void clear_remainder_skb(void);
void calculate_partition(char *playload, int playload_len, struct kfifo *fifo);

struct replace_item 
{
	struct list_head c_list;
	
	int start;
	int end;
	uint8_t sha1[SHA1SIZE];
};
