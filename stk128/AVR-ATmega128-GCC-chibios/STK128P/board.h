/*
    ChibiOS/RT - Copyright (C) 2006,2007,2008,2009,2010,
                 2011,2012 Giovanni Di Sirio.

    This file is part of ChibiOS/RT.

    ChibiOS/RT is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    ChibiOS/RT is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _BOARD_H_
#define _BOARD_H_

/*
 * Setup for the MEGA128D proto board.
 */

/*
 * Board identifier.
 */
#define BOARD_NAME "STK128+"

#if defined(IV9)
/*        I7   I6   I5   I4   I3   I2   I1   I0
 *        OUT  OUT  OUT  OUT  OUT  OUT  OUT  OUT
 * DDRA   1    1    1    1    1    1    1    1
 *        VAL  VAL  VAL  VAL  VAL  VAL  VAL  VAL
 * PORTA  1    1    1    1    1    1    1    1
 */
#define VAL_DDRA  0xFF
#define VAL_PORTA 0xFF
#elif defined(CY7C4XX)
/*        D7   D6   D5   D4   D3   D2   D1   D0
 *        OUT  OUT  OUT  OUT  OUT  OUT  OUT  OUT
 * DDRA   1    1    1    1    1    1    1    1
 *        VAL  VAL  VAL  VAL  VAL  VAL  VAL  VAL
 * PORTA  1    1    1    1    1    1    1    1
 */
#define VAL_DDRA  0xFF
#define VAL_PORTA 0xFF
#else
/*        PA7  PA6  PA5  PA4  PA3  PA2  PA1  PA0
 *        IN   IN   IN   IN   IN   IN   IN   IN
 * DDRA   0    0    0    0    0    0    0    0
 *        PU   PU   PU   PU   PU   PU   PU   PU
 * PORTA  1    1    1    1    1    1    1    1
 */
#define VAL_DDRA  0x00
#define VAL_PORTA 0xFF
#endif

/*        OC2  PB6  PB5  PB4  K4   K3   K2   K1
 *        OUT  IN   IN   IN   IN   IN   IN   IN
 * DDRB   1    0    0    0    0    0    0    0
 *        VAL  PU   PU   PU   PU   PU   PU   PU
 * PORTB  0    1    1    1    1    1    1    1
 */
#define VAL_DDRB  0x80
#define VAL_PORTB 0x7F

/*        L7   L6   L5   L4   L3   L2   L1   L0
 *        OUT  OUT  OUT  OUT  OUT  OUT  OUT  OUT
 * DDRC   1    1    1    1    1    1    1    1
 *        VAL  VAL  VAL  VAL  VAL  VAL  VAL  VAL
 * PORTC  1    1    1    1    1    1    1    1
 */
#define VAL_DDRC  0xFF
#define VAL_PORTC 0xFF

/*        PD7  PD6  PD5  PD4  TXD  RXD  IT1  IT0
 *        IN   IN   IN   IN   OUT  IN   x    x
 * DDRD   0    0    0    0    1    0    0    0
 *        PU   PU   PU   PU   VAL  HiZ  x    x
 * PORTD  1    1    1    1    1    0    0    0
 */
#define VAL_DDRD  0x08
#define VAL_PORTD 0xF8


/*        PE7  PE6  PE5  PE4  PE3  PE2  PE1  PE0
 *        IN   IN   IN   IN   IN   IN   IN   IN
 * DDRE   0    0    0    0    0    0    0    0
 *        PU   PU   PU   PU   PU   PU   PU   PU
 * PORTE  1    1    1    1    1    1    1    1
 */
#define VAL_DDRE  0x00
#define VAL_PORTE 0xFF

#if defined(CY7C4XX)
/*        PF7  PF6  PF5  /MR  FR   /R   /W   /FF
 *        IN   IN   IN   OUT  OUT  IN   OUT  IN
 * DDRF   0    0    0    1    1    0    1    0
 *        PU   PU   PU   VAL  VAL  PU   VAL  PU
 * PORTF  1    1    1    0    0    0    1    1
 */
#define VAL_DDRF  0x1A
#define VAL_PORTF 0xE3
#else
/*        PF7  PF6  PF5  PF4  PF3  PF2  PF1  PF0
 *        IN   IN   IN   IN   IN   IN   IN   IN
 * DDRF   0    0    0    0    0    0    0    0
 *        PU   PU   PU   PU   PU   PU   PU   PU
 * PORTF  1    1    1    1    1    1    1    1
 */
#define VAL_DDRF  0x00
#define VAL_PORTF 0xFF
#endif

/*        x    x    x    x    PG3  PG2  PG1  PG0
 *        x    x    x    x    IN   IN   IN   IN
 * DDRG   0    0    0    0    0    0    0    0
 *        x    x    x    x    PU   PU   PU   PU
 * PORTG  0    0    0    0    1    1    1    1
 */
#define VAL_DDRG  0x00
#define VAL_PORTG 0x0F

#define BTN_K1                   (1<<0)
#define BTN_K2                   (1<<1)
#define BTN_K3                   (1<<2)
#define BTN_K4                   (1<<3)

#define BTN_K1_VAL               (PINB&BTN_K1)
#define BTN_K2_VAL               (PINB&BTN_K2)
#define BTN_K3_VAL               (PINB&BTN_K3)
#define BTN_K4_VAL               (PINB&BTN_K4)

#define BTN_K1_PRESSED           (!BTN_K1_VAL)
#define BTN_K2_PRESSED           (!BTN_K2_VAL)
#define BTN_K3_PRESSED           (!BTN_K3_VAL)
#define BTN_K4_PRESSED           (!BTN_K4_VAL)

#define LED1                     (1<<0)
#define LED2                     (1<<1)
#define LED3                     (1<<2)
#define LED4                     (1<<3)
#define LED5                     (1<<4)
#define LED6                     (1<<5)
#define LED7                     (1<<7)
#define LED8                     (1<<7)

#define LED_ON(led) do {                        \
        PORTC &= ~(1 << (led));                 \
    } while (0)

#define LED_OFF(led) do {                       \
        PORTC |= 1 << (led);                    \
    } while (0)

#define LED_TOGGLE(led) do {                    \
        PORTC ^= 1 << (led);                    \
        /* PINC = 1 << (led); */                \
    } while (0)

#define SET_LED_MASK(mask) do {                 \
        PORTC = ~(mask);                        \
    } while (0)

#if defined(IV9)
#define IV9_PORT PORTA
#define IV9_DIRECT_CONNECTION
#endif

#if defined(CY7C4XX)
#define CY7C4XX_PORT PORTA

#define CY7C4XX_MASTER_RESET_N_PORT    PORTF
#define CY7C4XX_MASTER_RESET_N_BIT     4

#define CY7C4XX_FIFO_READY_PORT        PORTF
#define CY7C4XX_FIFO_READY_BIT         3

#define CY7C4XX_READ_N_PORT            PINF
#define CY7C4XX_READ_N_BIT             2

#define CY7C4XX_WRITE_N_PORT           PORTF
#define CY7C4XX_WRITE_N_BIT            1

#define CY7C4XX_FULL_N_PORT            PINF
#define CY7C4XX_FULL_N_BIT             0
#endif

#if defined(CY7C4XX) && defined(IV9)
#if (CY7C4XX_PORT == IV9_PORT)
#error "CY7C4XX_PORT == IV9_PORT"
#endif
#endif

#if !defined(_FROM_ASM_)
#ifdef __cplusplus
extern "C" {
#endif
  void boardInit(void);
#ifdef __cplusplus
}
#endif
#endif /* _FROM_ASM_ */

#endif /* _BOARD_H_ */
