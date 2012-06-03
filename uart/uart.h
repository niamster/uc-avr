#ifndef _UART_H_
#define _UART_H_

#define UART_INTERRUPT_DRIVEN

int usart_read(uint8_t *buf, int max);
void usart_write(const uint8_t *data, int len);

void usart_putc(uint8_t c);
void usart_puts(const uint8_t *data);
void usart_getc(const uint8_t *c);

/* returns available bytes in buffer */
int usart_bytes_available(void);

/* boud rate is defined by BAUD preprocessor definition */
void usart_init(void);

#endif
