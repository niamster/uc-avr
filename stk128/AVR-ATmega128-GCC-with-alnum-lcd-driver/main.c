#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include <string.h>

typedef unsigned char u8;
typedef signed char s8;

/* PORTA[0] = busy */
/* PORTA[1] = clk */
/* PORTA[2] = data */
void lcd_init(void)
{
    DDRA |= 0x6;
    DDRA &= ~0x1;

    PORTA &= ~0x6;
}

void lcd_putb(u8 c)
{
    s8 i;

    c |= 1<<7; /* mandatory bit */

    while (PINA&1);

    for (i=7;i>=0;--i) {
        if ((c>>i)&0x1)
            PORTA |= 1<<2;
        else
            PORTA &= ~(1<<2);

        PORTA |= 1<<1;
        PORTA &= ~(1<<1);
    }
}

#define lcd_clr() lcd_putb(0x61)
#define lcd_goto(a) lcd_putb(0x40|((a)&0x1f))

void lcd_putc(u8 c)
{
    lcd_putb(1);
    lcd_putb(c);
}

void lcd_puts(u8 *s)
{
    if (!s || !*s)
        return;

    lcd_putb(strlen(s)&0x1F);

    while (*s) {
        lcd_putb(*s);
        ++s;
    }
}

int main(void)
{

    lcd_init();
    lcd_puts("HELLO WORLD");

    for (;;) {
        _delay_ms(1000);
    }

    return 0;
}
