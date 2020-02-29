ifeq ($(KERNELRELEASE),)

KERNELDIR = /usr/src/linux-headers-4.15.0-88-generic
PWD := $(shell pwd)

modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

clean:
	rm -rf *.o *.ko .mod.o *.mod.c *.symvers

else
	obj-m := dev_mem.o drv_mem.o
endif
