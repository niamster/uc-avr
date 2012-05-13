#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include <string.h>

#include "uart/uart.h"
#include "vw/VirtualWire.h"
#include "sh/sh.h"

typedef unsigned char u8;
typedef signed char s8;

static void shVw(int argc, char **argv)
{
    int i;

    for (i=0;i<argc;++i) {
        usart_write("Sending:", 8);
        usart_write(argv[i], strlen(argv[i]));
        usart_write("\r\n", 2);
        vw_wait_tx();
        vw_send(argv[i], strlen(argv[i]));
    }

    if (i==0)
        usart_write("\r\n", 2);
}

static struct shCmd shVwCmd = {
	.cmd  = "vw",
	.cbk  = shVw,
    .next = NULL
};

int main(void)
{
    usart_init();
    vw_setup(1200);

    shCmdRegisterCmd(&shVwCmd);

    sei();

    for (;;) {
        shProcessUart();
    }

    return 0;
}
