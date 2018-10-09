CROSS_COMPILE	?=mipsel-linux-
ARCH		?=mips
KERNEL_DIR	?=/home/eric/Works/jz4775/c340A/kernel

CC		:= $(CROSS_COMPILE)gcc
KERNEL_INCLUDE	:= -I$(KERNEL_DIR)/include -I$(KERNEL_DIR)/arch/$(ARCH)/include -I$(KERNEL_DIR)/arch/$(ARCH)/include/uapi -I$(KERNEL_DIR)/include/uapi
CFLAGS		:= -W -Wall -g $(KERNEL_INCLUDE)
LDFLAGS		:= -g

all: uvc-gadget

uvc-gadget: uvc-gadget.o
	$(CC) $(LDFLAGS) -o $@ $^ 

clean:
	rm -f *.o
	rm -f uvc-gadget
