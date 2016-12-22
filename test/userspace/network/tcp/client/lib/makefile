CFLAGS = -Wall -g
objects = error.o lx_sock.o readline.o str_cli.o wrapstdio.o 

tcpcli : $(objects)

error.o : error.h
lx_sock.o : error.h
str_cli.o : lx_sock.h error.h
readline.o : lx_sock.h error.h 
wrapstdio.o : lx_sock.h error.h

.PHONY : clean
clean:
	rm $(objects) 
