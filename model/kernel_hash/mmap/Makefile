obj-m	:= kernel_hash.o 

KDIR:=/lib/modules/`uname -r`/build
PWD:=$(shell pwd)

default:
	$(MAKE) -C $(KDIR) M=$(PWD) modules
