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

#ifndef _CY7C4XX_9403A_H_
#define _CY7C4XX_9403A_H_

#if defined(CY7C4XX_9403A)
void cy7c4xx_9403a_init(void);

/* pushes data to fifo
   fragments if needed
 */
int fifo_cy7c4xx_push(unsigned char *s, int len);

/* pulls data from fifo */
int fifo_9403a_pull_one(unsigned char *c);
#else
static inline void cy7c4xx_9403a_init(void) {}
#endif

#endif /* _CY7C4XX_9403A_H_ */
