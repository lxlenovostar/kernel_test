#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define PAGE 4096
#define SLOT 512
void *mmap_addr = NULL;
unsigned long phymem_size = PAGE + 1024 * SLOT;
#define SIZE ((phymem_size - PAGE)/SLOT)

#define BITS_PER_LONG (sizeof(unsigned long) * 8)
#define BIT_WORD(nr)        ((nr) / BITS_PER_LONG)
#define BITMAP_FIRST_WORD_MASK(start) (~0UL << ((start) % BITS_PER_LONG))
#define BITMAP_LAST_WORD_MASK(nbits)                    \
		(                                   \
											    ((nbits) % BITS_PER_LONG) ?                 \
											        (1UL<<((nbits) % BITS_PER_LONG))-1 : ~0UL       \
		)
#define BITS_PER_BYTE 8
#define DIV_ROUND_UP(n,d) (((n) + (d) - 1) / (d))
#define BITS_TO_LONGS(nr)   DIV_ROUND_UP(nr, BITS_PER_BYTE * sizeof(long))

/*
 * __ffs - find first set bit in word
 * @word: The word to search
 *   
 * Undefined if no bit exists, so code should check against 0 first.
 */
static inline unsigned long
__ffs(unsigned long word)
{
      asm("bsf %1,%0":"=r"(word)
      :    "rm"(word));
	return word;
}

/*
 * Find the first set bit in a memory region.
 */
unsigned long
find_first_bit(const unsigned long *addr, unsigned long size)
{
	const unsigned long *p = addr;
	unsigned long result = 0;
	unsigned long tmp;
	while (size & ~(BITS_PER_LONG - 1)) {
		if ((tmp = *(p++)))
			goto found;
		result += BITS_PER_LONG;
		size -= BITS_PER_LONG;
	}
	if (!size)
		return result;

	tmp = (*p) & (~0UL >> (BITS_PER_LONG - size));
	if (tmp == 0UL)		/* Are any bits set? */
		return result + size;	/* Nope. */
      found:
	return result + __ffs(tmp);
}

void
bitmap_clear(unsigned long *map, int start, int nr)
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

int
main(void)
{
	int fd_read, index;

	index = 0;

	fd_read = open("/dev/eudyptula", O_RDWR);
	if (fd_read < 0) {
		perror("open");
		printf("what1\n");
		return -1;
	}

	printf("what2\n");
	mmap_addr =
	    mmap(NULL, phymem_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_read,
		 0);
	if (mmap_addr == MAP_FAILED) {
		perror("mmap");
		printf("what3\n");
		return -1;
	}
	printf("what4\n");

	/*
	 * just for test 
	 * */
	/*int i;
	   for (i = 0; i < phymem_size; i += PAGE) {
	   printf("mmap_addr               %d          is: %s\n", i,
	   (char *) mmap_addr + i);
	   } */

	/*
	 * just for test 
	 * */
	char temp_buf[2000];
	while (1) {
	      again:;
		index = find_first_bit(mmap_addr, SIZE);
		if (index == SIZE) {
			printf("no memory slot read\n");
			sleep(0.2);
			goto again;
		}

		memcpy(temp_buf, mmap_addr + PAGE + index * SLOT, SLOT);
		bitmap_clear(mmap_addr, index, 1);
		printf("read index is %d\n", index);
		sleep(0.1);
	}

	mmap_addr = NULL;
	close(fd_read);
	return 0;
}
