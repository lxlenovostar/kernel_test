obj-m := jpro1.o jpro2.o

all:
	make -C /usr/src/kernels/2.6.18-53.5AXS3custom-x86_64 M=`pwd` V=1

.PHONY: ins rm rei clean
ins:
	insmod jpro1.ko

rm:
	rmmod jpro1.ko

rei:
	rmmod jpro1.ko
	insmod jpro1.ko

clean:
	rm -f *.ko *.mod.ko *.o *.mod.c Module.symvers
