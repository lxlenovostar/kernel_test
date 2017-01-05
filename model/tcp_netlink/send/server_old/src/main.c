#include    <stdio.h>       /* for convenience */
#include    <stdlib.h>      /* for convenience */
#include    <string.h>      /* for convenience */
#include    <unistd.h>      /* for convenience */
#include    <pthread.h>
#include	"../lib/error.h"
#include	"../lib/lx_netlink.h"
#include	"../lib/libevent_api.h"

int netlink_fd;

int 
ping_pong_kernel(void) 
{
	int res = 0;

	res = init_sock();
	if (res != 0) {
		printf("netlink socket build failed.\n");
		return res;
	}

	res = check_netlink_status();  
	if (res != 0) {
		printf("kernel module don't insmod. Please insmod it.\n");
		free_netlink_resource();		
	}
	else 
		printf("kernel module insmod succcess.\n");

	return res;
} 

static void* 
thread_comm_kernel(void *arg) 
{
	long res;
	res = ping_pong_kernel();

    return (void *)res;
}

static void* 
thread_comm_server(void *arg) 
{
	/* start socket by libevent. */
	run();

	return NULL;
}

int
main(int argc, char *argv[])
{
    pthread_t kernel_tid, socket_tid;
	int res;
	void *res_check;

	res = pthread_create(&kernel_tid, NULL, thread_comm_kernel, NULL);
	if (res != 0) {
		err_quit("netlink thread creation failed");
	}
	
	res = pthread_join(kernel_tid, &res_check);
	if (res != 0) {
		err_quit("Thread join failed");
	}

	if ((long)res_check != 0) {
		err_quit("insmod kernel module failed");
	}
	
	res = pthread_create(&socket_tid, NULL, thread_comm_server, NULL);
	if (res != 0) {
		err_quit("tcp socket thread creation failed");
	}

	res = pthread_join(socket_tid, NULL);
	if (res != 0) {
		err_quit("Thread join failed");
	}

	printf("Process will end\n");
    exit(0);
}
