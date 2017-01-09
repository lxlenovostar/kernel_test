tcpcli_old : 
	cd lib; make 
	cd src; make 
	#gcc -D_REENTRANT -Wall -g -lpthread -levent -I /usr/local/include/libnl3 -lnl-3 lib/*.o src/*.o -o tcpcli
	gcc -D_REENTRANT -Wall -g -lpthread -levent lib/*.o src/*.o -o tcpcli_old

.PHONY : clean
clean:
	rm tcpcli_old; 
	cd lib; make clean
	cd src; make clean  
