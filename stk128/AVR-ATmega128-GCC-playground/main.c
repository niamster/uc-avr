#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include <string.h>

typedef unsigned char u8;
typedef signed char s8;

typedef void (*led_cbk_t)(void);

#define configure_leds(port) do {               \
        DDR##port = 0xFF;                       \
    } while (0)

#define light_leds(port, leds) do {                  \
        PORT##port = (~leds)&0xFF;                   \
    } while (0)

volatile u8 int_flag = 0;
#define int_ack(i) do {                       \
        int_flag |= 1 << (i);                 \
    } while (0)
#define int_clear_ack(i) do {                 \
        int_flag &= ~(1 << (i));              \
    } while (0)
#define int_received(i) (int_flag & (1 << (i)))

void led_cbk_0(void)
{
    u8 i;
    u8 led = 0;
    for (i=0;i<8;++i) {
        if (int_received(1))
            return;

        led = (1<<i) | (1<<(7-i));
        light_leds(C, led);
        if (i != 3 && i != 7)
            _delay_ms(500);
    }
}

void led_cbk_1(void)
{
    u8 i;
    u8 led = 0;
    for (i=0;i<16;++i) {
        if (int_received(1))
            return;

        if (i<8)
            led |= (1<<i);
        else
            led &= ~(1<<(i-8));
        light_leds(C, led);
        _delay_ms(500);
    }
}

#define LED_CBK_QTY 2
led_cbk_t led_cbk[LED_CBK_QTY] = {
    led_cbk_0,
    led_cbk_1,
};

volatile u8 cur_led_cbk = 0;

ISR(INT0_vect, ISR_BLOCK)
{
    u8 i;

    ++cur_led_cbk;
    if (cur_led_cbk >= LED_CBK_QTY)
        cur_led_cbk = 0;

    int_ack(0);
}

ISR(INT1_vect, ISR_BLOCK)
{
    u8 i;

    light_leds(C, 0xFF);
    for (i=0;i<8;++i) {
        light_leds(C, 1<<i);
        _delay_ms(50);
    }

    int_ack(1);
}

#define IT_LEVEL        0
#define IT_RAISING_EDGE 1
#define IT_FALLING_EDGE 2

void enable_interrupt(u8 it, u8 sence)
{
    u8 eicr, itcr;

    if (it > 7)
        return;

    if (it > 3) {
        eicr = EICRB;
        itcr = it - 4;
    } else {
        eicr = EICRA;
        itcr = it;
    }

    switch (sence) {
        case IT_LEVEL:
            eicr &= ~(3<<itcr);
            break;
        case IT_RAISING_EDGE:
            eicr |= 3<<itcr;
            break;
        case IT_FALLING_EDGE:
            eicr |= 2<<itcr;
            break;
    }

    if (it > 3)
        EICRB = eicr;
    else
        EICRA = eicr;

    EIMSK |= 1 << it;
}

int main(void)
{
    u8 cnt;

    configure_leds(C);
    light_leds(C, 0x0);

    enable_interrupt(0, IT_RAISING_EDGE);
    enable_interrupt(1, IT_LEVEL);

    sei();

    light_leds(C, 0x1);

    for (;;) {
        _delay_ms(1000);
        int_clear_ack(0);
        int_clear_ack(1);

        led_cbk[cur_led_cbk]();
    }

    return 0;
}
