/* For sockaddr_in */
#include <netinet/in.h>
/* For socket functions */
#include <sys/socket.h>
/* For fcntl */
#include <fcntl.h>

#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>

#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#define MAX_LINE 16384

void read_cb(struct bufferevent *bev, void *ctx)
{
#define MAX_LINE1    256
    char line[MAX_LINE1+1];
    int n;
    evutil_socket_t fd = bufferevent_getfd(bev);

    while (n = bufferevent_read(bev, line, MAX_LINE1), n > 0) {
        line[n] = '\0';
        printf("fd=%u, read line: %s\n", fd, line);

        bufferevent_write(bev, line, n);
    }
}

void error_cb(struct bufferevent *bev, short event, void *arg)
{
	evutil_socket_t fd = bufferevent_getfd(bev);
    printf("fd = %u, ", fd);
    if (event & BEV_EVENT_TIMEOUT) {
        printf("Timed out\n"); //if bufferevent_set_timeouts() called
    }
    else if (event & BEV_EVENT_EOF) {
        printf("connection closed\n");
    }
    else if (event & BEV_EVENT_ERROR) {
        printf("some other error\n");
    }
    bufferevent_free(bev);
}

void
do_accept(evutil_socket_t listener, short event, void *arg)
{
    struct event_base *base = (struct event_base *)arg;
    struct sockaddr_storage ss;
    socklen_t slen = sizeof(ss);
    int fd = accept(listener, (struct sockaddr*)&ss, &slen);
    if (fd < 0) {
        perror("accept");
    } else if (fd > FD_SETSIZE) {
        perror("fd > FD_SETSIZE\n");
        close(fd);
    } else {
        struct bufferevent *bev;
        evutil_make_socket_nonblocking(fd);
		/* Create a new socket bufferevent over an existing socket. */
        bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
		/* Changes the callbacks for a bufferevent. */
        bufferevent_setcb(bev, read_cb, NULL, error_cb, NULL);
		/*
         Sets the watermarks for read and write events.

		 On input, a bufferevent does not invoke the user read callback unless there is 
         at least low watermark data in the buffer. If the read buffer is beyond the high 
         watermark, the bufferevent stops reading from the network.

		 On output, the user write callback is invoked whenever the buffered data falls 
         below the low watermark. Filters that write to this bufev will try not to write 
         more bytes to this buffer than the high watermark would allow, except when flushing.
         */	
        bufferevent_setwatermark(bev, EV_READ, 0, MAX_LINE);
		/* Enable a bufferevent. */
        bufferevent_enable(bev, EV_READ|EV_WRITE);
    }
}

void
run(void)
{
    evutil_socket_t listener;
    struct sockaddr_in sin;
    struct event_base *base;
    struct event *listener_event;

	/*
     The event_base_new() function allocates and returns a new event base with the default settings. 
     It examines the environment variables and returns a pointer to a new event_base. If there is an 
     error, it returns NULL.
 	
	 When choosing among methods, it picks the fastest method that the OS supports.
     */
    base = event_base_new();
    if (!base)
        return; /*XXXerr*/

    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = 0;
    sin.sin_port = htons(9877);

    listener = socket(AF_INET, SOCK_STREAM, 0);
	/* Do platform-specific operations as needed to make a socket nonblocking. */
    evutil_make_socket_nonblocking(listener);

#ifndef WIN32
    {
        int one = 1;
		/*
		 第二个参数为级别，第三个参数为选项名。
         SO_REUSEADDR: 允许重用本地地址  
         */
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    }
#endif

    if (bind(listener, (struct sockaddr*)&sin, sizeof(sin)) < 0) {
        perror("bind");
        return;
    }

	/*
     第二个参数: The backlog argument defines the maximum length to which the queue 
     of pending connections for sockfd  may  grow.
     */
    if (listen(listener, 16)<0) {
        perror("listen");
        return;
    }

	/*
     For each file descriptor that you wish to monitor, you must create an event 
	 structure with event_new(). (You may also declare an event structure and call 
     event_assign() to initialize the members of the structure.) To enable notification, 
     you add the structure to the list of monitored events by calling event_add(). The  
     event structure must remain allocated as long as it is active, so it should generally 
     be allocated on the heap.
     */
	/*
     The EV_PERSIST flag can also be passed in the events argument: it makes event_add() 
     persistent until event_del() is called.
     */
    listener_event = event_new(base, listener, EV_READ|EV_PERSIST, do_accept, (void*)base);
    /*XXX check it */
    event_add(listener_event, NULL);

	/*
     Finally, you call event_base_dispatch() to loop and dispatch events. You can also use 
     event_base_loop() for more fine-grained control.

	 Currently, only one thread can be dispatching a given event_base at a time. If you want 
     to run events in multiple threads at once, you can either have a single event_base whose 
     events add work to a work queue, or you can create multiple event_base objects.
     */
    event_base_dispatch(base);
}

int
main(int c, char **v)
{
    setvbuf(stdout, NULL, _IONBF, 0);

    run();

    printf("The End.");
    return 0;
}
