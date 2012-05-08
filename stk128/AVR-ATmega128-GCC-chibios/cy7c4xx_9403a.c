/*
    ChibiOS/RT - Copyright (C) 2006,2007,2008,2009,2010,
                 2011 Giovanni Di Sirio.

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

#include "ch.h"
#include "hal.h"
#include "board.h"
#include "cy7c4xx_9403a.h"

#if defined(CY7C4XX_9403A)
#define FIFO_CY7C4XX_MAX_MSG_LEN 0x40

void cy7c4xx_9403a_init(void)
{
    /* CY7C4XX reset cycle is about 25nS */
    /* CY7C4XX recover from reset cycle is about 10nS */

    /* FIFO_CY7C4XX_WRITE_N_PORT |= 1 << FIFO_CY7C4XX_WRITE_N_BIT; */
    /* FIFOS_MASTER_RESET_N_PORT &= ~(1 << FIFOS_MASTER_RESET_N_BIT); */
    /* FIFO_9403_TOP_PORT &= ~(1 << FIFO_9403_TOP_BIT); */

    while (!(FIFO_CY7C4XX_READ_N_PORT & (1<<FIFO_CY7C4XX_READ_N_BIT)));

    FIFOS_MASTER_RESET_N_PORT |= 1 << FIFOS_MASTER_RESET_N_BIT;
    FIFO_9403_TOP_PORT |= 1 << FIFO_9403_TOP_BIT;

    FIFOS_READY_PORT |= 1 << FIFOS_READY_BIT;
}

static inline int fifo_cy7c4xx_push_one(unsigned char c, int block)
{
    if (block)
        while (!(FIFO_CY7C4XX_FULL_N_PORT & (1<<FIFO_CY7C4XX_FULL_N_BIT)));
    else if (!(FIFO_CY7C4XX_FULL_N_PORT & (1<<FIFO_CY7C4XX_FULL_N_BIT)))
        return -1;

    FIFO_CY7C4XX_PORT = c;
    FIFO_CY7C4XX_WRITE_N_PORT &= ~(1 << FIFO_CY7C4XX_WRITE_N_BIT);
    FIFO_CY7C4XX_WRITE_N_PORT |= 1 << FIFO_CY7C4XX_WRITE_N_BIT;

    return 0;
}

int fifo_cy7c4xx_push(unsigned char *s, int len)
{
    int blocks = len/FIFO_CY7C4XX_MAX_MSG_LEN;
    int remainder = len%FIFO_CY7C4XX_MAX_MSG_LEN;
    int i;

    while (blocks) {
        fifo_cy7c4xx_push_one(FIFO_CY7C4XX_MAX_MSG_LEN, 1);
        for (i=0;i<FIFO_CY7C4XX_MAX_MSG_LEN;++i, ++s)
            fifo_cy7c4xx_push_one(*s, 1);
        --blocks;
    }

    if (remainder) {
        fifo_cy7c4xx_push_one(remainder, 1);
        for (i=0;i<remainder;++i, ++s)
            fifo_cy7c4xx_push_one(*s, 1);
    }

    return 0;
}


int fifo_9403a_pull_one(unsigned char *c)
{
    if (!(FIFO_9403_ORE_N_PORT & (1<<FIFO_9403_ORE_N_BIT)))
        return -1;

    *c = (FIFO_9403_PORT & FIFO_9403_PORT_MASK) >> FIFO_9403_PORT_SHIFT;

    FIFO_9403_TOP_PORT &= ~(1 << FIFO_9403_TOP_BIT);
    FIFO_9403_TOP_PORT |= 1 << FIFO_9403_TOP_BIT;

    while (!(FIFO_9403_ORE_N_PORT & (1<<FIFO_9403_ORE_N_BIT)));

    *c |= ((FIFO_9403_PORT & FIFO_9403_PORT_MASK) >> FIFO_9403_PORT_SHIFT) << 4;

    FIFO_9403_TOP_PORT &= ~(1 << FIFO_9403_TOP_BIT);
    FIFO_9403_TOP_PORT |= 1 << FIFO_9403_TOP_BIT;

    return 0;
}
#endif
