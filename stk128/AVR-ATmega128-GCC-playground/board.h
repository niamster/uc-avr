#ifndef _BOARD_H_
#define _BOARD_H_

/*        PA7  PA6  PA5  PA4  PA3  PA2  PA1  PA0
 *        IN   IN   IN   IN   IN   IN   IN   IN
 * DDRA   0    0    0    0    0    0    0    0
 *        PU   PU   PU   PU   PU   PU   PU   PU
 * PORTA  1    1    1    1    1    1    1    1
 */
#define VAL_DDRA  0x00
#define VAL_PORTA 0xFF

/*        PB7  PB6  PB5  PB4  K4   K3   K2  K1
 *        IN   IN   IN   IN   IN   IN   IN   IN
 * DDRB   0    0    0    0    0    0    0    0
 *        PU   PU   PU   PU   PU   PU   PU   PU
 * PORTB  1    1    1    1    1    1    1    1
 */
#define VAL_DDRB  0x00
#define VAL_PORTB 0xFF

/*        PC7  PC6  PC5  PC4  K4   K3   K2  K1
 *        OUT  OUT  OUT  OUT  OUT  OUT  OUT  IN
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

/*        PF7  PF6  PF5  PF4  PF3  PF2  PF1  PF0
 *        IN   IN   IN   IN   IN   IN   IN   IN
 * DDRF   0    0    0    0    0    0    0    0
 *        PU   PU   PU   PU   PU   PU   PU   PU
 * PORTF  1    1    1    1    1    1    1    1
 */
#define VAL_DDRF  0x00
#define VAL_PORTF 0xFF

/*        x    x    x    x    PG3  PG2  PG1  PG0
 *        x    x    x    x    IN   IN   IN   IN
 * DDRG   0    0    0    0    0    0    0    0
 *        x    x    x    x    PU   PU   PU   PU
 * PORTG  0    0    0    0    1    1    1    1
 */
#define VAL_DDRG  0x00
#define VAL_PORTG 0x0F
#endif /* _BOARD_H_ */
