#include "libevent_api.h"
#include "lx_netlink.h"
#include "unp.h"

void stdin_read_cb(struct bufferevent *bev, void *ctx)
{
    char line[MAX_LINE];
    int n;
    evutil_socket_t fd = bufferevent_getfd(bev);
	struct bufferevent *send_bev = (struct bufferevent *)ctx;

    while (n = bufferevent_read(bev, line, MAX_LINE), n > 0) {
        line[n] = '\0';
        printf("fd=%u, read line: %s\n", fd, line);

        bufferevent_write(send_bev, line, n);
    }
}

void stdin_error_cb(struct bufferevent *bev, short event, void *arg)
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

/*
void wait_from_other()
{

    for (;;) {
        Pthread_mutex_lock(&mtx);

        while (avail == 0) {            // Wait for something to consume 
            Pthread_cond_wait(&cond, &mtx);
        }

        // At this point, 'mtx' is locked... 

        while (avail > 0) {             // Consume all available units 
            // Do something with produced unit 
            avail--;
            printf("we spend one\n");
        }

        Pthread_mutex_unlock(&mtx);
    }
}
*/

void event_handler(evutil_socket_t fd, short event, void *arg)
{
	if (event & EV_TIMEOUT) 
	{
  		printf("timeout\n");
    	exit(1);
  	} else if (event & EV_READ) {
		//debug_info();
  	}

}

void eventcb(struct bufferevent *bev, short events, void *ptr)
{
    if (events & BEV_EVENT_CONNECTED) {
         /* We're connected to 127.0.0.1:9877. Ordinarily we'd do
            something here, like start reading or writing. */
		/*
        struct bufferevent *bev;
		struct event_base *base = (struct event_base *)ptr;
        //evutil_make_socket_nonblocking(fileno(stdin));
        bev = bufferevent_socket_new(base, fileno(stdin), BEV_OPT_CLOSE_ON_FREE);
        bufferevent_setcb(bev, stdin_read_cb, NULL, stdin_error_cb, bev);
        bufferevent_setwatermark(bev, EV_READ, 0, MAX_LINE);
        bufferevent_enable(bev, EV_READ|EV_WRITE);
		*/
		//wait_from_other();

  		struct event *netlink_event;
		struct event_base *base = (struct event_base *)ptr;
  		//netlink_event = event_new(base, netlink_fd, EV_READ | EV_PERSIST, event_handler, NULL);
  		netlink_event = event_new(base, netlink_fd, EV_READ | EV_ET | EV_PERSIST, event_handler, NULL);
  		event_add(netlink_event, NULL);
    } else if (events & BEV_EVENT_ERROR) {
         /* An error occured while connecting. */
        printf("connection error\n");
    }
    bufferevent_free(bev);
}

void run(void)
{
    struct sockaddr_in servaddr;
    struct event_base *base;
	struct bufferevent *bev;

    base = event_base_new();
    if (!base)
        return; 

	memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);
	//inet_pton(AF_INET, argv[1], &servaddr.sin_addr);
	servaddr.sin_addr.s_addr = htonl(0x7f000001); /* 127.0.0.1 */

	/*
     BEV_OPT_CLOSE_ON_FREE: When the bufferevent is freed, close the underlying transport. 
	 This will close an underlying socket, free an underlying bufferevent, etc.
     */
	bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);

	/*
	 The bufferevent_setcb() function changes one or more of the callbacks of a bufferevent. 
	 The readcb, writecb, and eventcb functions are called (respectively) when enough data is 
	 read, when enough data is written, or when an event occurs.
     */
	bufferevent_setcb(bev, NULL, NULL, eventcb, (void *)base);
    bufferevent_enable(bev, EV_READ|EV_WRITE);

	if (bufferevent_socket_connect(bev, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
     	perror("connect");
        bufferevent_free(bev);
        return;
    }

	printf("begin dispatch\n");
    event_base_dispatch(base);
  	return; 
}

/*
int
main(int c, char **v)
{
    setvbuf(stdout, NULL, _IONBF, 0);

    run();

    printf("The End.");
    return 0;
}
*/
