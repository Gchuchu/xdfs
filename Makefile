# make file for file system kernel module

obj-m:= xdfs.o
CURRENT_PATH:=$(pwd)
LINUX_KERNEL_VERSION:=$(uname -r)
LINUX_KERNEL_PATH:=/lib/modules/$(LINUX_KERNEL_VERSION)/build
all:
    $(MAKE) -C $(LINUX_KERNEL_PATH) M=$(CURRENT_PATH) modules
clean:
    $(MAKE) -C $(LINUX_KERNEL_PATH) M=$(CURRENT_PATH) clean