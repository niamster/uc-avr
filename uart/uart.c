#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/setbaud.h>

#include <string.h>

#include <mi/mi.h>

#include <board.h>

#if !defined(UART_RX_INTERRUPT) && !defined(UART_RX_POLL)
#error UART module is not configured: specify UART RX option(UART_RX_POLL or UART_RX_INTERRUPT)
#endif

#if defined(UART_RX_INTERRUPT)
#define USART_BUFFER_SIZE      16
#define USART_BUFFER_SIZE_MASK (USART_BUFFER_SIZE-1)

static uint8_t usart_buffer[USART_BUFFER_SIZE];
static uint8_t usart_buffer_rd = 0;
static uint8_t usart_buffer_wr = 0;

SIGNAL(USART_RXC_vect)
{
    uint8_t next_wr = (usart_buffer_wr + 1) & USART_BUFFER_SIZE_MASK;
    uint8_t c = UDR;
    if (next_wr == usart_buffer_rd) /* FIFO full, drop */
        (void)c;
    else {
        usart_buffer[usart_buffer_wr] = UDR;
        usart_buffer_wr = next_wr;
    }
}
#endif

int usart_read(uint8_t *buf, int max)
{
#if defined(UART_RX_INTERRUPT)
    uint8_t rd, wr;
    uint8_t p = 0;

	cli();
    rd = usart_buffer_rd;
    wr = usart_buffer_wr;
    sei();

    while (rd != wr && p < max) {
        buf[p] = usart_buffer[rd];
        ++p;

        ++rd;
        rd &= USART_BUFFER_SIZE_MASK;
    }

    cli();
    usart_buffer_rd = rd;
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
	return (usart_buffer_wr - usart_buffer_rd) & USART_BUFFER_SIZE_MASK;
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
