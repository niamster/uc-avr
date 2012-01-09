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
#include "iv9.h"

static unsigned char iv9_symbol_table[] = {
    [IV9_SYMBOL_0] = ~0xDE,
    [IV9_SYMBOL_1] = ~0x06,
    [IV9_SYMBOL_2] = ~0xEA,
    [IV9_SYMBOL_3] = ~0x6E,
    [IV9_SYMBOL_4] = ~0x36,
    [IV9_SYMBOL_5] = ~0x7C,
    [IV9_SYMBOL_6] = ~0xFC,
    [IV9_SYMBOL_7] = ~0x0E,
    [IV9_SYMBOL_8] = ~0xFE,
    [IV9_SYMBOL_9] = ~0x7E,
    [IV9_SYMBOL_COMA] = ~0x01,
};

void iv9_init(void)
{
}

void iv9_show(enum iv9_symbol sym)
{
#if defined(IV9_PORT)
    IV9_PORT = iv9_symbol_table[sym];
#if defined(IV9_DIRECT_CONNECTION)
#else
#error IV9 indirect access not supported
#endif
#endif
}
