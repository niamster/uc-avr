#include <avr/io.h>
#include <avr/interrupt.h>

/* #define INTERRUPT_DRIVEN */

#ifdef INTERRUPT_DRIVEN
static unsigned char usart_buffer[16];
static unsigned char usart_buffer_head = 0;
static unsigned char usart_buffer_tail = 0;

SIGNAL(SIG_USART1_RECV)
{
	usart_buffer[usart_buffer_head]=UDR1;
	usart_buffer_head++;
	usart_buffer_head &= 0xf;
}
#endif

int usart_read(unsigned char *buf, int max)
{
#ifdef INTERRUPT_DRIVEN
	int i=0;
	cli();
	while (usart_buffer_head != usart_buffer_tail && i<max) 
	{
		buf[i] = usart_buffer[usart_buffer_tail];
		usart_buffer_tail++;
		usart_buffer_tail &= 0xf;
		i++;
	}
	sei();
	return i;
#else
	while (!(UCSR1A & (1<<RXC1)));
	*buf = UDR1;
	return 1;
#endif
}

char usart_buffer_poll(void)
{
#ifdef INTERRUPT_DRIVEN
	cli();
	if (usart_buffer_head != usart_buffer_tail) {
		sei();
		return 1;
	}
	sei();
	return 0;
#else
	return UCSR1A & (1<<RXC1);
#endif
}

unsigned char usart_wait_byte(void)
{
	unsigned char c;
	while (!usart_buffer_poll());
	usart_read(&c, 1);
	return c;
}

void usart_write(const unsigned char *data, int len)
{
	int i;
	for (i=0; i<len; i++)
	{
		// wait for buffer to be empty
		while (!(UCSR1A & (1<<UDRE1)));
		UDR1 = data[i];
        //		data++;
	}
	// wait for complete transfer
	while (!(UCSR1A & (1<<TXC1)));
}

void usart_init(unsigned int baud)
{
    unsigned long ubrr = (((F_CPU / baud) >> 4) - 1);

	UBRR1H = (unsigned char)(ubrr>>8);
	UBRR1L = (unsigned char)ubrr;

	UCSR1A = 0;
#ifdef INTERRUPT_DRIVEN
	UCSR1B = (1<<RXEN1)|(1<<TXEN1)|(1<<RXCIE1);
#else
    UCSR1B = (1<<RXEN1)|(1<<TXEN1);
#endif
	UCSR1C = (1 << UCSZ11)|(1 << UCSZ10);
}
