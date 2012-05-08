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

#ifndef _IV9_H_
#define _IV9_H_

#if defined(IV9)
enum iv9_symbol {
    IV9_SYMBOL_0,
    IV9_SYMBOL_1,
    IV9_SYMBOL_2,
    IV9_SYMBOL_3,
    IV9_SYMBOL_4,
    IV9_SYMBOL_5,
    IV9_SYMBOL_6,
    IV9_SYMBOL_7,
    IV9_SYMBOL_8,
    IV9_SYMBOL_9,
    IV9_SYMBOL_COMA,
};

void iv9_init(void);

void iv9_show(enum iv9_symbol sym);
#else
static inline void iv9_init(void) {}
#endif

#endif /* _IV9_H_ */
