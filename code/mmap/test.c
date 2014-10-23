#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define BITS_PER_LONG (sizeof(unsigned long) * 8)
#define BIT_WORD(nr)        ((nr) / BITS_PER_LONG)
#define BITMAP_FIRST_WORD_MASK(start) (~0UL << ((start) % BITS_PER_LONG))
#define BITMAP_LAST_WORD_MASK(nbits)                    \
(                                   \
    ((nbits) % BITS_PER_LONG) ?                 \
        (1UL<<((nbits) % BITS_PER_LONG))-1 : ~0UL       \
)

/*
 * __ffs - find first set bit in word
 * @word: The word to search
 *  
 * Undefined if no bit exists, so code should check against 0 first.
 */
static inline unsigned long __ffs(unsigned long word)
{
    asm("bsf %1,%0"
        : "=r" (word)
        : "rm" (word));
    return word;
}

/*
 * ffz - find first zero bit in word
 * @word: The word to search
 *
 * Undefined if no zero exists, so code should check against ~0UL first.
 */
static inline unsigned long ffz(unsigned long word)
{   
    asm("bsf %1,%0"
        : "=r" (word)
        : "r" (~word));
    return word;
}


/*
 * Find the first cleared bit in a memory region.
 */
unsigned long find_first_zero_bit(const unsigned long *addr, unsigned long size)
{
    const unsigned long *p = addr;
    unsigned long result = 0;
    unsigned long tmp;

    while (size & ~(BITS_PER_LONG-1)) {
        if (~(tmp = *(p++)))
            goto found;
        result += BITS_PER_LONG;
        size -= BITS_PER_LONG;
    }
    if (!size) 
        return result;

    tmp = (*p) | (~0UL << size);
    if (tmp == ~0UL)    /* Are any bits zero? */
        return result + size;   /* Nope. */
found:
    return result + ffz(tmp);
}

/*
 *Find the first set bit in a memory region.
 */ 
unsigned long find_first_bit(const unsigned long *addr, unsigned long size)
{
    const unsigned long *p = addr;
    unsigned long result = 0;
    unsigned long tmp;
    
    while (size & ~(BITS_PER_LONG-1)) {
        if ((tmp = *(p++)))
            goto found;
        result += BITS_PER_LONG;
        size -= BITS_PER_LONG;
    }   
    if (!size) 
        return result;  
        
    tmp = (*p) & (~0UL >> (BITS_PER_LONG - size));
    if (tmp == 0UL)     /* Are any bits set? */
        return result + size;   /* Nope. */
found:
    return result + __ffs(tmp);
}

void bitmap_clear(unsigned long *map, int start, int nr)
{
    unsigned long *p = map + BIT_WORD(start);
    const int size = start + nr;
    int bits_to_clear = BITS_PER_LONG - (start % BITS_PER_LONG);
    unsigned long mask_to_clear = BITMAP_FIRST_WORD_MASK(start);

    while (nr - bits_to_clear >= 0) {
        *p &= ~mask_to_clear;
        nr -= bits_to_clear;
        bits_to_clear = BITS_PER_LONG;
        mask_to_clear = ~0UL;
        p++;
    }
    if (nr) {
        mask_to_clear &= BITMAP_LAST_WORD_MASK(size);
        *p &= ~mask_to_clear;
    }
}

void bitmap_set(unsigned long *map, int start, int nr)
{
        unsigned long *p = map + BIT_WORD(start);
        const int size = start + nr;
        int bits_to_set = BITS_PER_LONG - (start % BITS_PER_LONG);
        unsigned long mask_to_set = BITMAP_FIRST_WORD_MASK(start);

        while (nr - bits_to_set >= 0) {
                *p |= mask_to_set;
                nr -= bits_to_set;
                bits_to_set = BITS_PER_LONG;
                mask_to_set = ~0UL;
                p++;
        }
        if (nr) {
                mask_to_set &= BITMAP_LAST_WORD_MASK(size);
                *p |= mask_to_set;
        }
}


void *mmap_addr = NULL;
unsigned long phymem_addr = 0;
unsigned long phymem_size = 4096+4096*512;


/*
 * should command : mknod /dev/wsmmap c 30 0
 */
int main(void)
{
	int fd;

	fd = open("/dev/wsmmap", O_RDWR);
	if(fd < 0) {
		perror("open");
		return 0;
	}

	mmap_addr = mmap(NULL, phymem_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	if(mmap_addr == MAP_FAILED) {
		perror("mmap");
	 	return 0;
	}

	
		
	/*int res;
	pthread_t t_read[THREAD_NUM];
	for (i = 0; i < THREAD_NUM; ++i) {	
		res = pthread_create(&t_read[i], NULL, thread_read, NULL);
		if (res != 0){
			perror("join failed");
			return -1;
		}
	}*/

	
	mmap_addr=NULL;
       	close(fd); 
    	return 0;    
}
