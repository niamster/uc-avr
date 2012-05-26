#ifndef _UART_H_
#define _UART_H_

#define UART_INTERRUPT_DRIVEN

int usart_read(unsigned char *buf, int max);
void usart_write(const unsigned char *data, int len);

void usart_putc(unsigned char c);
void usart_puts(const unsigned char *data);
void usart_getc(unsigned char *c);

/* returns available bytes in buffer */
int usart_bytes_available(void);

/* boud rate is defined by BAUD preprocessor definition */
void usart_init(void);

#endif
