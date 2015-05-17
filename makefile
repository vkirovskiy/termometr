CC=avr-gcc
CFLAGS=-g -mmcu=atmega16
OBJ2HEX=avr-objcopy
TARGET=termometr
PART=m16
PROG=avr109
USBPORT=/dev/ttyUSB0
SOURCES=termometr.c
OBJECTS=$(SOURCES:%.c=%.o)

$(TARGET).hex : $(TARGET).o
	$(OBJ2HEX) -R .eeprom -O ihex $(TARGET).o $(TARGET).hex

$(TARGET).o : $(SOURCES)
	$(CC) $(CFLAGS) -Os -o $(TARGET).o $(SOURCES)

asm:
	$(CC) $(CFLAGS) -S -o $(TARGET).s $(SOURCES)

elf:
	$(CC) $(CFLAGS) -Os -c $(SOURCES)
	$(CC) $(CFLAGS) -o $(TARGET).elf $(TARGET).o

flash: 
	avrdude -p $(PART) -c $(PROG) -P $(USBPORT) -U flash:w:$(TARGET).hex

clean:
	rm -f $(OBJECTS) $(TARGET).hex $(TARGET).elf

