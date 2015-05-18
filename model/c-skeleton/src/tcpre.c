#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "lcthw/dbg.h"
#include "lcthw/hashmap.h"

#define SIZE 1024
#define PACKET_LEN 1460

char packet[PACKET_LEN];

/*
 * choose 1460 or the other bytes calculate their hash values.
 */
char * 
get_packet()
{
	return NULL;
}

int
main()
{
	FILE *input;
	char *source = NULL;
	char *data = NULL;
	char *temp = NULL;
	int copy_len = 0;
	Hashmap *map = NULL;
	long Q = 1;
	int i; 

	for (i = 0; i < 60; ++i)
    	Q = (2 * Q);
	printf("Q is %ld, int_max is %d, and %ld\n", Q, INT_MAX, INT_MAX-Q);
	//printf("long is %d and int is %d and size_t is %d\n", sizeof(long), sizeof(int), sizeof(size_t));

	input = fopen("/root/c-skeleton/bin/history.txt", "r");
	check((input != NULL), "Can't find the file.");

	// bulid a hash map
	Hashmap_create(Q, NULL, NULL);		

	source = (char *)malloc(SIZE);
	check((source != NULL), "Can't alloc merrory.");
	
	data = (char *)malloc(SIZE);
	check((data != NULL), "Can't alloc merrory.");
	
	temp = (char *)malloc(SIZE);
	check((temp != NULL), "Can't alloc merrory.");
	
	while(fgets(source, SIZE, input) != NULL){
		// First: delete the '\n'
		int len = strlen(source);
		//printf("len is %d and source is %s\n", len, source);
		strncpy(temp, source, len-1);
		temp[len-1] = '\0';
	
		// Second: copy and calculate the hash value 	
		len = strlen(temp);
		strncpy(packet, temp, len);
		copy_len += len;
	
		if (copy_len >= PACKET_LEN) {
			//handle the hash

			copy_len = 0;
			memset(packet, '\0', PACKET_LEN);
		}	
		//printf("len is %d and temp is %s\n", strlen(temp), temp);
		memset(temp, '\0', SIZE);
		//printf("len is %d and tempend is %s\n", strlen(temp), temp);
	}


	fclose(input);
	free(source);
	free(data);
	free(temp);
	return 0;

    error:
		free(source);
		free(data);
		free(temp);
		return -1;
}
