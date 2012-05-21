#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#include <stdlib.h>

#include "vs1053/vs1053.h"
#include "spi/spi.h"

#define VS1053_XCS_SHARED
#define VS1053_SPI_TRANSACTIONS
/* #define VS1053_SPI_HIGHEST_SPEED */

#if !defined(VS1053_XCS_SHARED)
#define VS1053_XDCS_DDR    DDRA
#define VS1053_XDCS_PORT   PORTA
#define VS1053_XDCS_BIT    0
#endif

#define VS1053_DREQ_DDR    DDRA
#define VS1053_DREQ_PORT   PINA
#define VS1053_DREQ_PU     PORTA
#define VS1053_DREQ_BIT    1

#define VS1053_XRES_DDR    DDRA
#define VS1053_XRES_PORT   PORTA
#define VS1053_XRES_BIT    2

#define VS1053_XCS_DDR     DDRA
#define VS1053_XCS_PORT    PORTA
#define VS1053_XCS_BIT     3

#define VS1053_DATA_CHUNK_SIZE 32

#define min(a,b) ((a) < (b) ? (a) : (b))

static inline void vs1053_wait_data_request(void)
{
    while (!(VS1053_DREQ_PORT & (1<<VS1053_DREQ_BIT)));
}

static inline void vs1053_enable_cs(void)
{
    VS1053_XCS_PORT &= ~(1 << VS1053_XCS_BIT);
}

static inline void vs1053_disable_cs(void)
{
    VS1053_XCS_PORT |= 1 << VS1053_XCS_BIT;
}

#if !defined(VS1053_XCS_SHARED)
static inline void vs1053_enable_dcs(void)
{
    VS1053_XDCS_PORT &= ~(1 << VS1053_XDCS_BIT);
}

static inline void vs1053_disable_dcs(void)
{
    VS1053_XDCS_PORT |= 1 << VS1053_XDCS_BIT;
}
#else
static inline void vs1053_enable_dcs(void)
{
}

static inline void vs1053_disable_dcs(void)
{
}
#endif

static inline void
__vs1053_write_register(vs1053_sci_reg_t reg, unsigned int value)
{
    _delay_us(1); // tXCSS

#if !defined(VS1053_SPI_TRANSACTIONS)
    spi_write(0x2); // vs1053 write command
    spi_write(reg);

    spi_write(value >> 8);
    spi_write(value);
#else
    {
        unsigned char out[4] = {0x2, reg, (value>>8)&0xFF, value&0xFF};
        spi_transfer_t xfer = {.in = NULL, .out = out, .len = 4};
        spi_transfer(&xfer);
    }
#endif

    _delay_us(1); // tXCSH
}

void vs1053_write_register(vs1053_sci_reg_t reg, unsigned int value)
{
    vs1053_wait_data_request();

    vs1053_enable_cs();
    __vs1053_write_register(reg, value);
    vs1053_disable_cs();

    _delay_us(1); // tXCS
}

static inline void
__vs1053_read_register(vs1053_sci_reg_t reg, unsigned int *value)
{
    _delay_us(1); // tXCSS

#if !defined(VS1053_SPI_TRANSACTIONS)
    {
        unsigned char c;

        spi_write(0x3); // vs1053 read command
        spi_write(reg);

        spi_read(&c);
        *value = c << 8;
        spi_read(&c);
        *value |= c;
    }
#else
    {
        unsigned char in[4], out[4] = {0x3, reg, 0xff, 0xff};
        spi_transfer_t xfer = {.in = in, .out = out, .len = 4};
        spi_transfer(&xfer);
        *value = in[2] << 8 | in[3];
    }
#endif

    _delay_us(1); // tXCSH
}


void vs1053_read_register(vs1053_sci_reg_t reg, unsigned int *value)
{
    vs1053_wait_data_request();

    vs1053_enable_cs();
    __vs1053_read_register(reg, value);
    vs1053_disable_cs();

    _delay_us(1); // tXCS
}

void vs1053_soft_reset(void)
{
#if !defined(VS1053_XCS_SHARED)
    vs1053_write_register(VS1053_SCI_REG_MODE,
            (1<<VS1053_SCI_REG_MODE_SDINEW)
            | (1<<VS1053_SCI_REG_MODE_RESET));
#else
    vs1053_write_register(VS1053_SCI_REG_MODE,
            (1<<VS1053_SCI_REG_MODE_SDINEW)
            | (1<<VS1053_SCI_REG_MODE_RESET)
            | (1<<VS1053_SCI_REG_MODE_SDISHARE));
#endif
    _delay_ms(1);
    vs1053_wait_data_request();
}

void vs1053_setup(void)
{
    VS1053_XRES_DDR  |= 1 << VS1053_XRES_BIT;
    VS1053_XRES_PORT &= ~(1 << VS1053_XRES_BIT);

#if !defined(VS1053_XCS_SHARED)
    VS1053_XDCS_DDR  |= 1 << VS1053_XDCS_BIT;
    VS1053_XDCS_PORT |= 1 << VS1053_XDCS_BIT;
#endif

    VS1053_XCS_DDR  |= 1 << VS1053_XCS_BIT;
    VS1053_XCS_PORT |= 1 << VS1053_XCS_BIT;

    VS1053_DREQ_DDR  &= ~(1 << VS1053_DREQ_BIT);
    /* VS1053_DREQ_PU   |= 1 << VS1053_DREQ_BIT; */

    spi_enable();
    spi_setup(SPI_MODE0, SPI_CLK_DIV16/* 1MHz@Fosc=16MHz */, SPI_BIT_ORDER_MSB_FIRST);

    VS1053_XRES_PORT &= ~(1 << VS1053_XRES_BIT);
    _delay_ms(1);
    VS1053_XRES_PORT |= 1 << VS1053_XRES_BIT;
    _delay_ms(10);

    vs1053_soft_reset();

    vs1053_write_register(VS1053_SCI_REG_VOL, 0xffff);    /* switch analog off */
    _delay_ms(1);
    vs1053_write_register(VS1053_SCI_REG_AUDATA,10);      /* 10Hz : slow sample rate for slow analog part startup */
    vs1053_write_register(VS1053_SCI_REG_VOL, 0xfefe);    /* switch analog on, silence */
    vs1053_write_register(VS1053_SCI_REG_AUDATA, 44101);  /* 44.1kHz stereo */

#if 0
    vs1053_write_register(VS1053_SCI_REG_VOL, 0x2020);    /* initial volume = 0x20*(-0.5) = -16dB */
#endif
    vs1053_write_register(VS1053_SCI_REG_VOL, 0x0000); /* max vol */

    // CLK freq = 12.288MHz[FREQ=0/default] * 3[MULT=0] = 43.008MHz
    vs1053_write_register(VS1053_SCI_REG_CLOCKF, 0x9800);
    _delay_ms(100);
    vs1053_wait_data_request();

#if defined(VS1053_SPI_HIGHEST_SPEED)
    // max SDI clock freq is CLKI/7, XTALI = 12.288MHz, CLKI = 43.008MHz -> max SDI clock freq = CLKI/7 = 6.144MHz
    spi_setup(SPI_MODE0, SPI_CLK_DIV4/* 4MHz@Fosc=16MHz */, SPI_BIT_ORDER_MSB_FIRST);
#endif
}

static inline
void vs1053_push_chunk(const unsigned char *data, unsigned char len)
{
#if !defined(VS1053_SPI_TRANSACTIONS)
    unsigned char i;

    for (i=0;i<len;++i)
        spi_write(data[i]);
#else
    {
        spi_transfer_t xfer = {.in = NULL, .out = data, .len = len};
        spi_transfer(&xfer);
    }
#endif
}

static void vs1053_push_zeros(unsigned int len)
{
    unsigned char c[VS1053_DATA_CHUNK_SIZE] = {0x0, };

    while (len) {
        unsigned int l = min(len, VS1053_DATA_CHUNK_SIZE);

        vs1053_wait_data_request();

        vs1053_push_chunk(c, l);

        len -= l;
    }
}

void vs1053_play_progmem(const unsigned char *data, unsigned int len)
{
    unsigned char c[VS1053_DATA_CHUNK_SIZE];
    unsigned char sent;

    vs1053_enable_dcs();

    vs1053_push_zeros(10);

    while (len) {
        unsigned int l = min(len, VS1053_DATA_CHUNK_SIZE);

        memcpy_P(c, data, l);

        vs1053_wait_data_request();

        _delay_us(3);

        vs1053_push_chunk(c, l);

        len -= l;
        data += l;
    }

    vs1053_push_zeros(2052);

    vs1053_disable_dcs();
}

void vs1053_play(const unsigned char *data, unsigned int len)
{
    vs1053_enable_dcs();

    vs1053_push_zeros(10);

    while (len) {
        unsigned char l = min(len, VS1053_DATA_CHUNK_SIZE);

        vs1053_wait_data_request();

        _delay_us(3);

        vs1053_push_chunk(data, l);

        len -= l;
        data += l;
    }

    vs1053_push_zeros(2052);

    vs1053_disable_dcs();
}

void vs1053_play_sine(unsigned char pitch)
{
#if !defined(VS1053_XCS_SHARED)
    vs1053_write_register(VS1053_SCI_REG_MODE,
            (1<<VS1053_SCI_REG_MODE_SDINEW)
            | (1<<VS1053_SCI_REG_MODE_TESTS));
#else
    vs1053_write_register(VS1053_SCI_REG_MODE,
            (1<<VS1053_SCI_REG_MODE_SDINEW)
            | (1<<VS1053_SCI_REG_MODE_TESTS)
            | (1<<VS1053_SCI_REG_MODE_SDISHARE));
#endif

    vs1053_wait_data_request();

    vs1053_enable_cs();
    _delay_us(1); // tXCSS

#if !defined(VS1053_SPI_TRANSACTIONS)
    spi_write(0x53);
    spi_write(0xEF);
    spi_write(0x6E);
    spi_write(pitch);

    spi_write(0);
    spi_write(0);
    spi_write(0);
    spi_write(0);
#else
    {
        unsigned char out[8] = {0x53, 0xEF, 0x6E, pitch, 0x0, 0x0, 0x0, 0x0};
        spi_transfer_t xfer = {.in = NULL, .out = out, .len = 8};
        spi_transfer(&xfer);
    }
#endif

    _delay_us(1); // tXCSH
    vs1053_disable_cs();

    _delay_us(1); // tXCS
}
