#ifndef _SPI_H_
#define _SPI_H_

typedef enum spi_mode {
    SPI_MODE0 = 0x00,
    SPI_MODE1 = 0x04,
    SPI_MODE2 = 0x08,
    SPI_MODE3 = 0x0C,
} spi_mode_t;

typedef enum spi_bit_order {
    SPI_BIT_ORDER_LSB_FIRST,
    SPI_BIT_ORDER_MSB_FIRST,
} spi_bit_order_t;

typedef enum spi_clk_div {      /* Fosc divider */
    SPI_CLK_DIV2   = 0b100,
    SPI_CLK_DIV4   = 0b000,
    SPI_CLK_DIV8   = 0b101,
    SPI_CLK_DIV16  = 0b001,
    SPI_CLK_DIV32  = 0b110,
    SPI_CLK_DIV64  = 0b010,
    SPI_CLK_DIV128 = 0b011,
} spi_clk_div_t;

void spi_enable(void);
void spi_disable(void);

void spi_interrupt_enable(void);
void spi_interrupt_disable(void);

static inline spi_clk_div_t
spi_speed_to_clkdiv(uint32_t speed)
{
    if (F_CPU/speed <= 2)
        return SPI_CLK_DIV2;
    else if (F_CPU/speed <= 4)
        return SPI_CLK_DIV4;
    else if (F_CPU/speed <= 8)
        return SPI_CLK_DIV8;
    else if (F_CPU/speed <= 16)
        return SPI_CLK_DIV16;
    else if (F_CPU/speed <= 32)
        return SPI_CLK_DIV32;
    else if (F_CPU/speed <= 64)
        return SPI_CLK_DIV64;
    else
        return SPI_CLK_DIV128;
}

void spi_setup(spi_mode_t mode, spi_clk_div_t clk_div, spi_bit_order_t bit_order);

typedef struct spi_transfer {
    uint8_t *in;
    const uint8_t *out;
    uint16_t len;
} spi_transfer_t;

void spi_transfer(spi_transfer_t *xfer);

void spi_write(uint8_t out);
void spi_read(uint8_t *in);

#endif
