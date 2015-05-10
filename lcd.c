#define F_E	0
#define F_RW	1
#define F_RS	2
#define DATA_DDR	DDRB
#define DATA_PORT	PORTB
#define DATA_PIN	PINB
#define SPEED		12	
#define LCD_INT		5
#define LCD_8B		4	
#define LCD_2L		3	
#define	LCD_CLR		0
#define LCD_DSP		3
#define LCD_ON		2

void init_ports(void) {
	DATA_PORT &= 0xF0;
	DATA_DDR |= (1<<F_RS)|(1<<F_RW)|(1<<F_E);
}

void set_port_out(void) {
        DATA_DDR |= 0xF0;
}

void set_port_in(void) {
        DATA_DDR &= 0x0F;
        DATA_PORT |= 0xF0;
}

void delay_lcd(void) {
	_delay_us(1);
}

void wait_lcd_ready(unsigned char fb) {
	if (fb) { return; }				//fb - first boot, skip waiting for Wait flag
	set_port_in();
	//delay_lcd();
	unsigned char lcd_flag = 0;
	DATA_PORT &= ~(1<<F_RS);
	DATA_PORT |= (1<<F_RW); 
        do { 
		DATA_PORT |= (1<<F_E);
        	delay_lcd();
        	lcd_flag = DATA_PIN;
        	DATA_PORT &= ~(1<<F_E);
        	delay_lcd();
        	DATA_PORT |= (1<<F_E);
        	delay_lcd();
        	DATA_PORT &= ~(1<<F_E);
        	delay_lcd();
        } while (lcd_flag & 0x80);
}

void lcd_write(unsigned char c, unsigned char rs, unsigned char fb) {			//rs: 0 - cmd, 1 - data
	unsigned char data;								// fb
	wait_lcd_ready(fb);								// 0 - wait busy flag & 4 bit normal mode
	switch (rs) {									// 1 - doesnt wait busy flag, 4 bit init mode			
		case 0 : DATA_PORT &= ~(1<<F_RS); break;				//    when hi tetrade is sent twice
		case 1 : DATA_PORT |= (1<<F_RS); break;					// >1 - doesnt wait busy flag & 4 bit normal mode
	}
	DATA_PORT &= ~(1<<F_RW);
	DATA_PORT |= (1<<F_E);				//strob on
	set_port_out();
	data = (c & 0xF0) | (DATA_PORT & 0x0F);
	DATA_PORT = data;
	delay_lcd();
	DATA_PORT &= ~(1<<F_E);				//strob off
	delay_lcd();
	if (fb == 1) {					
		DATA_PORT |= (1<<F_E);			//additional strob for first init
		delay_lcd();
		DATA_PORT &= ~(1<<F_E);
		delay_lcd();
	}
	DATA_PORT |= (1<<F_E);				//strob on
	data = ((c & 0x0F) << 4) | (DATA_PORT & 0x0F);
	DATA_PORT = data;
	delay_lcd();
	DATA_PORT &= ~(1<<F_E);				//strob off
	set_port_in();	
	delay_lcd();
		
}

void lcd_write_str_p(const char *string) {
        while (pgm_read_byte(string)!='\0') {
            lcd_write(pgm_read_byte(string), 1,0 );
            string++;
        }
}

void lcd_init() {
	unsigned int lcdwait = 65535;
	init_ports();
	_delay_ms(1000);
	lcd_write((1<<LCD_INT)|(0<<LCD_8B)|(1<<LCD_2L),0, 1);		//first init fb=1
	_delay_ms(5);
	lcd_write((1<<LCD_INT)|(0<<LCD_8B)|(1<<LCD_2L),0, 2);
	_delay_us(100);
	lcd_write((1<<LCD_INT)|(0<<LCD_8B)|(1<<LCD_2L),0, 2);
	lcd_write((1<<LCD_CLR),0, 0);
	lcd_write(0b00000110,0, 0);
	lcd_write(0b00001100,0, 0);
	lcd_write((1<<LCD_CLR),0, 0);
}

#define lcd_shift_r() (lcd_write(0b00011000,0,0))
#define lcd_shift_l() (lcd_write(0b00011100,0,0))
#define lcd_clr() (lcd_write((1<<LCD_CLR),0, 0))

