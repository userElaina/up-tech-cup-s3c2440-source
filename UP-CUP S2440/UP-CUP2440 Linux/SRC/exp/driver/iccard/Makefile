
CC=arm-linux-gcc
EXEC = ./iccard-test
OBJS = iccard-test.o 

all: $(EXEC)
$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) 
install:
	$(EXP_INSTALL) $(EXEC) $(INSTALL_DIR)


clean:
	-rm -f $(EXEC) *.elf *.gdb *.o
