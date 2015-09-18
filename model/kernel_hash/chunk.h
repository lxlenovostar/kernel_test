#include <linux/kfifo.h>
#include <linux/percpu_counter.h>
#include "uthash.h"

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
extern struct tcp_chunk *hash_head;
extern struct percpu_counter save_num;
extern struct percpu_counter sum_num;

 
struct tcp_chunk {
	uint8_t *sha;                     /* key */ 
    int id;                           /* file contetnt position */
    UT_hash_handle hh;                /* makes this structure hashable */ 
};

void calculate_partition(char *playload, int playload_len, struct kfifo *fifo);
