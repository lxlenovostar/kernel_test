#include <linux/percpu.h>
#include <linux/vmalloc.h>

#define CHUNKSIZE 32
#define FILESIZE  4UL //1G:4MB  10G:40MB

int alloc_bitmap(void);
void free_bitmap(void);
unsigned long bitmap_find_next_zero_area(unsigned long *map,
                                         unsigned long size,
                                         unsigned long start,
                                         unsigned int nr,
                                         unsigned long align_mask);

 
