tcpcli : 
	cd lib; make 
	cd src; make 
	gcc -D_REENTRANT -Wall -g -lpthread -levent -I /usr/local/include/libnl3 -lnl-3 lib/*.o src/*.o -o tcpserver

.PHONY : clean
clean:
	rm tcpserver; 
	cd lib; make clean
	cd src; make clean  
