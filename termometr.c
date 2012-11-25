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
#include "lcd.c"

volatile unsigned char rom[3][8];
volatile unsigned char dstemp[9];
volatile unsigned char pa = 0;
volatile unsigned char intc = 0;
char EEMEM em = 0;
char dsmessage[] PROGMEM = "SENSORS ";

#define ds_pin_up()	(DS_PORT |= (1<<DS_CTL))
#define ds_pin_down()	(DS_PORT &= ~(1<<DS_CTL))

void ds_port_in(void) {
	DS_DDR &= ~(1<<DS_CTL); 
	DS_PORT &= ~(1<<DS_CTL);
}

void ds_port_out(void) {
	DS_DDR |= (1<<DS_CTL); 
	DS_PORT |=(1<<DS_CTL);
}

unsigned long hex2ascii(unsigned int n) {
	uint8_t t1,t2,t3;
	t1 = '0';
	t2 = '0';
	t3 = '0';
	while (n>=100) {
		t1++;
		n-=100;
	}
	while (n>=10) {
		t2++;
		n-=10;
	}
	t3 += (n & 0xFF);
	return (((unsigned long)t1)<<16) | (((unsigned long)t2)<<8) | (unsigned long)t3;
}

uint16_t hex2bcd(uint8_t n) {
	uint8_t bcd[2];
	bcd[0] = n & 0x0F;
	bcd[1] = (n & 0xF0)>>4;
	if (bcd[0]<10) {
		bcd[0]+='0';
	} else {
		bcd[0] = 'A' + (bcd[0]-10);
	}
	if (bcd[1]<10) {
                bcd[1]+='0';
        } else {
                bcd[1] = 'A' + (bcd[1]-10);
        }

	return (bcd[0]<<8) | bcd[1];
				
}

unsigned char ds_init() {
	cli();
	ds_port_out();
	_delay_us(5);
	ds_pin_down();
	_delay_us(485);
	ds_pin_up();
	ds_port_in();
	_delay_us(60);
	if (!(DS_PIN & (1<<DS_CTL))) {
		_delay_us(480);
		//sei();
		return 1;
	}
	_delay_us(480);
	sei();
	return 0;
	
}

uint8_t ds_read_bit() {
	uint8_t bitflag;
	ds_port_out();
        _delay_us(5);
        ds_pin_down();
        _delay_us(2);
        ds_port_in();
        _delay_us(15);
        if (DS_PIN & (1<<DS_CTL)) {
        	bitflag = 1;
        } else {
        	bitflag = 0;
        }
	_delay_us(15+30);
	return bitflag;
}

void ds_write_bit(uint8_t n) {
	cli();
	ds_port_out();
        _delay_us(5);
        ds_pin_down();
        if (n) {
            _delay_us(10);
            ds_port_in();
            _delay_us(15+30+5);
        } else {
            _delay_us(62);
            ds_port_in();
        }
	sei();
}

unsigned char ds_read_byte() {
	unsigned char dsbyte,cnt = 0;
	//cli();
	do {
		if (ds_read_bit()) {
			dsbyte |= (1<<7);
		} else {
			dsbyte &= ~(1<<7);
		}
		if (cnt<7) {
			dsbyte = dsbyte>>1;
		}
		cnt++;
	} while (cnt<8);
	//sei();

	return dsbyte;
}

void ds_read_scratch() {
	unsigned char i;
	for (i=0; i<9; i++) {
		dstemp[i] = ds_read_byte();
	}
}

void ds_write_byte(unsigned char n) {
	unsigned char cnt = 0;
	do {
	    ds_write_bit(n & (1<<cnt));
	    cnt++;
	} while (cnt<8);
}
void ds_write_data(uint8_t *n, char m) {
//void ds_write_data(unsigned char *n, unsigned char *m, char g) {
	unsigned char cnt,b;
	do {
	    cnt=0;
	    do {
		b = *n & (1<<cnt);
		ds_write_bit(b>1);
		cnt++;
	    } while (cnt<8);
	    n++;
	} while (m--);
	
}

unsigned char ds_search_rom() {
	// crasy void. Dont's try to understand this, just look to "iButton search rom" block-scheme.
	uint8_t romind,rombyte,rombit,rombit_index = 0;
	uint8_t dsbyte,ndsbyte,cnt = 0;
	char last_zero,last_disc;
	last_disc = -1;
	romind=0;
	do {
	    ds_init();
            ds_write_byte(0xf0);
	    last_zero = -1;
	    rombit_index = 0;
	    cnt = 0;
	    rombyte=0;
	    do {
		dsbyte = ds_read_bit();
		ndsbyte = ds_read_bit();
		if (dsbyte!=ndsbyte) {
		    rombit = dsbyte;
		} else {
		    rombit_index = (rombyte<<3)+cnt;
		    if (rombit_index==last_disc) {
			rombit = 1;
		    } else if (rombit_index>last_disc) {
			rombit = 0;
		    } else {
			rombit = rom[romind-1][rombyte] & (1<<cnt) > 0;
		    }
		    if (!rombit) { last_zero = rombit_index; }
		}
		if (rombit) {
                    rom[romind][rombyte] |= (1<<cnt);
                } else { 
                    rom[romind][rombyte] &= ~(1<<cnt);
                }
		ds_write_bit(rombit);
		if (cnt<7) {
                    cnt++;
                } else {
                    rombyte++;
                    cnt=0;
                }
	    } while (rombyte<8);
	    last_disc = last_zero;		
	    romind++;
	} while (last_zero>-1);
	return romind;
}

void lcd_write_str_p(char *string) {
	while (pgm_read_byte(string)!='\0') {
	    lcd_write(pgm_read_byte(string), 1,0 );
	    string++;    
	}
}

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
	unsigned char i,h = 0;
	unsigned int mantiss = 0;
	uint16_t lcdbcd;
	uint8_t roms = 0;
	unsigned long temp;
	
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
	lcd_init();
	i=0;
	i = eeprom_read_byte(&em);
	lcd_write_str_p(dsmessage);
	lcd_write(' ',1,0);
	lcd_write('0'+i,1,0);
	
	if (ds_init()) {
		roms = ds_search_rom();
		eeprom_busy_wait();
		eeprom_write_byte(&em, roms);	
		temp = hex2ascii(roms);
                lcd_write((uint8_t)(temp>>16),1,0);
                lcd_write((uint8_t)(temp>>8),1,0);
                lcd_write((uint8_t)temp & 0xFF,1,0);
	}
	_delay_ms(2000);

	while (1) {
		if (intc) {
		    h = 0;
		    ds_init();
		    ds_write_byte(0xcc);
                    ds_write_byte(0x44);
                    _delay_ms(1000);
		    lcd_write(0b00000001,0,0);
		    do {
			ds_init();
			ds_write_byte(0x55);
			for (i=0; i<8; i++) {
			    ds_write_byte(rom[h][i]);
			}
			ds_write_byte(0xbe);
			ds_read_scratch();
			if (!dstemp[1]) {
				i = dstemp[0]>>1;
				if (dstemp[6] & (1<<4)) {i--;}
                	} else {
				i = (uint8_t)(~dstemp[0])>>1;
				if (dstemp[6] & (1<<4)) {i++;}
                	}
			temp = hex2ascii((uint8_t)i);
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

			if (h>1) { lcd_write(0b11000000,0,0); }
			if (dstemp[1]) {
				lcd_write('-', 1,0);
			}
			
			if ((uint8_t)(temp>>16) > '0') { 
			    lcd_write((uint8_t)(temp>>16),1,0); 
			    lcd_write((uint8_t)(temp>>8),1,0);
			} else if ((uint8_t)(temp>>8) > '0') {
			    lcd_write((uint8_t)(temp>>8),1,0);
			}
			lcd_write((uint8_t)temp,1,0); 
			
                	lcd_write('.', 1,0);
			temp = hex2ascii(mantiss);
			lcd_write((uint8_t)(temp>>16),1,0);
			lcd_write('[',1,0);
			temp = hex2ascii(dstemp[6]);
			lcd_write((uint8_t)(temp>>8), 1,0);
			lcd_write((uint8_t)(temp), 1,0);
			lcd_write(']',1,0);
                	lcd_write(' ', 1,0);
			intc = 0;
			h++;
		    } while (h<roms);
		}
	}

}
