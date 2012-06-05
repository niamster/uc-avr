#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/setbaud.h>

#include <string.h>

#include <mi/mi.h>

#include <board.h>

#if !defined(UART_RX_INTERRUPT) && !defined(UART_RX_POLL)
#error UART module is not configured: specify UART RX option: UART_RX_POLL or UART_RX_INTERRUPT
#endif

#if defined(UART_RX_INTERRUPT)
#define USART_BUFFER_SIZE      16
#define USART_BUFFER_SIZE_MASK (USART_BUFFER_SIZE-1)

static uint8_t usart_buffer[USART_BUFFER_SIZE];
static uint8_t usart_buffer_head = 0;
static uint8_t usart_buffer_tail = 0;

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
        uint8_t c = UDR;
        (void)c;
    }
}
#endif

int usart_read(uint8_t *buf, int max)
{
#if defined(UART_RX_INTERRUPT)
    uint8_t head, tail;
    uint8_t p = 0;

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
#if defined(UART_RX_INTERRUPT)
	return (usart_buffer_tail - usart_buffer_head) & USART_BUFFER_SIZE_MASK;
#else
	return (UCSRA & (1<<RXC))?1:0;
#endif
}

inline
void usart_putc(uint8_t c)
{
    while (!(UCSRA & (1<<UDRE))); // wait for buffer to be empty

    UDR = c;

	/* while (!(UCSRA & (1<<TXC))); // wait for complete transfer */
}

void usart_write(const uint8_t *data, int len)
{
	int i;

	for (i=0; i<len; i++)
        usart_putc(data[i]);
}

void usart_puts(const uint8_t *data)
{
    while (*data) {
        usart_putc(*data);
        ++data;
    }
}

void usart_getc(uint8_t *c)
{
    while (usart_bytes_available() == 0);

    usart_read(c, 1);
}

static
void usart_init(void)
{
    UBRRH = UBRRH_VALUE;
    UBRRL = UBRRL_VALUE;

#if USE_2X
    UCSRA |= (1 << U2X);
#else
    UCSRA &= ~(1 << U2X);
#endif
#if defined(UART_RX_INTERRUPT)
	UCSRB = (1 << RXEN)|(1 << TXEN)|(1 << RXCIE);
#else
    UCSRB = (1 << RXEN)|(1 << TXEN);
#endif
	UCSRC = (1 << URSEL)|(1 << UCSZ1)|(1 << UCSZ0);
}

MI_INIT_MODULE(0, usart_init);
