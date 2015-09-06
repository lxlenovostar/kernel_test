#include <linux/err.h>
#include "chunk.h"

#define STEP 32
int step = STEP - 1; //控制块长

/*
 * Compute hash for a string.
 */
unsigned long pack_hash(char *key, int M, int R, long Q)
{
    int j;
    unsigned long h = 0;

    for (j = 0; j < M; j++) {
        h = (R * h + key[j]) % Q;
    }
    
	return h;
    //printf("%lu\n", h);
}

void calculate_partition(char *playload, int playload_len, struct kfifo *fifo) 
{
	int i;
	int delay_time = 0;
	unsigned long txthash = pack_hash(playload, chunk_num, R, Q);

	/*
     * Fix up:
	 * if (kfifo_avail(fifo) < sizeof(int)*2048)
	 *      BUG;
     */
	
	if (likely(delay_time == 0)) {
		if ((txthash & zero_value) == 0) {
		//if ((txthash & zero_value) == 0 || check_data_point(playload, Q, R, (chunk_num - 1))) {
		//if (check_data_point(playload, Q, R, (chunk_num - 1))) {
			int pos = chunk_num - 1;
			//kfifo_in(fifo, &pos, sizeof(pos));
			kfifo_put(fifo, (unsigned char *)&pos, sizeof(pos));
			printk(KERN_INFO "pos is:%d->%lu", pos, txthash);
			delay_time = step;
		}
	} else {
		delay_time--;
	}

	for (i = chunk_num; i < playload_len; i++) {
		txthash = (txthash + Q - RM * playload[i - chunk_num] % Q) % Q;
		txthash = (txthash * R + playload[i]) % Q;

		if (delay_time == 0) {
			if ((txthash & zero_value) == 0) {
				//if ((txthash & zero_value) == 0 || check_data_point(playload, Q, R, i)) {
				//if (check_data_point(playload, Q, R, i)) {
				//kfifo_in(fifo, &i, sizeof(i));
				kfifo_put(fifo, (unsigned char *)&i, sizeof(i));
				printk(KERN_INFO "i is:%d->%lu", i, txthash);
				delay_time = step;
			}
		} else {
			delay_time--;
		}
	}
}
