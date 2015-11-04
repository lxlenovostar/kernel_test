#include <linux/percpu.h>
#include <linux/vmalloc.h>

#define CHUNKSIZE 32
#define FILESIZE  1                 // 10G

extern unsigned long *percpu_ptr;

int alloc_bitmap(void);
void free_bitmap(void);
