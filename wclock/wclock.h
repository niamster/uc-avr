#ifndef _WCLOCK_H_
#define _WCLOCK_H_

#if !defined(WCLOCK_FREQUENCY)
#error WCLOCK frequency was not defined
#endif

extern volatile uint32_t jiffies;

void wclock_init(void);

static inline uint32_t
wclock_jiffies_to_msec(uint32_t j)
{
    return (j*1000)/WCLOCK_FREQUENCY;
}

static inline uint32_t
wclock_msec_to_jiffies(uint32_t m)
{
    return (m*WCLOCK_FREQUENCY)/1000;
}

static inline uint8_t
wclock_jiffies_after(uint32_t a, uint32_t b)
{
    return ((int32_t)b - (int32_t)a < 0);
}

static inline uint8_t
wclock_jiffies_before(uint32_t a, uint32_t b)
{
    return wclock_jiffies_after(b, a);
}

static inline uint8_t
wclock_jiffies_after_or_equal(uint32_t a, uint32_t b)
{
    return ((int32_t)b - (int32_t)a <= 0);
}

static inline uint8_t
wclock_jiffies_before_or_equal(uint32_t a, uint32_t b)
{
    return wclock_jiffies_after_or_equal(b, a);
}

#endif
