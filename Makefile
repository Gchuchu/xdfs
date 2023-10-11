# make file for file system kernel module

CONFIG_MODULE_SIG = n
# to solve when insmod:
# module verification failed: 
# signature and/or required key missing - tainting kernel
CFLAGS := -Wall –std=c99
obj-m:= xdfs.o
CURRENT_PATH:=$(shell pwd)
LINUX_KERNEL_VERSION:=$(shell uname -r)
LINUX_KERNEL_PATH:=/lib/modules/$(LINUX_KERNEL_VERSION)/build
all:
	$(MAKE) $(CFLAGS) -C $(LINUX_KERNEL_PATH) M=$(CURRENT_PATH) modules
clean:
	$(MAKE) $(CFLAGS) -C $(LINUX_KERNEL_PATH) M=$(CURRENT_PATH) clean
