#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/setbaud.h>

#include <string.h>

#ifdef INTERRUPT_DRIVEN
#define USART_BUFFER_SIZE      16
#define USART_BUFFER_SIZE_MASK (USART_BUFFER_SIZE-1)

static unsigned char usart_buffer[USART_BUFFER_SIZE];
static unsigned char usart_buffer_head = 0;
static unsigned char usart_buffer_tail = 0;

SIGNAL(USART_RXC_vect)
{
    /* one element will be always unused
     * this is the only option to avoid ambiguous situation when both pointers equal and FIFO is full
     * and keep things simple
     */
    if (((usart_buffer_tail - usart_buffer_head) & USART_BUFFER_SIZE_MASK) < (USART_BUFFER_SIZE-1)) {
        usart_buffer[usart_buffer_tail] = UDR;
        ++usart_buffer_tail;
        usart_buffer_tail &= USART_BUFFER_SIZE_MASK;
    } else { // drop
        unsigned char c = UDR;
        c;
    }
}
#endif

int usart_read(unsigned char *buf, int max)
{
#ifdef INTERRUPT_DRIVEN
    unsigned char head, tail;
    unsigned char p = 0;

	cli();
    head = usart_buffer_head;
    tail = usart_buffer_tail;
    sei();

    while (head != tail && p < max) {
        buf[p] = usart_buffer[head];
        ++p;

        ++head;
        head &= USART_BUFFER_SIZE_MASK;
    }

    cli();
    usart_buffer_head = head;
    sei();

    return p;
#else
	if (!(UCSRA & (1<<RXC)) || max == 0)
        return 0;

    *buf = UDR;

	return 1;
#endif
}

int usart_bytes_available(void)
{
#ifdef INTERRUPT_DRIVEN
	return (usart_buffer_tail - usart_buffer_head) & USART_BUFFER_SIZE_MASK;
#else
	return (UCSRA & (1<<RXC))?1:0;
#endif
}

inline
void usart_putc(unsigned char c)
{
    while (!(UCSRA & (1<<UDRE))); // wait for buffer to be empty

    UDR = c;

	/* while (!(UCSRA & (1<<TXC))); // wait for complete transfer */
}

void usart_write(const unsigned char *data, int len)
{
	int i;

	for (i=0; i<len; i++)
        usart_putc(data[i]);
}

void usart_puts(unsigned char *data)
{
    while (*data) {
        usart_putc(*data);
        ++data;
    }
}

void usart_getc(unsigned char *c)
{
    while (usart_bytes_available() == 0);

    usart_read(c, 1);
}

void usart_init(void)
{
    UBRRH = UBRRH_VALUE;
    UBRRL = UBRRL_VALUE;

#if USE_2X
    UCSRA |= (1 << U2X);
#else
    UCSRA &= ~(1 << U2X);
#endif
#ifdef INTERRUPT_DRIVEN
	UCSRB = (1 << RXEN)|(1 << TXEN)|(1 << RXCIE);
#else
    UCSRB = (1 << RXEN)|(1 << TXEN);
#endif
	UCSRC = (1 << URSEL)|(1 << UCSZ1)|(1 << UCSZ0);
}
