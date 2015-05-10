#include "variables.c"
#define DS_PORT		PORTA
#define DS_DDR		DDRA
#define DS_PIN		PINA
#define DS_CTL		5
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <string.h>
#include "strings.c"
#include "lcd.c"
#include "ds1820.c"

volatile uint8_t rom[3][8];
uint8_t dstemp[9];
volatile unsigned char pa = 0;
volatile unsigned char intc = 0;
volatile uartbuffer[32];
volatile char* str;
volatile char str_b[20];

#include "uart.c"

char EEMEM em = 0;
const char dsmessage[] PROGMEM = "Searching roms..";
const char romfound[] PROGMEM = "Found: ";


ISR (TIMER1_COMPA_vect) {
	unsigned char p;
	pa = (~pa) & 0x0F;
	p = PORTA & (0xF0);
	PORTA = pa | p;
	intc = 1;
}

void main(void) {
	cli();
	unsigned char tmsg[4] = {'T','E','M','P'};
	unsigned char i,h,j = 0;
	unsigned int mantiss = 0;
	uint16_t lcdbcd;
	uint8_t roms = 0;
	unsigned long temp;
        char sind,sbyte;

	
	TIMSK |= (1<<OCIE1A);
	TCCR1B = (1<<CS12)|(0<<CS11)|(1<<CS10)|(0<<WGM13)|(1<<WGM12);
	TCCR1A = 0;
	OCR1AH = 0x7F;
	OCR1AL = 0xFF;
	TCNT1H = 0;
	TCNT1L = 0;
	DDRA |= 0x0F;
	PORTA &= 0xF0;
	sei();
	
	memset(&uartbuffer, 0, 32);
	uart_strcpy("Rom found: ");
	uart_init();
	lcd_init();
	i=0;
	i = eeprom_read_byte(&em);
	lcd_write_str_p(dsmessage);
	uart_send_str((char *)&uartbuffer);
	uart_txc_wait();
	
	if (ds_init()) {
		roms = ds_search_rom((uint8_t *)&rom);
		lcd_second_line();
		lcd_write_str_p(romfound);
		memset(&uartbuffer, 0, 32);
		uart_print_int((unsigned int)roms);
		uart_strncat("\r\n", 2);
		uart_send_str((char *)&uartbuffer);
		uart_txc_wait();
	
		for (i=0; i<roms; i++) {
		    memset(&uartbuffer, 0, 32);
		    for (h=0; h<8; h++) {
		    	temp = hex2bcd(rom[i][h]);
			j = temp>>8;
			uart_strncat(&j,1);	
			j = temp;
			uart_strncat(&j,1);	
		    }
		    uart_strncat("\r\n", 2);
		    uart_send_str((char *)&uartbuffer);
		    uart_txc_wait();
		}

		lcd_print_int(roms);
	}
	_delay_ms(2000);

	while (1) {
		if (intc) {
		    h = 0;
		    ds_init();
		    ds_write_byte(0xcc);
                    ds_write_byte(0x44);
                    _delay_ms(1000);
		    lcd_clr();
		    sind = 0;
		    memset(&uartbuffer, 0, 32);
		    uart_strncat("> ", 2);

		    do {
			ds_init();
			ds_write_byte(0x55);
			for (i=0; i<8; i++) {
			    ds_write_byte(rom[h][i]);
			}
			ds_write_byte(0xbe);
			ds_read_scratch(dstemp);
			if (!dstemp[1]) {
				i = dstemp[0]>>1;
				if (dstemp[6] & (1<<4)) {
					// flag 5 of dstemp[6] means that we need to decrease  
					// the temperature variable(i)
					// but if the value is zero, value will be 255, but we want -1
					// so we set munis flag of dstemp[1] and just increase i
					if (i==0) {
						i++;
						dstemp[1] = 1;
					} else {
						i--;
					}
				}
                	} else {
				i = (uint8_t)(~dstemp[0])>>1;
				if (dstemp[6] & (1<<4)) {i++;}
                	}
			//temp = hex2ascii((uint8_t)i);
			temp = (uint8_t)i;
			if (!dstemp[1]) {
				i = ~dstemp[6] & 0x0F;	
			} else {
				i = dstemp[6];
			}
			mantiss = 0;
			if (i & (1<<3)) { mantiss += 500; }
			if (i & (1<<2)) { mantiss += 250; }
			if (i & (1<<1)) { mantiss += 125; }
			if (i & (1)) { mantiss += 62; }

			if (h>1) { lcd_second_line(); }
			if (dstemp[1]) {
				lcd_write('-', 1,0);
				uart_strncat("-",1);
			}

			lcd_print_int(temp);
			uart_print_int(temp);
			
                	lcd_write('.', 1,0);
			uart_strncat(".",1);
			temp = hex2ascii(mantiss);
			sbyte = (uint8_t)(temp>>16);
			lcd_write(sbyte,1,0);
                	lcd_write(' ', 1,0);
			uart_strncat(&sbyte, 1);
			h++;
			if (h<roms) {
			    uart_strncat(",",1);
			}
			intc = 0;
		    } while (h<roms);
		    uart_strncat("\r\n",2);
		    uart_send_str((char *)&uartbuffer);
		}
	}

}
