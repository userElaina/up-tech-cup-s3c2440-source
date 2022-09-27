CC= arm-linux-gcc
#CC= gcc
#LDFLAG= -g -I$(KERNELDIR)/include
EXTRA_LIBS += -lpthread

all: v4lcap

v4lcap: 
	$(CC) main.c drv-v4l.c grab-ng.c struct-dump.c struct-v4l.c $(LDFLAG) -o v4lcap $(EXTRA_LIBS)

clean:
	rm -f *.o a.out v4lcap *.gdb
