##### Goal definitions #####
# Goal definitions are the main part of the kbuild Makefile. These lines define the files to be built.
# Example: obj-y += foo.o
# This tells kbuild that there is one object named foo.o to be built. The .o file will be built beforehand from foo.c
# the 'm' in obj-m means that foo.o shall be built as a module in kernel
# m - module
# y - yes ( built-in to kernel )
# obj-m += uio_pdrv_genirq.o

##### Detect the number of processors for parallel build #####
NPROCS:=1
OS:=$(shell uname -s)
ifeq ($(OS),Linux)
	NPROCS:=$(shell grep -c ^processor /proc/cpuinfo)
endif
ifeq ($(OS),Darwin) # Assume Mac OS X
	NPROCS:=$(shell system_profiler | awk '/Number Of CPUs/{print $4}{next;}')
endif

# CC is a variable to specify the compiler to be used. By default it would be cc, but as we are cross-compiling
# we should specify another compiler for that. Therefore, we can redefine it to take $CROSS_COMPILE variable into account which
# we will be specifying when running ''make''. Example: make CROSS_COMPILE=arm-linux-gnueabihf-
# At this point it is only necessary for compiling the userspace program
CC=$(CROSS_COMPILE)gcc

##### Targets #####
# Targets specify what will be executed during the make
all:
	$(MAKE) -j$(NPROCS) ARCH=arm -C ../../../../linux/
	$(MAKE) -j$(NPROCS) ARCH=arm -C ../../../../linux/ M=$(PWD)
	$(CC) -lpthread ZedboardOLED.c io_lib.c switch_input.c udpclient.c music_receiver.c main.c -o main # compiles the userspace program

clean:
	$(MAKE) -C ../../../../linux/ M=$(PWD) clean
