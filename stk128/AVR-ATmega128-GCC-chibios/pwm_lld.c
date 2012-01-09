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

/**
 * @file    templates/pwm_lld.c
 * @brief   PWM Driver subsystem low level driver source template.
 *
 * @addtogroup PWM
 * @{
 */

#include "ch.h"
#include "hal.h"
#include "chprintf.h"

#if HAL_USE_PWM || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

PWMDriver PWMD2;

/*===========================================================================*/
/* Driver local variables.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

/**
 * @brief   TIMER2 interrupt handler.
 *
 * @isr
 */
CH_IRQ_HANDLER(TIMER2_COMP_vect) {
  CH_IRQ_PROLOGUE();

  /* if (PWMD2.config->channels[0].callback) */
  PWMD2.config->channels[0].callback(&PWMD2);

  CH_IRQ_EPILOGUE();
}

/**
 * @brief   TIMER2 interrupt handler.
 *
 * @isr
 */
CH_IRQ_HANDLER(TIMER2_OVF_vect) {
  CH_IRQ_PROLOGUE();

  /* if (PWMD2.config->callback) */
  PWMD2.config->callback(&PWMD2);

  CH_IRQ_EPILOGUE();
}

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level PWM driver initialization.
 *
 * @notapi
 */
void pwm_lld_init(void) {
  /* Driver initialization.*/
  pwmObjectInit(&PWMD2);
}

/**
 * @brief   Configures and activates the PWM peripheral.
 *
 * @param[in] pwmp      pointer to the @p PWMDriver object
 *
 * @notapi
 */
void pwm_lld_start(PWMDriver *pwmp) {

  if (pwmp->state == PWM_STOP) {
      if (pwmp == &PWMD2) {
          const unsigned long factor = 510;
          // PWM, Phase correct, 8-Bit mode
          TCCR2 = (0 << WGM21) | (1 << WGM20);

          /* Clock select */
          if (PWMD2.config->frequency == 0) {
              /* do nothing here */
          } else {
              switch ((F_CPU / factor) / PWMD2.config->frequency) {
                  case 1:
                      TCCR2 |= 1;
                      break;
                  case 8:
                      TCCR2 |= 2;
                      break;
                  case 32:
                      TCCR2 |= 3;
                      break;
                  case 64:
                      TCCR2 |= 4;
                      break;
                  case 128:
                      TCCR2 |= 5;
                      break;
                  case 256:
                      TCCR2 |= 6;
                      break;
                  case 1024:
                      TCCR2 |= 7;
                      break;
                  default:
                      chprintf((BaseChannel *)&SD2, "Unsupported PWM frequency: %u(div=%d)\r\n",
                              PWMD2.config->frequency,
                              (F_CPU / factor) / PWMD2.config->frequency);
                      chprintf((BaseChannel *)&SD2, "Supported PWM frequencies: %lu %lu %lu %lu %lu %lu %lu\r\n",
                              (F_CPU / factor) /    1,
                              (F_CPU / factor) /    8,
                              (F_CPU / factor) /   32,
                              (F_CPU / factor) /   64,
                              (F_CPU / factor) /  128,
                              (F_CPU / factor) /  256,
                              (F_CPU / factor) / 1024);
              }
              TCCR2 |= ((F_CPU / factor) / PWMD2.config->frequency) & 0x7;
          }

          OCR2 = PWMD2.period;

          TCNT2 = 0; /* Reset counter.   */

          if (PWMD2.config->channels[0].callback) {
              TIFR = (1 << OCF2);  /* Reset pending.   */
              TIMSK = (1 << OCIE2); /* IRQ on compare.  */
          }

          if (PWMD2.config->callback) {
              TIFR = (1 << TOV2);  /* Reset pending.   */
              TIMSK = (1 << TOIE2); /* IRQ on compare.  */
          }
      }
    /* Clock activation.*/
  }
  /* Configuration.*/
}

/**
 * @brief   Deactivates the PWM peripheral.
 *
 * @param[in] pwmp      pointer to the @p PWMDriver object
 *
 * @notapi
 */
void pwm_lld_stop(PWMDriver *pwmp) {
    TCCR2 = 0x0;
}

/**
 * @brief   Changes the period the PWM peripheral.
 * @details This function changes the period of a PWM unit that has already
 *          been activated using @p pwmStart().
 * @pre     The PWM unit must have been activated using @p pwmStart().
 * @post    The PWM unit period is changed to the new value.
 * @note    The function has effect at the next cycle start.
 * @note    If a period is specified that is shorter than the pulse width
 *          programmed in one of the channels then the behavior is not
 *          guaranteed.
 *
 * @param[in] pwmp      pointer to a @p PWMDriver object
 * @param[in] period    new cycle time in ticks
 *
 * @notapi
 */
void pwm_lld_change_period(PWMDriver *pwmp, pwmcnt_t period) {
    if (pwmp == &PWMD2) {
        pwmp->period = period;

        OCR2 = period;
    }
}

/**
 * @brief   Enables a PWM channel.
 * @pre     The PWM unit must have been activated using @p pwmStart().
 * @post    The channel is active using the specified configuration.
 * @note    Depending on the hardware implementation this function has
 *          effect starting on the next cycle (recommended implementation)
 *          or immediately (fallback implementation).
 *
 * @param[in] pwmp      pointer to a @p PWMDriver object
 * @param[in] channel   PWM channel identifier (0...PWM_CHANNELS-1)
 * @param[in] width     PWM pulse width as clock pulses number
 *
 * @notapi
 */
void pwm_lld_enable_channel(PWMDriver *pwmp,
                            pwmchannel_t channel,
                            pwmcnt_t width) {
    if (PWMD2.config->channels[0].mode == PWM_OUTPUT_ACTIVE_LOW) {
        // Clear on compare match when up-counting, set on compare match when down-counting
        TCCR2 |= (1 << COM21) | (0 << COM20);
    } else if (PWMD2.config->channels[0].mode == PWM_OUTPUT_ACTIVE_HIGH) {
        // Set on compare match when up-counting, cleared on compare match when down-counting
        TCCR2 |= (1 << COM21) | (1 << COM20);
    }/*  else if (PWMD2.config->channels[0].mode == PWM_OUTPUT_DISABLED) { */
    /*     // Output disconnected */
    /*     TCCR2 |= (0 << COM21) | (0 << COM20); */
    /* } */
}

/**
 * @brief   Disables a PWM channel.
 * @pre     The PWM unit must have been activated using @p pwmStart().
 * @post    The channel is disabled and its output line returned to the
 *          idle state.
 * @note    Depending on the hardware implementation this function has
 *          effect starting on the next cycle (recommended implementation)
 *          or immediately (fallback implementation).
 *
 * @param[in] pwmp      pointer to a @p PWMDriver object
 * @param[in] channel   PWM channel identifier (0...PWM_CHANNELS-1)
 *
 * @notapi
 */
void pwm_lld_disable_channel(PWMDriver *pwmp, pwmchannel_t channel) {
    TCCR2 &= ~((1 << COM21) | (1 << COM20));
}

#endif /* HAL_USE_PWM */

/** @} */
