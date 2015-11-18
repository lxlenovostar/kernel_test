APP_NAME	:= wstest

#All the EXTRA_ variables apply only to the kbuild makefile
#where they are assigned. The EXTRA_ variables apply to all
#commands executed in the kbuild makefile.
#$(EXTRA_CFLAGS) specifies options for compiling C files with $(CC).
EXTRA_CFLAGS +=  -I$(LDDINC) -g 

#The check for KERNELRELEASE is used to separate the two parts
#of the makefile. In the example, kbuild will only see the two
#assignments, whereas "make" will see everything except these
#two assignments. This is due to two passes made on the file:
#the first pass is by the "make" instance run on the command
#line; the second pass is by the kbuild system, which is
#initiated by the parameterized "make" in the default target.
ifneq ($(KERNELRELEASE),)
obj-m	:= $(APP_NAME).o
$(APP_NAME)-objs := $(WS_OBJS)

else

#find all source file.
#WS_CFILES   = $(shell find -name "*.c")
WS_CFILES   = $(shell find -maxdepth 1 -name "*.c")
#get the object file.
export WS_OBJS = $(WS_CFILES:.c=.o)

#if $KERNELDIR don't assigned, ?= will assign it.
KERNELDIR ?= /lib/modules/$(shell uname -r)/build
#KERNELDIR ?= /root/usr/src/kernels/2.6.32-358.6.1.ws5.b.5.2.8

PWD := $(shell pwd)

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) LDDINC=$(PWD) modules
endif

.PHONY : clean tags
clean:
	-rm -rf *.o *~ core .depend .*.cmd .*.d *.mod.c .tmp_versions modules.order Module.symvers Module.markers *.unsigned
	-find -name "*.o" | xargs rm -f

distclean: clean
	-rm -f *.ko
	-rm -f cscope.* 
	-rm tags	

tags:
	cscope -Rbq
	ctags -R .
