CC = arm-linux-gcc
EXTRA_LIBS += -lpthread

EXEC =  fingermap
#OBJS =  main-bak1.o 
OBJS =  main.o tty.o fingermap.o  consoleshell.o finger_process.o

all: $(EXEC)


$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS)  $(EXTRA_LIBS)

install:
	$(EXP_INSTALL) $(EXEC) $(INSTALL_DIR)

clean:
	-rm -f $(EXEC) *.elf *.elf2flt *.gdb *.o
