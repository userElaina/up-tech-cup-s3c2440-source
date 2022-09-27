CC = iccavr
CFLAGS =  -IC:\PROGRA~1\icc\include -e -DATMega8  -l -g -Mavr_enhanced_small -Wa-W 
ASFLAGS = $(CFLAGS)  -Wa-g
LFLAGS =  -LC:\PROGRA~1\icc\lib -g -Wl-W -bfunc_lit:0x26.0x2000 -dram_end:0x45f -bdata:0x60.0x45f -dhwstk_size:16 -beeprom:1.512 -fihx_coff -S2
FILES = 18b20.o ADC.o Sensor.o SensorEXB.o SPI.o TimeDelay.o Timer.o UART.o 

SensorEXBIcc:	$(FILES)
	$(CC) -o SensorEXBIcc $(LFLAGS) @SensorEXBIcc.lk  
18b20.o: C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc/Generic.h C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc/SPI.h C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc/UART.h C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc/ADC.h C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc/TimeDelay.h\
 C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc/18b20.h C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc/Sensor.h C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc/Timer.h C:/PROGRA~1/icc/include/iom8v.h C:/PROGRA~1/icc/include/macros.h
18b20.o:	C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc\18b20.c
	$(CC) -c $(CFLAGS) C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc\18b20.c
ADC.o: C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc/Generic.h C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc/SPI.h C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc/UART.h C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc/ADC.h C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc/TimeDelay.h\
 C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc/18b20.h C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc/Sensor.h C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc/Timer.h C:/PROGRA~1/icc/include/iom8v.h C:/PROGRA~1/icc/include/macros.h
ADC.o:	C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc\ADC.c
	$(CC) -c $(CFLAGS) C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc\ADC.c
Sensor.o: C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc/Generic.h C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc/SPI.h C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc/UART.h C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc/ADC.h C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc/TimeDelay.h\
 C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc/18b20.h C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc/Sensor.h C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc/Timer.h C:/PROGRA~1/icc/include/iom8v.h C:/PROGRA~1/icc/include/macros.h
Sensor.o:	C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc\Sensor.c
	$(CC) -c $(CFLAGS) C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc\Sensor.c
SensorEXB.o: C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc/generic.h C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc/SPI.h C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc/UART.h C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc/ADC.h C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc/TimeDelay.h\
 C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc/18b20.h C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc/Sensor.h C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc/Timer.h C:/PROGRA~1/icc/include/iom8v.h C:/PROGRA~1/icc/include/macros.h
SensorEXB.o:	C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc\SensorEXB.c
	$(CC) -c $(CFLAGS) C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc\SensorEXB.c
SPI.o: C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc/Generic.h C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc/SPI.h C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc/UART.h C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc/ADC.h C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc/TimeDelay.h\
 C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc/18b20.h C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc/Sensor.h C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc/Timer.h C:/PROGRA~1/icc/include/iom8v.h C:/PROGRA~1/icc/include/macros.h C:/PROGRA~1/icc/include/math.h
SPI.o:	C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc\SPI.c
	$(CC) -c $(CFLAGS) C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc\SPI.c
TimeDelay.o: C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc/Generic.h C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc/SPI.h C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc/UART.h C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc/ADC.h C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc/TimeDelay.h\
 C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc/18b20.h C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc/Sensor.h C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc/Timer.h C:/PROGRA~1/icc/include/iom8v.h C:/PROGRA~1/icc/include/macros.h
TimeDelay.o:	C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc\TimeDelay.c
	$(CC) -c $(CFLAGS) C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc\TimeDelay.c
Timer.o: C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc/Generic.h C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc/SPI.h C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc/UART.h C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc/ADC.h C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc/TimeDelay.h\
 C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc/18b20.h C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc/Sensor.h C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc/Timer.h C:/PROGRA~1/icc/include/iom8v.h C:/PROGRA~1/icc/include/macros.h C:/PROGRA~1/icc/include/math.h
Timer.o:	C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc\Timer.c
	$(CC) -c $(CFLAGS) C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc\Timer.c
UART.o: C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc/generic.h C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc/SPI.h C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc/UART.h C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc/ADC.h C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc/TimeDelay.h\
 C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc/18b20.h C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc/Sensor.h C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc/Timer.h C:/PROGRA~1/icc/include/iom8v.h C:/PROGRA~1/icc/include/macros.h
UART.o:	C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc\UART.c
	$(CC) -c $(CFLAGS) C:\DOCUME~1\蔡颖鹏\桌面\SensorEXBIcc\UART.c
