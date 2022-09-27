
include $(TOPDIR)Rules.mak

EXTRA_LIBS += -lsqlite3

EXEC =  $(INSTALL_DIR)/SQLite_test
#OBJS =  main-bak1.o 
OBJS =  main.o SQLite.o consoleshell.o tty.o

all: $(EXEC)


$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -I ./sqlite/ -L ./sqlite/ -o $@ $(OBJS)  $(EXTRA_LIBS)

install:
	$(EXP_INSTALL) $(EXEC) $(INSTALL_DIR)

clean:
	-rm -f $(EXEC) *.elf *.elf2flt *.gdb *.o