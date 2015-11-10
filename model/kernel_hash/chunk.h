#include <linux/kfifo.h>
#include <linux/percpu_counter.h>

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
extern struct percpu_counter save_num;
extern struct percpu_counter sum_num;
extern struct workqueue_struct *writeread_wq; // for read/write file

typedef struct {
	struct work_struct wr_work;
	unsigned long      index;
	unsigned long      sum;	//the sum size memory will alloc.
} w_work_t;


//extern struct tcp_chunk *hash_head;
/* 
struct tcp_chunk {
	uint8_t *sha;                      
    int id;                           
    UT_hash_handle hh;                
};
*/

void calculate_partition(char *playload, int playload_len, struct kfifo *fifo);
