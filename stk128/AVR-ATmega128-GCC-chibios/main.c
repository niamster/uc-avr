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

#include <stdlib.h>
#include <string.h>

#include "ch.h"
#include "hal.h"
#include "evtimer.h"
#include "shell.h"
#include "chprintf.h"
#include "chheap.h"

#include "iv9.h"
#include "cy7c4xx.h"

enum g_event {
    G_EVENT_GEN_TMR,

    G_EVENT_QTY
};

static EvTimer gEvt[G_EVENT_QTY];
static EventListener gEl[G_EVENT_QTY];

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

#if 0
/*
 * PWM callback on counter reset.
 */
static void pwmCntRst(PWMDriver *pwmp) {

  (void)pwmp;

  chSysLockFromIsr();

  chSysUnlockFromIsr();
}

/*
 * PWM callback on compare event.
 */
static void pwmChCmp(PWMDriver *pwmp) {

  (void)pwmp;

  chSysLockFromIsr();

  chSysUnlockFromIsr();
}
#endif

/*
 * PWM configuration structure.
 */
static PWMConfig pwmCfg = {
  14456,                                    /* 14KHz PWM clock frequency.   */
  0xFF,                                     /* PWM period (full).           */
  NULL,
  {
    {PWM_OUTPUT_ACTIVE_HIGH, NULL},
  },
};

#if defined(IV9)
static WORKING_AREA(waIv9CntThread, 128);
static msg_t iv9CntThread(void *arg) {
    enum iv9_symbol sym = IV9_SYMBOL_0;

    uint8_t period = 0;
    const uint8_t floor = 0;
    const uint8_t ceil = 200;

    /*
     * Initializes the PWM driver 2.
     */
    pwmStart(&PWMD2, &pwmCfg);
    pwmEnableChannel(&PWMD2, 0, 0);

    while (TRUE) {
        chThdSleepMilliseconds(2000);

        for (;period < ceil; ++period) {
            pwmChangePeriod(&PWMD2, period);
            chThdSleepMilliseconds(10);
        }

        iv9_show(sym);
        if (++sym > IV9_SYMBOL_9)
            sym = IV9_SYMBOL_0;

        for (;period > floor; --period) {
            pwmChangePeriod(&PWMD2, period);
            chThdSleepMilliseconds(10);
        }
    }

    return 0;
}
#endif

static void TimerEvtHandler(eventid_t id) {
    if (id != 0)
        return;

    if (BTN_K1_PRESSED)
        hello();
    if (BTN_K2_PRESSED)
        LED_TOGGLE(1);
}

#define SHELL_WA_SIZE   THD_WA_SIZE(1024)

static void cmd_mem(BaseChannel *chp, int argc, char *argv[]) {
#if CH_USE_HEAP
    size_t n, size;
#endif
  extern uint8_t __heap_base__[];
  extern uint8_t __heap_end__[];
  extern uint8_t __data_start[];
  extern uint8_t __data_end[];
  extern uint8_t __bss_start[];
  extern uint8_t __bss_end[];

    (void)argv;
#if CH_USE_HEAP
    if (argc > 0) {
        chprintf(chp, "Usage: mem\r\n");
        return;
    }
    n = chHeapStatus(NULL, &size);
    chprintf(chp, "heap fragments   : %u\r\n", n);
    chprintf(chp, "heap free total  : %u bytes\r\n", size);
#else
    chprintf(chp, "Heap was not built in\r\n");
#endif
#if CH_USE_MEMCORE
    chprintf(chp, "core free memory : %u bytes\r\n", chCoreStatus());
#endif
    chprintf(chp, "data: %.8x:%.8x(%d bytes)\r\n",
            (uint16_t)(uint8_t *)__data_start, (uint16_t)(uint8_t *)__data_end, (uint16_t)(uint8_t *)__data_end - (uint16_t)(uint8_t *)__data_start);
    chprintf(chp, "bss : %.8x:%.8x(%d bytes)\r\n",
            (uint16_t)(uint8_t *)__bss_start, (uint16_t)(uint8_t *)__bss_end, (uint16_t)(uint8_t *)__bss_end - (uint16_t)(uint8_t *)__bss_start);
    chprintf(chp, "heap: %.8x:%.8x(%d bytes)\r\n",
            (uint16_t)(uint8_t *)__heap_base__, (uint16_t)(uint8_t *)__heap_end__, (uint16_t)(uint8_t *)__heap_end__ - (uint16_t)(uint8_t *)__heap_base__);

    chprintf(chp, "total: %d bytes\r\n",
            (uint16_t)(uint8_t *)__data_end - (uint16_t)(uint8_t *)__data_start
            + (uint16_t)(uint8_t *)__heap_end__ - (uint16_t)(uint8_t *)__heap_base__
            + (uint16_t)(uint8_t *)__bss_end - (uint16_t)(uint8_t *)__bss_start);
}

static void cmd_threads(BaseChannel *chp, int argc, char *argv[]) {
#if CH_USE_REGISTRY
    static const char *states[] = {
        "READY",
        "CURRENT",
        "SUSPENDED",
        "WTSEM",
        "WTMTX",
        "WTCOND",
        "SLEEPING",
        "WTEXIT",
        "WTOREVT",
        "WTANDEVT",
        "SNDMSGQ",
        "SNDMSG",
        "WTMSG",
        "WTQUEUE",
        "FINAL"
    };
    Thread *tp;
#endif

    (void)argv;
#if CH_USE_REGISTRY
    if (argc > 0) {
        chprintf(chp, "Usage: threads\r\n");
        return;
    }
    chprintf(chp, "    addr    stack prio refs     state time\r\n");
    tp = chRegFirstThread();
    do {
#if CH_DBG_THREADS_PROFILING
        systime_t p_time = tp->p_time;
#else
        systime_t p_time = 0;
#endif
        chprintf(chp, "%.8x %.8x %4lu %4lu %9s %lu\r\n",
                (uint16_t)tp, (uint16_t)tp->p_ctx.sp,
                (uint32_t)tp->p_prio, (uint32_t)(tp->p_refs - 1),
                states[tp->p_state], (uint32_t)p_time);
        tp = chRegNextThread(tp);
    } while (tp != NULL);
#else
    chprintf(chp, "Registry was not built in\r\n");
#endif
}

static void cmd_pwm(BaseChannel *chp, int argc, char *argv[]) {
    uint32_t frequency;
    pwmcnt_t period;

    if (argc != 2) {
        chprintf(chp, "Usage: pwm [freq] [period]\r\n");
        return;
    }

    frequency = atoi(argv[0]);
    if (frequency)
        pwmCfg.frequency = frequency;
    period = atoi(argv[1]);
    if (period)
        pwmCfg.period = period;
    chprintf(chp, "Setting PWM period=%d, freq=%u\r\n",
            pwmCfg.period, pwmCfg.frequency);

    pwmStop(&PWMD2);
    pwmStart(&PWMD2, &pwmCfg);

    pwmEnableChannel(&PWMD2, 0, 0);
}

#if defined(CY7C4XX)
static void cmd_cy7c4xx(BaseChannel *chp, int argc, char *argv[]) {
    if (argc != 1) {
        chprintf(chp, "Usage: cy7c4xx [string]\r\n");
        return;
    }

    cy7c4xx_push(argv[0], strlen(argv[0]));
}
#endif

static const ShellCommand shCmds[] = {
    {"mem", cmd_mem},
    {"threads", cmd_threads},
    {"pwm", cmd_pwm},
#if defined(CY7C4XX)
    {"cy7c4xx", cmd_cy7c4xx},
#endif
    {NULL, NULL}
};

static const ShellConfig shCfg = {
    (BaseChannel *)&SD2,
    shCmds
};

int main(int argc, char **argv) {
    static evhandler_t handlers[G_EVENT_QTY] = {
        [G_EVENT_GEN_TMR] = TimerEvtHandler,
    };

    Thread *sh = NULL;

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
     * Shell manager initialization.
     */
    shellInit();

    /*
     * Event Timer initialization.
     */
    evtInit(&gEvt[G_EVENT_GEN_TMR], MS2ST(500));              /* Initializes an event timer object.   */
    chEvtRegister(&gEvt[G_EVENT_GEN_TMR].et_es, &gEl[G_EVENT_GEN_TMR], G_EVENT_GEN_TMR);   /* Registers on the timer event source. */

    /*
     * Start threads.
     */
    chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);
#if defined(IV9)
    chThdCreateStatic(waIv9CntThread, sizeof(waIvCntThread), HIGHPRIO, iv9CntThread, NULL);
#endif

    /* Starts the event on generic timer. */
    evtStart(&gEvt[G_EVENT_GEN_TMR]);

    while (TRUE) {
        if (!sh) {
            sh = shellCreate(&shCfg, SHELL_WA_SIZE, NORMALPRIO);
        } else if (chThdTerminated(sh)) {
            chThdRelease(sh);    /* Recovers memory of the previous shell. */
            sh = NULL;           /* Triggers spawning of a new shell.      */
        }

        if (!sh)
            LED_ON(6);
        else
            LED_OFF(6);

        LED_TOGGLE(7);

        chEvtDispatch(handlers, chEvtWaitAny(ALL_EVENTS));
    }

    return 0;
}
