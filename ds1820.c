#define ds_pin_up()     (DS_PORT |= (1<<DS_CTL))
#define ds_pin_down()   (DS_PORT &= ~(1<<DS_CTL))

void ds_port_in(void) {
        DS_DDR &= ~(1<<DS_CTL);
        DS_PORT &= ~(1<<DS_CTL);
}

void ds_port_out(void) {
        DS_DDR |= (1<<DS_CTL);
        DS_PORT |=(1<<DS_CTL);
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

void ds_read_scratch(uint8_t *ds_reg) {
        unsigned char i;
        for (i=0; i<9; i++) {
		*ds_reg = ds_read_byte();
		ds_reg++;
                //dstemp[i] = ds_read_byte();
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

unsigned char ds_search_rom(uint8_t *rom) {
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
			rombit = *(rom+(romind-1)*8+rombyte) & (1<<cnt) > 0;
                    }
                    if (!rombit) { last_zero = rombit_index; }
                }
                if (rombit) {
		    *(rom+romind*8+rombyte) |= (1<<cnt);
                } else {
		    *(rom+romind*8+rombyte) &= ~(1<<cnt);
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

