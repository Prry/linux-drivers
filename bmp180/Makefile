ifeq ($(KERNELRELEASE),)

#KERNELDIR = /usr/src/linux-headers-4.15.0-72-generic
#KERNELDIR = /opt/rk3399/linux/linux-kernel
KERNELDIR = /opt/rk3399/linux-sdk/linux-sdk/kernel
PWD := $(shell pwd)

modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

clean:
	rm -rf *.o *.ko .mod.o *.mod.c *.symvers

else
	obj-m := bmp180.o
endif