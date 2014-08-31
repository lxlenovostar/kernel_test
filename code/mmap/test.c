#include <stdio.h>
#include <stdlib.h>
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

#define SLOT 1024
unsigned long phymem_addr = 0;
unsigned long phymem_size = 4*1024*1024*20;
//unsigned long phymem_size = 4*1024;

/*manage data*/
typedef struct {
   char *buffer;
   int length;
   volatile int start;
   volatile int end;
} RingBuffer;

void *mmap_addr = NULL;
RingBuffer *ring = NULL;

#define RingBuffer_available_data(B) ((B)->end % (B)->length - (B)->start)
#define RingBuffer_starts_at(B) ((mmap_addr + SLOT) + (B)->start)
#define RingBuffer_commit_read(B, A) ((B)->start = ((B)->start + (A)) % (B)->length)

int RingBuffer_read(RingBuffer *buffer, char *target, int amount)
{
       printf("\nwhat1");
       if (amount > RingBuffer_available_data(buffer)){
                printf("Not enough in the buffer: has %d, needs %d", RingBuffer_available_data(buffer), amount);
                return -1;
        }
 
       printf("what2");
        void *result = memcpy(target, RingBuffer_starts_at(buffer), amount);
       printf("what3");
        if (result != target){
                 printf("Failed to write buffer into data.");
                 return -1;
        }

       printf("what4");
       RingBuffer_commit_read(buffer, amount);
       printf("what5");
 
        if(buffer->end == buffer->start) {
                 buffer->start = buffer->end = 0;
        }
       printf("what6\n");
        
	return amount;
}


void *thread_read(void *arg)
{
	char mem_data[SLOT];
	int num = 2048;

	while (num){
		RingBuffer_read(ring, mem_data, SLOT);
		mem_data[2] = '\0';
		printf("read length is %d, start is %d, end is %d, data is %s, buffer is %p\n", ring->length, ring->start, ring->end, mem_data, ring->buffer);
		--num;
		sleep(1);
	}
}

int main(void)
{
	int fd;
	int i=0;

	signal(SIGSEGV, handler);   // install our handler

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

	ring = (RingBuffer *)mmap_addr;
	
		
	int res;
	pthread_t t_read;
	
	res = pthread_create(&t_read, NULL, thread_read, NULL);
	if (res != 0){
		perror("join failed");
		return -1;
	}

	void *thread_r_read;
	res = pthread_join(t_read, &thread_r_read);
	if (res != 0){
		perror("Thread join failed");
		return -1;
	}
	

        /*
	// just test
	char *a = "aa";
	printf("length is %d, start is %d, end is %d\n", ring->length, ring->start, ring->end);
	RingBuffer_write(ring, a, 2); 
	printf("length is %d, start is %d, end is %d\n", ring->length, ring->start, ring->end);
	char mem_data[64];
	RingBuffer_read(ring, mem_data, 2);
	printf("length is %d, start is %d, end is %d, data is %s\n", ring->length, ring->start, ring->end, mem_data);
	*/
	
	mmap_addr=NULL;
       	close(fd); 
    	return 0;    
}
