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
	while (!(UCSRA & (1<<RXC)));
	*buf = UDR;
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
	return UCSRA & (1<<RXC);
#endif
}

void usart_init(unsigned int baud)
{
    unsigned short ubrr = (((F_CPU / baud) >> 4) - 1);

	cli();
	UBRRH = (unsigned char)(ubrr>>8);
	UBRRL = (unsigned char)baud;

	UCSRA = 0;
#ifdef INTERRUPT_DRIVEN
	UCSRB = (1<<RXEN)|(1<<TXEN)|(1<<RXCIE);
#else
    UCSRB = (1<<RXEN)|(1<<TXEN);
#endif
	UCSRC = (1 << URSEL)|(1 << UCSZ1)|(1 << UCSZ0);
	sei();
}

void usart_write(const unsigned char *data, int len)
{
	int i;
	for (i=0; i<len; i++)
	{
		// wait for buffer to be empty
		while (!(UCSRA & (1<<UDRE)));
		UDR = data[i];
        //		data++;
	}
	// wait for complete transfer
	while (!(UCSRA & (1<<TXC)));
}

unsigned char usart_wait_byte(void)
{
	unsigned char c;
	while (!usart_buffer_poll());
	usart_read(&c, 1);
	return c;
}
