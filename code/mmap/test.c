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

#include <unistd.h>
#include <stdio.h>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>

#define THREAD_NUM 1 
#define PAGE 4096
#define SLOT 512
#define SIZE ((phymem_size - PAGE)/SLOT)
#define HEAD 42
void *mmap_addr = NULL;
void *mmap_addr_write = NULL;
unsigned long phymem_addr = 0;
unsigned long phymem_size = PAGE + 128*SLOT;
pthread_mutex_t read_lock;
pthread_mutex_t write_lock;

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

static inline void bitmap_zero(unsigned long *dst, int nbits)
{
	int len = BITS_TO_LONGS(nbits) * sizeof(unsigned long);
        memset(dst, 0, len);
}

void* thread_read(void *arg)
{
	char temp_buf[1000];
	int index = 0;

	while(1) {
		again: ;
			pthread_mutex_lock(&read_lock);
			index = find_first_bit(mmap_addr, SIZE);
			if (index == SIZE){
				printf("no memory slot read\n");
				pthread_mutex_unlock(&read_lock);
				sleep(0.2);
				goto again;
			}
			memcpy(temp_buf, mmap_addr + PAGE + index*SLOT, 100);
			bitmap_clear(mmap_addr, index, 1);
			pthread_mutex_unlock(&read_lock);
			printf("read index is %d\n", index);
			sleep(0.1);
	}
	return ((void *)0);
}

void* thread_write(void *arg)
{
	char temp_buf[1000];
	int index = 0;

	while(1) {
		again: ;
			pthread_mutex_lock(&write_lock);
			index = find_first_zero_bit(mmap_addr_write, SIZE);
			if (index == SIZE){
				printf("no memory slot write\n");
				pthread_mutex_unlock(&write_lock);
				sleep(0.2);
				goto again;
			}
			memcpy(mmap_addr_write + PAGE + index*SLOT + HEAD, temp_buf, 100);
			bitmap_set(mmap_addr_write, index, 1);
			pthread_mutex_unlock(&write_lock);
			printf("write index is %d\n", index);
			sleep(0.1);
	}
	return ((void *)0);
}

void handler(int sig)
{
#ifndef WIN32
        void *array[10];
        size_t size;
        size = backtrace(array, 10);
        int file_dump = open("/opt/dump.log", O_APPEND | O_RDWR);
        char message[7] = "BEGIN ";
        write(file_dump, message, 7);

        time_t now;
        struct tm *timenow;
        char strtemp[255];

        time(&now);
        timenow = localtime(&now);
        sprintf(strtemp, "recent time is : %s\n", asctime(timenow));

        int length=strlen(strtemp)+1;
        write(file_dump, strtemp, length);
  backtrace_symbols_fd(array, size, file_dump);

        close(file_dump);

        exit(1);
#endif
}

/*
 * should command : mknod /dev/wsmmap c 30 0
 */
int main(void)
{
	int fd_read, fd_write, i;

   	signal(SIGSEGV, handler);   // install our handler
	fd_read = open("/dev/wsmmap", O_RDWR);
	if(fd_read < 0) {
		perror("open");
		return -1;
	}

	mmap_addr = mmap(NULL, phymem_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd_read, 0);
	if(mmap_addr == MAP_FAILED) {
		perror("mmap");
	 	return -1;
	}
	
	fd_write = open("/dev/wsmmapsend", O_RDWR);
	if(fd_write < 0) {
		perror("open");
		return -1;
	}

	mmap_addr_write = mmap(NULL, phymem_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd_write, 0);
	if(mmap_addr_write == MAP_FAILED) {
		perror("mmap");
	 	return -1;
	}
	
	if (pthread_mutex_init(&read_lock, NULL) != 0){
		perror("init mutex");
	 	return -1;
	}
	
	if (pthread_mutex_init(&write_lock, NULL) != 0){
		perror("init mutex");
	 	return -1;
	}
	//int index = 0;
	//index = find_first_zero_bit(mmap_addr, (phymem_size-4096)/512);
	
	/*
	mmap_addr = malloc(sizeof(unsigned long));	
	bitmap_zero(mmap_addr, 8);
	bitmap_set(mmap_addr, 0, 1);
	bitmap_set(mmap_addr, 1, 1);
	find_first_zero_bit(mmap_addr, 8);
	*/
	
	int res, err_read, err_write;
	void *rret;
	pthread_t t_read[THREAD_NUM];
	pthread_t t_write[THREAD_NUM];
	for (i = 0; i < THREAD_NUM; ++i) {	
		printf("what0\n");
		res = pthread_create(&t_read[i], NULL, thread_read, NULL);
		if (res != 0){
			perror("create failed");
			return -1;
		}
		printf("what1\n");/*
		err = pthread_join(t_read[i], &rret);
		if (err != 0){
			perror("join failed");
			return -1;
		}
		printf("what2\n");*/
	}
	
	for (i = 0; i < THREAD_NUM; ++i) {	
		printf("what0\n");
		res = pthread_create(&t_write[i], NULL, thread_write, NULL);
		if (res != 0){
			perror("create failed");
			return -1;
		}
	}

	for (i = 0; i < THREAD_NUM; ++i) {	
		err_read = pthread_join(t_read[i], &rret);
		if (err_read != 0){
			perror("join failed");
			return -1;
		}
		
		err_write = pthread_join(t_write[i], &rret);
		if (err_write != 0){
			perror("join failed");
			return -1;
		}
		printf("what2\n");
	}
	
	mmap_addr = NULL;
	mmap_addr_write = NULL;
       	close(fd_read);
       	close(fd_write);
    	return 0;  
} 
