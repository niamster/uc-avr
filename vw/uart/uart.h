#ifndef _UART_H_
#define _UART_H_

int usart_read(unsigned char *buf, int max);
void usart_write(const unsigned char *data, int len);
void usart_putc(unsigned char c);

char usart_buffer_poll(void);
unsigned char usart_wait_byte(void);

void usart_init(void);

#endif
