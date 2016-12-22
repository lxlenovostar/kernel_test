#CFLAGS = -Wall -g  -I/usr/local/include/libnl3/	-L/usr/local/lib -lnl-3
CFLAGS = -Wall -g  $(INCLUDES) -lnl-3
INCLUDES = -I /usr/local/include/libnl3
objects = userspace

tcpser : $(objects)

.PHONY : clean
clean:
	rm $(objects) 
