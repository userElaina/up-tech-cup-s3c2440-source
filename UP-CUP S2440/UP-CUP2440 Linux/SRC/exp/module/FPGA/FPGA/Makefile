CROSS_COMPILE=arm-linux-
#KERNELDIR = /home/wbn/arm2410/kernel/my-kernel/kernel-fpga
#INCLUDEDIR = $(KERNELDIR)/include
#CFLAGS += -I/usr/local/arm/2.95.3/arm-linux/include

CC      =$(CROSS_COMPILE)gcc
AS      =$(CROSS_COMPILE)as
LD      =$(CROSS_COMPILE)ld
CPP     =$(CC) -E
AR      =$(CROSS_COMPILE)ar
NM      =$(CROSS_COMPILE)nm
STRIP   =$(CROSS_COMPILE)strip
OBJCOPY =$(CROSS_COMPILE)objcopy
OBJDUMP =$(CROSS_COMPILE)objdump

EXEC = fpga-test
all: $(EXEC)

fpga-test: fpga-test.o
	$(CC) $^ -o $@
#	$(CC) $^ -o $@ -I$(INCLUDEDIR)

clean:
	-rm -f $(EXEC) *.elf *.gdb *.o
