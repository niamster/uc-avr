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
#include "cy7c4xx.h"

#if defined(CY7C4XX)
#define CY7C4XX_MAX_CMD_LEN 0xFF

void cy7c4xx_init(void)
{
}

static inline int cy7c4xx_push_one(unsigned char c)
{
    while (!(CY7C4XX_REMOTE_CPU_READY_PORT & (1<<CY7C4XX_REMOTE_CPU_READY_BIT))
            && !(CY7C4XX_FULL_N_PORT & (1<<CY7C4XX_FULL_N_BIT)));

    CY7C4XX_PORT = c;
    CY7C4XX_WRITE_N_PORT &= ~(1 << CY7C4XX_WRITE_N_BIT);
    /* asm volatile ("nop"); */
    CY7C4XX_WRITE_N_PORT |= 1 << CY7C4XX_WRITE_N_BIT;
    /* asm volatile ("nop"); */

    return 0;
}

int cy7c4xx_push(unsigned char *s, int len)
{
    int blocks = len/CY7C4XX_MAX_CMD_LEN;
    int remainder = len%CY7C4XX_MAX_CMD_LEN;
    int i;

    while (blocks) {
        cy7c4xx_push_one(CY7C4XX_MAX_CMD_LEN);
        for (i=0;i<CY7C4XX_MAX_CMD_LEN;++i, ++s)
            cy7c4xx_push_one(*s);
        --blocks;
    }

    if (remainder) {
        cy7c4xx_push_one(remainder);
        for (i=0;i<remainder;++i, ++s)
            cy7c4xx_push_one(*s);
    }

    return 0;
}
#endif
