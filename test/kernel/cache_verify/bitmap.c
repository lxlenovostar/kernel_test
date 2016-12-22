#include <linux/percpu.h>
#include <linux/vmalloc.h>
#include <linux/bitmap.h>
#include "bitmap.h"

DEFINE_PER_CPU(unsigned long *, bitmap); //percpu-BITMAP
DEFINE_PER_CPU(unsigned long, bitmap_index); //percpu-BITMAP-index
unsigned long long bitmap_size = 0;

int alloc_bitmap() {
	int cpu;
	unsigned long long file_size = (FILESIZE * 1024 * 1024 * 1024);
	unsigned long long chunk_num = file_size / CHUNKSIZE;
	bitmap_size = chunk_num / 8;

	for_each_online_cpu(cpu) {
		//alloc memory for every percpu-value.
		per_cpu(bitmap, cpu) = vmalloc(bitmap_size);	//a bit for a chunk.
		if (!per_cpu(bitmap, cpu)) {
			printk(KERN_ERR "alloc bitmap failed.");
			return -ENOMEM;
		}
		bitmap_zero(per_cpu(bitmap, cpu), chunk_num);
		per_cpu(bitmap_index, cpu) = 0;	
	}

	return 0;	
}

void free_bitmap() {
	int cpu;
	
	for_each_online_cpu(cpu) {
		if (per_cpu(bitmap, cpu))
			vfree(per_cpu(bitmap, cpu));	
	}	
}

/*
 * bitmap_find_next_zero_area - find a contiguous aligned zero area
 * @map: The address to base the search on
 * @size: The bitmap size in bits
 * @start: The bitnumber to start searching at
 * @nr: The number of zeroed bits we're looking for
 * @align_mask: Alignment mask for zero area
 *
 * The @align_mask should be one less than a power of 2; the effect is that
 * the bit offset of all zero areas this function finds is multiples of that
 * power of 2. A @align_mask of 0 means no alignment is required.
 */
unsigned long bitmap_find_next_zero_area(unsigned long *map,
                                         unsigned long size,
                                         unsigned long start,
                                         unsigned int nr,
                                         unsigned long align_mask)
{
        unsigned long index, end, i;
again:
        index = find_next_zero_bit(map, size, start);

        /* Align allocation */
        index = __ALIGN_MASK(index, align_mask);

        end = index + nr;
        if (end > size)
                return end;
        i = find_next_bit(map, end, index);
        if (i < end) {
                start = i + 1;
                goto again;
        }
        return index;
}



