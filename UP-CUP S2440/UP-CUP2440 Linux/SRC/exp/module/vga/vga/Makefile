setvga : setvga.o
	arm-linux-gcc -o setvga setvga.o 

setvga.o : classical-vga.c
	arm-linux-gcc -c classical-vga.c -o setvga.o

clean :
	rm *.o setvga 
