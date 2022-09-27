###############################################################################
# Makefile for the project uptech_coor
###############################################################################

## General Flags
PROJECT = uptech_coor
MCU = atmega128
TARGET = uptech_coor.elf
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
OBJECTS = zPHYCC2420.o console.o coord_test.o MSPI.o NVM.o Tick.o zAPL.o zAPS.o zMAC.o zNWK.o 

## Build
all: $(TARGET) uptech_coor.hex uptech_coor.eep

## Compile
zPHYCC2420.o: ../../../zPHYCC2420.c
	$(CC) $(INCLUDES) $(CFLAGS) -c   $< -o $@

console.o: ../../../console.c
	$(CC) $(INCLUDES) $(CFLAGS) -c   $< -o $@

coord_test.o: ../../../coord_test.c
	$(CC) $(INCLUDES) $(CFLAGS) -c   $< -o $@

MSPI.o: ../../../MSPI.c
	$(CC) $(INCLUDES) $(CFLAGS) -c   $< -o $@

NVM.o: ../../../NVM.c
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
	-rm -rf $(OBJECTS) uptech_coor.elf dep/ uptech_coor.hex uptech_coor.eep

## Other dependencies
-include $(shell mkdir dep 2>/dev/null) $(wildcard dep/*)

