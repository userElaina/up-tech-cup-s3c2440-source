#TOPDIR = ./
#include $(TOPDIR)Rules.mak

EXTRA_LIBS += -lpthread
CC=arm-linux-gcc
EXEC =  ./gps
OBJS =  main.o gps.o

all: $(EXEC)


$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS)  $(EXTRA_LIBS)

install:
	$(EXP_INSTALL) $(EXEC) $(INSTALL_DIR)

clean:
	-rm -f $(EXEC) *.elf *.elf2flt *.gdb *.o
