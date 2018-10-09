CROSS_COMPILE	?=mips-linux-gnu-
ARCH		?=mips
#KERNEL_DIR	?=/home/eric/Works/x1000/fa210/kernel
KERNEL_DIR	?=/home/eric/Works/jz4775/c340A/kernel

CC		:= $(CROSS_COMPILE)gcc
KERNEL_INCLUDE	:= -I$(KERNEL_DIR)/include -I$(KERNEL_DIR)/arch/$(ARCH)/include/ \
			#-I$(KERNEL_DIR)/arch/$(ARCH)/include/asm/mach-generic  \
			#-I$(KERNEL_DIR)/arch/$(ARCH)/include/asm	\
			#-I$(KERNEL_DIR)/arch/$(ARCH)/include/asm/generated
CFLAGS		:= -W -Wall -g $(KERNEL_INCLUDE)
LDFLAGS		:= -g

all: uvc-gadget

uvc-gadget: uvc-gadget.o video.o write_bmp_func.o TcYuvX.o
	$(CC) $(LDFLAGS) -o $@ $^ 

clean:
	rm -f *.o
	rm -f uvc-gadget
