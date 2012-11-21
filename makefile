CC=avr-gcc
CFLAGS=-mmcu=atmega16 -O2
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
	$(CC) $(CFLAGS) -o $(TARGET).o $(SOURCES)

flash: 
	avrdude -p $(PART) -c $(PROG) -P $(USBPORT) -U flash:w:$(TARGET).hex

clean:
	rm -f $(OBJECTS) $(TARGET).hex

