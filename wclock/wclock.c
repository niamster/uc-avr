#include <avr/io.h>
#include <avr/interrupt.h>

#include <mi/mi.h>

#if !defined(WCLOCK_FREQUENCY)
#warning No WCLOCK frequncy defined, using default value=1000
#define WCLOCK_FREQUENCY 1000
#endif

volatile uint32_t jiffies = 0;

SIGNAL(TIMER0_COMP_vect)
{
    ++jiffies;
}

#define WCLOCK_OCR_ESTIMATION (F_CPU / WCLOCK_FREQUENCY)

#if defined(__AVR_ATmega16__)
#if WCLOCK_OCR_ESTIMATION <= 0xFF // no prescaling
#define WCLOCK_TMR_CLK_DIV  ((0 << CS02)  | (0 << CS01)  | (1 << CS00))
#define WCLOCK_OCR_VALUE    (WCLOCK_OCR_ESTIMATION - 1)
#elif (WCLOCK_OCR_ESTIMATION/8) <= 0xFF // CLK/8
#define WCLOCK_TMR_CLK_DIV  ((0 << CS02)  | (1 << CS01)  | (0 << CS00))
#define WCLOCK_OCR_VALUE    (WCLOCK_OCR_ESTIMATION / 8 - 1)
#elif (WCLOCK_OCR_ESTIMATION/64) <= 0xFF // CLK/64
#define WCLOCK_TMR_CLK_DIV  ((0 << CS02)  | (1 << CS01)  | (1 << CS00))
#define WCLOCK_OCR_VALUE    (WCLOCK_OCR_ESTIMATION / 64 - 1)
#elif (WCLOCK_OCR_ESTIMATION/256) <= 0xFF // CLK/256
#define WCLOCK_TMR_CLK_DIV  ((1 << CS02)  | (0 << CS01)  | (0 << CS00))
#define WCLOCK_OCR_VALUE    (WCLOCK_OCR_ESTIMATION / 256 - 1)
#elif (WCLOCK_OCR_ESTIMATION/1024) <= 0xFF // CLK/1024
#define WCLOCK_TMR_CLK_DIV  ((1 << CS02)  | (0 << CS01)  | (1 << CS00))
#define WCLOCK_OCR_VALUE    (WCLOCK_OCR_ESTIMATION / 1024 - 1)
#else
#error WCLOCK frequency is out of range
#endif
#else
#error Unsupported platform
#endif

static
void wclock_init(void)
{
  TCCR0  = (1 << WGM01) | (0 << WGM00) | /* CTC mode        */
           (0 << COM01) | (0 << COM00) | /* OC0A disabled   */
           WCLOCK_TMR_CLK_DIV;

  OCR0   = WCLOCK_OCR_VALUE;

  TCNT0  = 0;             /* Reset counter  */
  TIFR   = (1 << OCF0);   /* Reset pending  */
  TIMSK  = (1 << OCIE0);  /* IRQ on compare */
}

MI_INIT_MODULE(0, wclock_init);
