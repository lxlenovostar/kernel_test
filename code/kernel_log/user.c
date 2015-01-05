#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define PAGE 4096
#define SLOT 4
void *mmap_addr = NULL;
unsigned long phymem_size = PAGE + 1024 * SLOT;

int
main(void)
{
	int fd_read;

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
	int i;
	for (i = 0; i < phymem_size; i += PAGE) {
		printf("mmap_addr               %d          is: %s\n", i, (char *)mmap_addr+i);
	}

	mmap_addr = NULL;
	close(fd_read);
	return 0;
}
