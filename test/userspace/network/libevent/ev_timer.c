#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <event.h>

#define BUFSIZE 256
#define TIMEOUT_SEC 3

void read_handler(int fd, short event, void *arg)
{
  char buffer[BUFSIZE];
  ssize_t read_len;
  read_len = read(fd, buffer, BUFSIZE);
  buffer[read_len] = '\0';
  printf("%s", buffer);
}

void event_handler(int fd, short event, void *arg)
{
  if (event & EV_TIMEOUT) {
    printf("timeout\n");
    exit(1);
  } else if (event & EV_READ) {
    read_handler(fd, event, arg);
  }
}

int main(int argc, const char* argv[])
{
  struct event_base *ev_base;
  struct event *ev;
  struct timeval tv;

  tv.tv_sec = TIMEOUT_SEC;
  tv.tv_usec = 0;

  ev_base = event_base_new();
  ev = event_new(ev_base, fileno(stdin), EV_TIMEOUT | EV_READ | EV_PERSIST, event_handler, NULL);
  event_add(ev, &tv);
  event_base_dispatch(ev_base);

  event_free(ev);
  event_base_free(ev_base);

  return 0;
}
