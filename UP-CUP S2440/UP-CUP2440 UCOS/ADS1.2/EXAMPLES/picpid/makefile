CC=armcc
AS=armasm
LD=armlink
FE=fromelf

romimage: kernel.bin app1.bin app2.bin nullapp.bin
	cat kernel.bin app1.bin app2.bin nullapp.bin > romimage

clean:
	rm -f *.o *.bin *.axf *.map romimage

kernel.bin: kernel.o
	-rm -f kernel.sym
	$(LD) -o kernel.axf -ro-base 0x8000 -rw-base 0x40000 kernel.o -symdefs kernel.sym -map -symbols -list kernel.map
	$(FE) kernel.axf -bin -o kernel.bin

kernel.o: kernel.c kernel.h
	$(CC) -c -g -O1 kernel.c

apphdr.o: apphdr.s
	$(AS) -g -apcs /ropi apphdr.s

kveneer.o: kveneer.s
	$(AS) -g -apcs /ropi kveneer.s

app1.o: app1.c kernel.h
	$(CC) -c -g -O1 -apcs /ropi/rwpi app1.c

app2.o: app2.c kernel.h
	$(CC) -c -g -O1 -apcs /ropi/rwpi app2.c

app1.bin: apphdr.o kveneer.o app1.o
	$(LD) -o app1.axf -ropi -rwpi -rw-base 0 -map -symbols -list app1.map kernel.sym kveneer.o app1.o apphdr.o
	$(FE) app1.axf -bin -o app1.bin

app2.bin: apphdr.o kveneer.o app2.o
	$(LD) -o app2.axf -ropi -rwpi -rw-base 0 -map -symbols -list app1.map kernel.sym kveneer.o app2.o apphdr.o
	$(FE) app2.axf -bin -o app2.bin

nullapp.o: nullapp.c
	$(CC) -c -g -O1 nullapp.c

nullapp.bin: nullapp.o
	$(LD) -o nullapp.axf nullapp.o
	$(FE) nullapp.axf -bin -o nullapp.bin
