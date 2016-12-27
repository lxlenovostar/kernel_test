#include "libevent_api.h"
#include "lx_netlink.h"
#include "lx_sock.h"
#include "unp.h"

void 
event_handler(evutil_socket_t fd, short event, void *arg)
{
  	if (event & EV_READ) {
		printf("callback start\n");
		struct bufferevent *send_bev = (struct bufferevent *)arg;

		rece_from_kernel();
       
		int n = strlen("send your message every 10s."); 	
		buffer_libnl_libevent[n] = '\0';
       	printf("fd=%u, read line: %s\n", fd, buffer_libnl_libevent);
       	bufferevent_write(send_bev, buffer_libnl_libevent, n);
		memset(buffer_libnl_libevent, '\0', 64);
  	}
}

void 
eventcb(struct bufferevent *bev, short events, void *ptr)
{
    if (events & BEV_EVENT_CONNECTED) {
         /* 
          We're connected to 127.0.0.1:9877. Ordinarily we'd do
          something here, like start reading or writing. 
          */
  		struct event *netlink_event;
		struct event_base *base = (struct event_base *)ptr;
  		netlink_event = event_new(base, netlink_fd, EV_READ | EV_ET | EV_PERSIST, event_handler, bev);
  		event_add(netlink_event, NULL);
    } else {
        printf("connection error\n");
    	bufferevent_free(bev);
    }
}

void 
run(void *dst_address)
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
	Inet_pton(AF_INET, dst_address, &servaddr.sin_addr.s_addr);
	//servaddr.sin_addr.s_addr = htonl(0x7f000001); /* 127.0.0.1 */

	/*
     BEV_OPT_CLOSE_ON_FREE: When the bufferevent is freed, close the underlying transport. 
	 This will close an underlying socket, free an underlying bufferevent, etc.
     */
	bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);

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
