CFLAGS = -Wall -g -levent -lpthread $(INCLUDES) -lnl-3
INCLUDES = -I /usr/local/include/libnl3
objects = error.o lx_sock.o readline.o str_cli.o wrapstdio.o lx_netlink.o libevent_api.o

tcpcli : $(objects)

error.o : error.h
lx_sock.o : error.h
str_cli.o : lx_sock.h error.h
readline.o : lx_sock.h error.h 
wrapstdio.o : lx_sock.h error.h
lx_netlink.o : lx_netlink.h
libevent_api.o : lx_sock.h libevent_api.h

.PHONY : clean
clean:
	rm $(objects) 
