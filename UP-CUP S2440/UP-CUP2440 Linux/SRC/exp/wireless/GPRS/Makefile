TOPDIR = ../
include ./Rules.mak

EXTRA_LIBS += -lpthread

EXEC =  gprs
OBJS =  main.o tty.o gprs.o keyshell.o $(TOPDIR)/keyboard/keyboard.o $(TOPDIR)/keyboard/get_key.o

all: $(EXEC)


$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS)  $(EXTRA_LIBS)

install:
	$(EXP_INSTALL) $(EXEC) $(INSTALL_DIR)

clean:
	-rm -f $(EXEC) *.elf *.elf2flt *.gdb *.o
	
