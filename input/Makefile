ifeq ($(KERNELRELEASE),)

KERNELDIR = /usr/src/linux-headers-4.15.0-99-generic
PWD := $(shell pwd)

modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

clean:
	rm -rf *.o *.ko .mod.o *.mod.c *.symvers *.order

else
	obj-m := devinput.o
	devinput-objs = dev_input.o drv_input.o
endif
