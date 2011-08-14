#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include <string.h>

typedef unsigned char u8;
typedef signed char s8;

u8 irq_save(void)
{
    // Save status register and disable interrupts
    u8 flags = SREG;
    cli();
    return flags;
}

void irq_restore(u8 flags)
{
    // Re-enable interrupts (if they were ever enabled)
    SREG = flags;
}

#define configure_leds(port) do {               \
        DDR##port = 0xFF;                       \
    } while (0)

#define light_leds(port, leds) do {                  \
        PORT##port = (~leds)&0xFF;                   \
    } while (0)

/* PORTD[2] = busy [interrupt] */
/* PORTA[1] = clk  [out] */
/* PORTA[2] = data [out] */

void lcd_init(void)
{
    /* Setup clk and data pins */
    DDRA |= 0x6;
    PORTA &= ~0x6;

    /* Setup busy pin */
    DDRA &= ~0x1;

    /* Setup INT2 */
    EICRA &= ~(3 << 2);
}

static inline void lcd_enable_interrupt(void)
{
    EIMSK |= 1 << 2;
}

static inline void lcd_disable_interrupt(void)
{
    EIMSK &= ~(1 << 2);
}

static inline u8 lcd_busy(void)
{
    return PIND&4;
}

static inline void set_bit(u8 bit)
{
    if (bit)
        PORTA |= 1<<2;
    else
        PORTA &= ~(1<<2);
}

static inline void push_bit(void)
{
    PORTA |= 1<<1;
    PORTA &= ~(1<<1);
}

#define LCD_CMD_FIFO_LEN 256
#define LCD_CMD_FIFO_MSK (LCD_CMD_FIFO_LEN - 1)

u8 lcd_cmd_fifo[LCD_CMD_FIFO_LEN];
volatile u8 lcd_cmd_fifo_rd = 0, lcd_cmd_fifo_wr = 0;

void lcd_process_cmd_nolock(u8 max)
{
    u8 cmd, curr;
    s8 i;

    if (!max)
        return;

    while (max--) {
        if (lcd_cmd_fifo_rd == lcd_cmd_fifo_wr) {
            lcd_disable_interrupt();
            return;
        }

        if (lcd_busy())
            return;

        curr = lcd_cmd_fifo_rd;
        cmd = lcd_cmd_fifo[curr];
        lcd_cmd_fifo_rd = (curr + 1)&LCD_CMD_FIFO_MSK;

        for (i=7;i>=0;--i) {
            set_bit((cmd>>i)&0x1);
            push_bit();
        }
    }
}

void lcd_process_cmd(u8 max)
{
    u8 cmd, curr;
    s8 i;
    u8 flags;

    flags = irq_save();
    lcd_process_cmd_nolock(max);
    irq_restore(flags);
}

ISR(INT2_vect, ISR_BLOCK)
{
    lcd_process_cmd_nolock((u8)~0);
}

void lcd_push_cmd_nolock(u8 c)
{
    u8 curr, next;

    c |= 1<<7; /* mandatory bit */

    do {
        lcd_process_cmd_nolock(1);      /* move the queue to avoid deadlock */

        curr = lcd_cmd_fifo_wr;
        next = (curr + 1)&LCD_CMD_FIFO_MSK;
    } while (next == lcd_cmd_fifo_rd);

    lcd_cmd_fifo[curr] = c;

    lcd_cmd_fifo_wr = next;

    lcd_enable_interrupt();
}

#define lcd_clr() do { u8 flags = irq_save(); lcd_push_cmd_nolock(0x61); irq_restore(flags); } while (0)
#define lcd_ver() do { u8 flags = irq_save(); lcd_push_cmd_nolock(0x60); irq_restore(flags); } while (0)
#define lcd_goto_nolock(row, col) do { lcd_push_cmd_nolock(0x40|(row)); lcd_push_cmd_nolock(col); } while (0)
#define lcd_goto(row, col) do { u8 flags = irq_save(); lcd_goto_nolock(row, col); irq_restore(flags); } while (0)

void lcd_putc_nolock(u8 c)
{
    lcd_push_cmd_nolock(1);
    lcd_push_cmd_nolock(c);
}

void lcd_putc(u8 c)
{
    u8 flags = irq_save();

    lcd_putc_nolock(c);

    irq_restore(flags);
}

void lcd_puts(u8 *s)
{
    u8 flags;

    if (!s || !*s)
        return;

    flags = irq_save();

    lcd_push_cmd_nolock(strlen(s)&0x1F);

    while (*s) {
        lcd_push_cmd_nolock(*s);
        ++s;
    }

    irq_restore(flags);
}

u8 chars[] = "Oo";
u8 cur;

ISR(TIMER3_COMPA_vect, ISR_NOBLOCK)
{
    u8 flags;

    flags = irq_save();
    lcd_goto_nolock(3, 0);
    lcd_putc_nolock(chars[cur]);
    irq_restore(flags);

    flags = irq_save();
    lcd_goto_nolock(3, 19);
    lcd_putc_nolock(chars[cur]);
    irq_restore(flags);

    ++cur;
    cur &= sizeof(chars) - 2;
}

int main(void)
{
    configure_leds(C);
    light_leds(C, 0x0);

    lcd_init();

    /* enable interrupts */
    sei();

    lcd_clr();

    /* OC3A/OC3B disconnected */
    TCCR3A = 0;
    /* CTC mode, top in OCR3A / Timer clock = CLK/1024 */
    TCCR3B = (1<<WGM32)|(1<<CS32)|(1<<CS30);
    /* set OCR3A top value for 2KHz (2/2 periods in one second) */
    /*   0x01 for CPU @ 7.3728MHz (7372800/1024)/2 */
    OCR3A = 0x0E10;
    /* enable Timer/Counter 3 interrupt */
    ETIMSK |= (1<<OCIE3A);

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
