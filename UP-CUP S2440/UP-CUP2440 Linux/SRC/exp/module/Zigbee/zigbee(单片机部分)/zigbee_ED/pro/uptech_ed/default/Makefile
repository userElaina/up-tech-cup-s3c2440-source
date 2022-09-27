###############################################################################
# Makefile for the project uptech_ed
###############################################################################

## General Flags
PROJECT = uptech_ed
MCU = atmega128
TARGET = uptech_ed.elf
CC = avr-gcc

## Options common to compile, link and assembly rules
COMMON = -mmcu=$(MCU)

## Compile options common for all C compilation units.
CFLAGS = $(COMMON)
CFLAGS += -Wall -gdwarf-2 -O0 -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
CFLAGS += -Wp,-M,-MP,-MT,$(*F).o,-MF,dep/$(@F).d 

## Assembly specific flags
ASMFLAGS = $(COMMON)
ASMFLAGS += -x assembler-with-cpp -Wa,-gdwarf2

## Linker flags
LDFLAGS = $(COMMON)
LDFLAGS += 


## Intel Hex file production flags
HEX_FLASH_FLAGS = -R .eeprom

HEX_EEPROM_FLAGS = -j .eeprom
HEX_EEPROM_FLAGS += --set-section-flags=.eeprom="alloc,load"
HEX_EEPROM_FLAGS += --change-section-lma .eeprom=0


## Objects that must be built in order to link
OBJECTS = 18b20.o console.o MSPI.o NVM.o RFD_test.o SensorProcess.o Tick.o zAPL.o zAPS.o zMAC.o zNWK.o zPHYCC2420.o 

## Build
all: $(TARGET) uptech_ed.hex uptech_ed.eep

## Compile
18b20.o: ../../../18b20.c
	$(CC) $(INCLUDES) $(CFLAGS) -c   $< -o $@

console.o: ../../../console.c
	$(CC) $(INCLUDES) $(CFLAGS) -c   $< -o $@

MSPI.o: ../../../MSPI.c
	$(CC) $(INCLUDES) $(CFLAGS) -c   $< -o $@

NVM.o: ../../../NVM.c
	$(CC) $(INCLUDES) $(CFLAGS) -c   $< -o $@

RFD_test.o: ../../../RFD_test.c
	$(CC) $(INCLUDES) $(CFLAGS) -c   $< -o $@

SensorProcess.o: ../../../SensorProcess.c
	$(CC) $(INCLUDES) $(CFLAGS) -c   $< -o $@

Tick.o: ../../../Tick.c
	$(CC) $(INCLUDES) $(CFLAGS) -c   $< -o $@

zAPL.o: ../../../zAPL.c
	$(CC) $(INCLUDES) $(CFLAGS) -c   $< -o $@

zAPS.o: ../../../zAPS.c
	$(CC) $(INCLUDES) $(CFLAGS) -c   $< -o $@

zMAC.o: ../../../zMAC.c
	$(CC) $(INCLUDES) $(CFLAGS) -c   $< -o $@

zNWK.o: ../../../zNWK.c
	$(CC) $(INCLUDES) $(CFLAGS) -c   $< -o $@

zPHYCC2420.o: ../../../zPHYCC2420.c
	$(CC) $(INCLUDES) $(CFLAGS) -c   $< -o $@

##Link
$(TARGET): $(OBJECTS)
	 $(CC) $(LDFLAGS) $(OBJECTS) $(LIBDIRS) $(LIBS) -o $(TARGET)

%.hex: $(TARGET)
	avr-objcopy -O ihex $(HEX_FLASH_FLAGS)  $< $@

%.eep: $(TARGET)
	avr-objcopy $(HEX_EEPROM_FLAGS) -O ihex $< $@

%.lss: $(TARGET)
	avr-objdump -h -S $< > $@

## Clean target
.PHONY: clean
clean:
	-rm -rf $(OBJECTS) uptech_ed.elf dep/ uptech_ed.hex uptech_ed.eep

## Other dependencies
-include $(shell mkdir dep 2>/dev/null) $(wildcard dep/*)

