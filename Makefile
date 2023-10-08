# make file for file system kernel module

CFLAGS := -Wall –g –std=c99
obj-m:= xdfs.o
CURRENT_PATH:=$(shell pwd)
LINUX_KERNEL_VERSION:=$(shell uname -r)
LINUX_KERNEL_PATH:=/lib/modules/$(LINUX_KERNEL_VERSION)/build
all:
	$(MAKE) $(CFLAGS) -C $(LINUX_KERNEL_PATH) M=$(CURRENT_PATH) modules
clean:
	$(MAKE) $(CFLAGS) -C $(LINUX_KERNEL_PATH) M=$(CURRENT_PATH) clean
