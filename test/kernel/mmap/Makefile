obj-m	:= my_mmap.o 

KDIR:=/lib/modules/`uname -r`/build
PWD:=$(shell pwd)

default:
	$(MAKE) -C $(KDIR) M=$(PWD) modules
