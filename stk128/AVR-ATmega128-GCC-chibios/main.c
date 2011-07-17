/*
    ChibiOS/RT - Copyright (C) 2006,2007,2008,2009,2010 Giovanni Di Sirio.

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
#include "evtimer.h"

static void print(const char *msgp) {

  while (*msgp)
    chIOPut(&SD2, *msgp++);
}

static void println(const char *msgp) {
    print(msgp);
    chIOPut(&SD2, '\r');
    chIOPut(&SD2, '\n');
}

void hello(void) {
  println("");
  println("*** ChibiOS/RT test suite");
  println("***");
  print("*** Kernel:       ");
  println(CH_KERNEL_VERSION);
#ifdef CH_COMPILER_NAME
  print("*** Compiler:     ");
  println(CH_COMPILER_NAME);
#endif
  print("*** Architecture: ");
  println(CH_ARCHITECTURE_NAME);
#ifdef CH_CORE_VARIANT_NAME
  print("*** Core Variant: ");
  println(CH_CORE_VARIANT_NAME);
#endif
#ifdef CH_PORT_INFO
  print("*** Port Info:    ");
  println(CH_PORT_INFO);
#endif
#ifdef PLATFORM_NAME
  print("*** Platform:     ");
  println(PLATFORM_NAME);
#endif
#ifdef BOARD_NAME
  print("*** Test Board:   ");
  println(BOARD_NAME);
#endif
  println("");
}

static WORKING_AREA(waThread1, 32);
static msg_t Thread1(void *arg) {

  while (TRUE) {
    chThdSleepMilliseconds(1000);
    LED_TOGGLE(0);
  }

  return 0;
}

static void TimerHandler(eventid_t id) {
  /* msg_t TestThread(void *p); */

  if (BTN_K1_PRESSED)
      hello();
    if (BTN_K2_PRESSED)
        LED_TOGGLE(1);
}

int main(int argc, char **argv) {
  static EvTimer evt;
  static evhandler_t handlers[1] = {
    TimerHandler
  };
  static EventListener el0;

  /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
  halInit();
  chSysInit();

  /*
   * Activates the serial driver 2 using the driver default configuration.
   */
  sdStart(&SD2, NULL);

  /*
   * Event Timer initialization.
   */
  evtInit(&evt, MS2ST(500));            /* Initializes an event timer object.   */
  evtStart(&evt);                       /* Starts the event timer.              */
  chEvtRegister(&evt.et_es, &el0, 0);   /* Registers on the timer event source. */

  /*
   * Starts thread.
   */
  chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);

  while(TRUE)
    chEvtDispatch(handlers, chEvtWaitOne(ALL_EVENTS));

  return 0;
}
