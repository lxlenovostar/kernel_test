#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>

#include <event2/event.h>
#include <event2/bufferevent.h>

#define LISTEN_PORT 9999
#define LISTEN_BACKLOG 32

void do_accept(evutil_socket_t listener, short event, void *arg);
void read_cb(struct bufferevent *bev, void *arg);
void error_cb(struct bufferevent *bev, short event, void *arg);
void write_cb(struct bufferevent *bev, void *arg);

int main(int argc, char *argv[])
{
    int ret;
    evutil_socket_t listener;
    listener = socket(AF_INET, SOCK_STREAM, 0);
    assert(listener > 0);

	/*
     Do platform-specific operations to make a listener socket reusable.
	 Specifically, we want to make sure that another program will be able to bind this 
     address right after we've closed the listener.
     */
    /* 防止绑定一个现有连接上的端口，从而出现失败的现象。*/
    evutil_make_listen_socket_reuseable(listener);

    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = 0;
    sin.sin_port = htons(LISTEN_PORT);

    if (bind(listener, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
        perror("bind");
        return 1;
    }

    if (listen(listener, LISTEN_BACKLOG) < 0) {
        perror("listen");
        return 1;
    }

    printf ("Listening...\n");

	/* 非阻塞的端口 */
    evutil_make_socket_nonblocking(listener);

    struct event_base *base = event_base_new();
    assert(base != NULL);
    struct event *listen_event;

	/* 参数：event_base, 监听的fd，事件类型及属性，绑定的回调函数，给回调函数的参数 */
	/*
      (a) EV_TIMEOUT: 超时
      (b) EV_READ: 只要网络缓冲中还有数据，回调函数就会被触发
      (c) EV_WRITE: 只要塞给网络缓冲的数据被写完，回调函数就会被触发
      (d) EV_PERSIST: 不指定这个属性的话，回调函数被触发后事件会被删除
      (e) EV_ET: Edge-Trigger边缘触发，参考EPOLL_ET
     */
    listen_event = event_new(base, listener, EV_READ|EV_PERSIST, do_accept, (void*)base);

	/* 参数：event，超时时间(struct timeval *类型的，NULL表示无超时设置) */
    event_add(listen_event, NULL);

	/* 启动事件循环 */
    event_base_dispatch(base);

    printf("The End.");
    return 0;
}

void do_accept(evutil_socket_t listener, short event, void *arg)
{
    struct event_base *base = (struct event_base *)arg;
    evutil_socket_t fd;
    struct sockaddr_in sin;
    socklen_t slen = sizeof(sin);
    fd = accept(listener, (struct sockaddr *)&sin, &slen);
    if (fd < 0) {
        perror("accept");
        return;
    } else if (fd > FD_SETSIZE) { 
        perror("fd > FD_SETSIZE\n");
		close(fd);
        return;
    } else {
    	printf("ACCEPT: fd = %u\n", fd);

		/* 创建一个struct bufferevent *bev，关联该sockfd，托管给event_base */
    	struct bufferevent *bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
    	bufferevent_setcb(bev, read_cb, NULL, error_cb, arg);
    	bufferevent_enable(bev, EV_READ|EV_WRITE|EV_PERSIST);
	}
}

void read_cb(struct bufferevent *bev, void *arg)
{
#define MAX_LINE    256
    char line[MAX_LINE+1];
    int n;
    evutil_socket_t fd = bufferevent_getfd(bev);

    while (n = bufferevent_read(bev, line, MAX_LINE), n > 0) {
        line[n] = '\0';
        printf("fd=%u, read line: %s\n", fd, line);

        bufferevent_write(bev, line, n);
    }
}

void write_cb(struct bufferevent *bev, void *arg) {}

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
