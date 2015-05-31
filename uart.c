#define baudrate 9600L
#define bauddivider (F_CPU/(16*baudrate)-1)
#define HI(x) ((x)>>8)
#define LO(x) ((x)& 0xFF)


ISR (USART_UDRE_vect) {
	if (*str) {
		UDR = *str;
		str++;
	} else {
		UCSRB &=~(1<<UDRIE);
	}
}

void uart_init () {
	//Init UART
	UBRRL = LO(bauddivider);
	UBRRH = HI(bauddivider);
	UCSRA = 0;
	UCSRB = 1<<RXEN|1<<TXEN|0<<RXCIE|0<<TXCIE;
	UCSRC = 1<<URSEL|1<<UCSZ0|1<<UCSZ1;
}

void uart_txc_wait() {
        while (! (UCSRA & (1<<TXC))) {
        }
}

void uart_send_str (char *string) {

	str = string;
	UDR = *str;
	str++; 
	UCSRA |= (1<<TXC);
	UCSRB |= (1<<UDRIE);
}

void uart_strncat (const char *src, uint8_t size) {
	strncat((char *)&uartbuffer, src, size); 
}

void uart_strcpy (char *src) {
	strcpy((char *)&uartbuffer, src);
}

void uart_print_int(unsigned int n) {
	unsigned long m;
        m = hex2ascii(n);
	uint8_t sbyte;

	if ((uint8_t)m > '0') {
		uart_strncat((char *)&m, 2);
	} else if ((uint8_t)(m>>8) > '0') {
		uart_strncat((char *)&m+1, 1);
	}
	
	//sbyte = (uint8_t)m;	
	uart_strncat((char *)&m+2, 1);
}

void uart_print_hex(uint8_t b) {
	unsigned int m;
	m = hex2bcd(b);
	
	uart_strncat((char *)&m+1, 1);
	uart_strncat((char *)&m, 1); 
}
