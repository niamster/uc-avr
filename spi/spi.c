#include <avr/io.h>
#include <avr/interrupt.h>

#include <inttypes.h>

#include <spi/spi.h>

#include <board.h>

#if defined(__AVR_ATmega16__)
#define SPI_SPCR_MODE_MASK    0x0C  // CPOL = bit 3, CPHA = bit 2
#define SPI_SPCR_CLOCK_MASK   0x03  // SPR1 = bit 1, SPR0 = bit 0
#define SPI_SPCR_ORDER_MASK   0x20  // DORD = bit 5

#define SPI_SPSR_2XCLOCK_MASK 0x01  // SPI2X = bit 0
#else
#error Unsupported platform
#endif

#if !defined(SPI_CLK_DEFINED)
#error SPI CLK port is not defined
#endif
#if !defined(SPI_MISO_DEFINED)
#error SPI MISO port is not defined
#endif
#if !defined(SPI_MOSI_DEFINED)
#error SPI MOSI port is not defined
#endif
#if !defined(SPI_SS_DEFINED)
#error SPI SS port is not defined
#endif

static uint8_t spi_async_xfer_complete = 1;
static spi_transfer_t *spi_async_xfer;
static uint16_t spi_async_xfer_idx;

SIGNAL(SPI_STC_vect)
{
    uint8_t c;
    spi_transfer_t *xfer = spi_async_xfer;

    c = SPDR; // clean SPIF

    if (xfer->in)
        xfer->in[spi_async_xfer_idx] = c;

    ++spi_async_xfer_idx;

    if (spi_async_xfer_idx == xfer->len) {
        spi_async_xfer_complete = 1;
        SPCR &= ~_BV(SPIE);
    } else {
        if (xfer->out)
            SPDR = xfer->out[spi_async_xfer_idx];
        else
            SPDR = 0xFF;
    }
}

void spi_enable(void)
{
    /* MOSI, SCK and SS must be output, all others input */
    SPI_CLK_DDR  |= 1 << SPI_CLK_BIT;
    SPI_CLK_PORT &= ~(1 << SPI_CLK_BIT);

    SPI_MOSI_DDR  |= 1 << SPI_MOSI_BIT;
    SPI_MOSI_PORT &= ~(1 << SPI_MOSI_BIT);

    SPI_MISO_DDR &= ~(1 << SPI_MISO_BIT);
    /* SPI_MISO_PU  |= 1 << SPI_MISO_BIT; */

    SPI_SS_DDR  |= 1 << SPI_SS_BIT;
    SPI_SS_PORT |= 1 << SPI_SS_BIT;

    SPCR |= _BV(MSTR);
    SPCR |= _BV(SPE);
}

void spi_disable(void)
{
    SPCR &= ~_BV(SPE);
}

void spi_setup(spi_mode_t mode, spi_clk_div_t clk_div, spi_bit_order_t bit_order)
{
    uint8_t spcr = SPCR & ~(SPI_SPCR_MODE_MASK | SPI_SPCR_CLOCK_MASK | SPI_SPCR_ORDER_MASK);
    uint8_t spsr = SPSR & ~SPI_SPSR_2XCLOCK_MASK;

    if (bit_order == SPI_BIT_ORDER_LSB_FIRST)
        spcr |= _BV(DORD);
    spcr |= mode;

    spcr |= clk_div&0x3;
    spsr |= clk_div>>2;

    SPCR = spcr;
    SPSR = spsr;
}

void spi_transfer_async(spi_transfer_t *xfer)
{
    if (!xfer->len)
        return;

    spi_async_xfer = xfer;
    spi_async_xfer_complete = 0;
    spi_async_xfer_idx = 0;

    if (xfer->out)
        SPDR = xfer->out[0];
    else
        SPDR = 0xFF;

    SPCR |= _BV(SPIE);
}

uint8_t spi_async_transfer_complete(void)
{
    return spi_async_xfer_complete;
}

void spi_transfer(spi_transfer_t *xfer)
{
    uint8_t c;
    uint16_t i;

    for (i=0;i<xfer->len;++i) {
        if (xfer->out)
            SPDR = xfer->out[i];
        else
            SPDR = 0xFF;

        while (!(SPSR & _BV(SPIF)));

        c = SPDR; // clean SPIF

        if (xfer->in)
            xfer->in[i] = c;
    }
}

void spi_write(uint8_t out)
{
    uint8_t c;

    SPDR = out;

    while (!(SPSR & _BV(SPIF)));

    c = SPDR; // clean SPIF
    (void)c;
}

void spi_read(uint8_t *in)
{
    SPDR = 0xFF;

    while (!(SPSR & _BV(SPIF)));

    *in = SPDR;
}
