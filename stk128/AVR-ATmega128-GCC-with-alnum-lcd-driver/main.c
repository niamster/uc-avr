#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include <string.h>

typedef unsigned char u8;
typedef signed char s8;

#define configure_leds(port) do {               \
        DDR##port = 0xFF;                       \
    } while (0)

#define light_leds(port, leds) do {                  \
        PORT##port = (~leds)&0xFF;                   \
    } while (0)

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
#define lcd_ver() lcd_putb(0x60)
#define lcd_goto(row, col) do { lcd_putb(0x40|(row)); lcd_putb(col); } while (0)

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
    configure_leds(C);
    light_leds(C, 0x0);

    lcd_init();

    lcd_clr();

    lcd_goto(0, 0);
    lcd_ver();
    lcd_goto(1, 0);
    lcd_puts("Hello, world ...");
    lcd_goto(2, 0);
    lcd_puts("Leds are blinking");

    for (;;) {
        light_leds(C, 0xFF);
        _delay_ms(1000);
        light_leds(C, 0x00);
        _delay_ms(1000);
    }

    return 0;
}
