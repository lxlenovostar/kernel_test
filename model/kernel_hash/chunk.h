#include <linux/kfifo.h>
#include "uthash.h"

#define KFIFOLEN (1024*sizeof(int))

extern unsigned long RM;
extern unsigned long zero_value;
extern int zero_num;
extern unsigned long Q;
extern unsigned long R;
//extern int R = 10;
extern int chunk_num;  //控制最小值
 
struct tcp_chunk {
	uint8_t sha[20];            /* key */ 
    char name[10];
    UT_hash_handle hh;          /* makes this structure hashable */ 
};

void calculate_partition(char *playload, int playload_len, struct kfifo *fifo);
