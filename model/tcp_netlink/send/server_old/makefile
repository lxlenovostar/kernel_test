tcpcli : 
	cd lib; make 
	cd src; make 
	#gcc -D_REENTANT -Wall -g -lpthread -lrt -lm -I/usr/local/include -I/usr/local/include/libnl3 -L/usr/local/lib -o tcpserver lib/*.o src/*.o /usr/local/lib/libnl-3.a /usr/local/lib/libevent.a 
	#gcc -D_REENTRANT -Wall -g -lpthread -levent -I /usr/local/include/libnl3 -lnl-3 lib/*.o src/*.o -o tcpserver
	gcc -D_REENTRANT -Wall -g -lpthread -levent lib/*.o src/*.o -o tcpserver_old

.PHONY : clean
clean:
	rm tcpserver_old; 
	cd lib; make clean
	cd src; make clean  
