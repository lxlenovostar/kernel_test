CFLAGS = -Wall -g -lpthread 
objects = error.o lx_netlink.o libevent_api.o

tcpcli : $(objects)

error.o : error.h
lx_netlink.o : lx_netlink.h
libevent_api.o : libevent_api.h

.PHONY : clean
clean:
	rm $(objects) 
