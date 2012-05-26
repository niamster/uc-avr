#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#include <stdlib.h>
#include <string.h>

#include "vs1053/vs1053.h"
#include "spi/spi.h"

#define VS1053_XCS_SHARED
#define VS1053_SPI_TRANSACTIONS
/* #define VS1053_SPI_HIGHEST_SPEED */

// CLK freq = 12.288MHz[FREQ=0/default] * 3.5[MULT=4] = 43.008MHz
#define VS1053_CLKI_43008KHZ  0x9800

#define VS1053_CLKI_SPEED_KHZ 43008UL

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

#if defined(VS1053_XCS_SHARED)
#define VS1053_MODE_REG_VALUE (0                \
            | (1<<VS1053_SCI_REG_MODE_SDINEW)   \
            | (1<<VS1053_SCI_REG_MODE_SDISHARE) \
            | (1<<VS1053_SCI_REG_MODE_RESET))
#else
#define VS1053_MODE_REG_VALUE (0                \
            | (1<<VS1053_SCI_REG_MODE_SDINEW)   \
            | (1<<VS1053_SCI_REG_MODE_RESET))
#endif

#define VS1053_PARAMETRIC_ADDRESS 0x1E02
#define VS1053_PARAMETRIC_VERSION 0x0003

struct vs1053_parametric {
    /* configs are not cleared between files */
    uint16_t version;       /* 0x1E02      structure version */
    uint16_t config1;       /* 0x1E03      ---- ---- ppss RRRR PS mode, SBR mode, Reverb */
    uint16_t playSpeed;     /* 0x1E04      0,1 = normal speed, 2 = twice, 3 = three times etc. */
    uint16_t byteRate;      /* 0x1E05      average byterate */
    uint16_t endFillByte;   /* 0x1E06      byte value to send after file sent */
    uint16_t reserved[16];  /* 0x1E07..15  file byte offsets */
    uint32_t jumpPoints[8]; /* 0x1E16..25  file byte offsets */
    uint16_t latestJump;    /* 0x1E26      index to lastly updated jumpPoint */
    uint32_t positionMsec;  /* 0x1E27-28   play position, if known (WMA, Ogg Vorbis) */
    int16_t resync;         /* 0x1E29      > 0 for automatic m4a, ADIF, WMA resyncs */
    union {
        struct {
            uint32_t curPacketSize;
            uint32_t packetSize;
        } wma;
        struct {
            uint16_t sceFoundMask;   /* 0x1E2a   SCE's found since last clear */
            uint16_t cpeFoundMask;   /* 0x1E2b   CPE's found since last clear */
            uint16_t lfeFoundMask;   /* 0x1E2c   LFE's found since last clear */
            uint16_t playSelect;     /* 0x1E2d   0 = first any, initialized at aac init */
            int16_t dynCompress;     /* 0x1E2e   -8192=1.0, initialized at aac init */
            int16_t dynBoost;        /* 0x1E2f   8192=1.0, initialized at aac init */
            uint16_t sbrAndPsStatus; /* 0x1E30   1=SBR, 2=upsample, 4=PS, 8=PS active */
        } aac;
        struct {
            uint32_t bytesLeft;
        } midi;
        struct {
            int16_t gain; /* 0x1E2a proposed gain offset in 0.5dB steps, default = -12 */
        } vorbis;
    } i;
};

#define VS1053_PARAMETRIC_ENDFILLBYTE_ADDRESS (VS1053_PARAMETRIC_ADDRESS + __builtin_offsetof(struct vs1053_parametric, endFillByte))
#define VS1053_PARAMETRIC_VERSION_ADDRESS     (VS1053_PARAMETRIC_ADDRESS + __builtin_offsetof(struct vs1053_parametric, version))

#define min(a,b) ((a) < (b) ? (a) : (b))

static inline void vs1053_wait_data_request(void)
{
    while (!(VS1053_DREQ_PORT & (1<<VS1053_DREQ_BIT)));
}

static inline void vs1053_enable_cs(void)
{
    _delay_us(2*(1000./(double)VS1053_CLKI_SPEED_KHZ)); // tXCS  : 2 CLKI
    VS1053_XCS_PORT &= ~(1 << VS1053_XCS_BIT);
    _delay_us(0.005); // tXCSS : 5 ns
}

static inline void vs1053_disable_cs(void)
{
    _delay_us(1000./(double)VS1053_CLKI_SPEED_KHZ); // tXCSH : 1 CLKI
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
__vs1053_write_register(vs1053_sci_reg_t reg, uint16_t value)
{
#if !defined(VS1053_SPI_TRANSACTIONS)
    spi_write(0x2); // vs1053 write command
    spi_write(reg);

    spi_write(value >> 8);
    spi_write(value);
#else
    {
        uint8_t out[4] = {0x2, reg, (value>>8)&0xFF, value&0xFF};
        spi_transfer_t xfer = {.in = NULL, .out = out, .len = 4};
        spi_transfer(&xfer);
    }
#endif
}

void vs1053_write_register(vs1053_sci_reg_t reg, uint16_t value)
{
    vs1053_wait_data_request();

    vs1053_enable_cs();
    __vs1053_write_register(reg, value);
    vs1053_disable_cs();
}

static inline void
__vs1053_read_register(vs1053_sci_reg_t reg, uint16_t *value)
{
#if !defined(VS1053_SPI_TRANSACTIONS)
    {
        uint8_t c;

        spi_write(0x3); // vs1053 read command
        spi_write(reg);

        spi_read(&c);
        *value = c << 8;
        spi_read(&c);
        *value |= c;
    }
#else
    {
        uint8_t in[4], out[4] = {0x3, reg, 0xff, 0xff};
        spi_transfer_t xfer = {.in = in, .out = out, .len = 4};
        spi_transfer(&xfer);
        *value = in[2] << 8 | in[3];
    }
#endif
}


void vs1053_read_register(vs1053_sci_reg_t reg, uint16_t *value)
{
    vs1053_wait_data_request();

    vs1053_enable_cs();
    __vs1053_read_register(reg, value);
    vs1053_disable_cs();
}

static void vs1053_read_ram(uint16_t addr, uint8_t count, uint16_t *value)
{
    vs1053_enable_cs();

    /* internal pointer is autoincremented */
    vs1053_wait_data_request();
    __vs1053_write_register(VS1053_SCI_REG_WRAMADDR, addr);

    while (count) {
        vs1053_wait_data_request();
        __vs1053_read_register(VS1053_SCI_REG_WRAM, value);
        --count;
        ++value;
    }

    vs1053_disable_cs();
}

static void vs1053_write_ram(uint16_t addr, uint8_t count, uint16_t *value)
{
    vs1053_enable_cs();

    /* internal pointer is autoincremented */
    vs1053_wait_data_request();
    __vs1053_write_register(VS1053_SCI_REG_WRAMADDR, addr);

    while (count) {
        vs1053_wait_data_request();
        __vs1053_write_register(VS1053_SCI_REG_WRAM, *value);
        --count;
        ++value;
    }

    vs1053_disable_cs();
}

void vs1053_soft_reset(void)
{
    vs1053_write_register(VS1053_SCI_REG_MODE, VS1053_MODE_REG_VALUE);

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
    spi_setup(SPI_MODE0, spi_speed_to_clkdiv(1000000UL), SPI_BIT_ORDER_MSB_FIRST);

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

#if VS1053_CLKI_SPEED_KHZ == 43008UL
    vs1053_write_register(VS1053_SCI_REG_CLOCKF, VS1053_CLKI_43008KHZ);
#else
#error Unsupported VS1053 CLK speed
#endif
    _delay_ms(100);
    vs1053_wait_data_request();

#if defined(VS1053_SPI_HIGHEST_SPEED)
    // max SDI clock freq is CLKI/7
    spi_setup(SPI_MODE0, spi_speed_to_clkdiv((VS1053_CLKI_SPEED_KHZ*1000UL)/7UL), SPI_BIT_ORDER_MSB_FIRST);
#endif
}

static inline
void vs1053_push_chunk(const uint8_t *data, uint8_t len)
{
#if !defined(VS1053_SPI_TRANSACTIONS)
    uint8_t i;

    for (i=0;i<len;++i)
        spi_write(data[i]);
#else
    {
        spi_transfer_t xfer = {.in = NULL, .out = data, .len = len};
        spi_transfer(&xfer);
    }
#endif
}

static void vs1053_push_byte(uint8_t b, uint16_t count)
{
    uint8_t c[VS1053_DATA_CHUNK_SIZE];

    memset(c, b, VS1053_DATA_CHUNK_SIZE);

    while (count) {
        uint8_t l = min(count, VS1053_DATA_CHUNK_SIZE);

        vs1053_wait_data_request();

        vs1053_push_chunk(c, l);

        count -= l;
    }
}

void vs1053_play_progmem(const uint8_t *data, uint16_t len)
{
    uint8_t c[VS1053_DATA_CHUNK_SIZE];
    uint8_t sent;
    uint16_t efb;

    vs1053_enable_dcs();

    while (len) {
        uint8_t l = min(len, VS1053_DATA_CHUNK_SIZE);

        memcpy_P(c, data, l);

        vs1053_wait_data_request();

        vs1053_push_chunk(c, l);

        len -= l;
        data += l;
    }

    vs1053_disable_dcs();

    vs1053_read_ram(VS1053_PARAMETRIC_ENDFILLBYTE_ADDRESS, 1, &efb);

    vs1053_enable_dcs();
    vs1053_push_byte(efb, 2052);
    vs1053_disable_dcs();

    vs1053_write_register(VS1053_SCI_REG_MODE,
            VS1053_MODE_REG_VALUE | (1<<VS1053_SCI_REG_MODE_CANCEL));

    sent = 0;
    for (;;) {
        uint16_t mode;

        vs1053_enable_dcs();
        vs1053_push_byte(efb, 32);
        vs1053_disable_dcs();

        vs1053_read_register(VS1053_SCI_REG_MODE, &mode);

        if (!(mode&(1<<VS1053_SCI_REG_MODE_CANCEL)))
            break;

        sent += 32;
        if (sent >= 2048/32) {
            vs1053_soft_reset();
            break;
        }
    }
}

void vs1053_play(const uint8_t *data, uint16_t len)
{
    uint8_t sent;
    uint16_t efb;

    vs1053_enable_dcs();

    while (len) {
        uint8_t l = min(len, VS1053_DATA_CHUNK_SIZE);

        vs1053_wait_data_request();

        vs1053_push_chunk(data, l);

        len -= l;
        data += l;
    }

    vs1053_disable_dcs();

    vs1053_read_ram(VS1053_PARAMETRIC_ENDFILLBYTE_ADDRESS, 1, &efb);

    vs1053_enable_dcs();
    vs1053_push_byte(efb, 2052);
    vs1053_disable_dcs();

    vs1053_write_register(VS1053_SCI_REG_MODE,
            VS1053_MODE_REG_VALUE | (1<<VS1053_SCI_REG_MODE_CANCEL));

    sent = 0;
    for (;;) {
        uint16_t mode;

        vs1053_enable_dcs();
        vs1053_push_byte(efb, 32);
        vs1053_disable_dcs();

        vs1053_read_register(VS1053_SCI_REG_MODE, &mode);

        if (!(mode&(1<<VS1053_SCI_REG_MODE_CANCEL)))
            break;

        sent += 32;
        if (sent >= 2048/32) {
            vs1053_soft_reset();
            break;
        }
    }
}

void vs1053_play_sine(uint8_t pitch)
{
    vs1053_write_register(VS1053_SCI_REG_MODE,
            VS1053_MODE_REG_VALUE | (1<<VS1053_SCI_REG_MODE_TESTS));

    vs1053_wait_data_request();

    vs1053_enable_cs();

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
        uint8_t out[8] = {0x53, 0xEF, 0x6E, pitch, 0x0, 0x0, 0x0, 0x0};
        spi_transfer_t xfer = {.in = NULL, .out = out, .len = 8};
        spi_transfer(&xfer);
    }
#endif

    vs1053_disable_cs();
}
