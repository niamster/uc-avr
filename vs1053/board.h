#ifndef _BOARD_H_
#define _BOARD_H_

/* SPI */
#define SPI_CLK_DEFINED
#define SPI_CLK_DDR     DDRB
#define SPI_CLK_PORT    PORTB
#define SPI_CLK_BIT     7

#define SPI_MISO_DEFINED
#define SPI_MISO_DDR    DDRB
#define SPI_MISO_PU     PORTB
#define SPI_MISO_BIT    6

#define SPI_MOSI_DEFINED
#define SPI_MOSI_DDR    DDRB
#define SPI_MOSI_PORT   PORTB
#define SPI_MOSI_BIT    5

#define SPI_SS_DEFINED
#define SPI_SS_DDR      DDRB
#define SPI_SS_PORT     PORTB
#define SPI_SS_BIT      4
/* ---- */


/* VS1053 */
#if !defined(VS1053_XCS_SHARED)
#define VS1053_XDCS_DEFINED
#define VS1053_XDCS_DDR    DDRA
#define VS1053_XDCS_PORT   PORTA
#define VS1053_XDCS_BIT    0
#endif

#define VS1053_DREQ_DEFINED
#define VS1053_DREQ_DDR    DDRA
#define VS1053_DREQ_PORT   PINA
#define VS1053_DREQ_PU     PORTA
#define VS1053_DREQ_BIT    1

#define VS1053_XRES_DEFINED
#define VS1053_XRES_DDR    DDRA
#define VS1053_XRES_PORT   PORTA
#define VS1053_XRES_BIT    2

#define VS1053_XCS_DEFINED
#define VS1053_XCS_DDR     DDRA
#define VS1053_XCS_PORT    PORTA
#define VS1053_XCS_BIT     3
/* ---- */

#endif
