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

void uart_send_str () {
	UDR = *str;
	str++; 
	UCSRB|=(1<<UDRIE);
}
