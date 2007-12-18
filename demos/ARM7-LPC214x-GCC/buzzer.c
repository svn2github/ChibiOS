/*
    ChibiOS/RT - Copyright (C) 2006-2007 Giovanni Di Sirio.

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

/*
 * Buzzer driver for Olimex LPC-P2148.
 * Uses the timer 1 for wave generation and a Virtual Timer for the sound
 * duration.
 * The driver also generates an event when the sound is done and the buzzer
 * goes silent.
 */
#include <ch.h>

#include "lpc214x.h"
#include "board.h"
#include "buzzer.h"

EventSource BuzzerSilentEventSource;

#define StartCounter(t) ((t)->TC_EMR = 0xF1, (t)->TC_TCR = 1)
#define StopCounter(t)  ((t)->TC_EMR = 0, (t)->TC_TCR = 2)

void InitBuzzer(void) {

  chEvtInit(&BuzzerSilentEventSource);

  /*
   * Switches P0.12 and P0.13 to MAT1.0 and MAT1.1 functions.
   * Enables Timer1 clock.
   */
  PINSEL0 &= 0xF0FFFFFF;
  PINSEL0 |= 0x0A000000;
  PCONP = (PCONP & PCALL) | PCTIM1;

  /*
   * Timer setup.
   */
  TC *tc = T1Base;
  StopCounter(tc);
  tc->TC_CTCR = 0;                      // Clock source is PCLK.
  tc->TC_PR   = 0;                        // Prescaler disabled.
  tc->TC_MCR  = 2;                       // Clear TC on match MR0.
}

static void stop(void *p) {
  TC *tc = T1Base;

  StopCounter(tc);
  chEvtSendI(&BuzzerSilentEventSource);
}

void PlaySound(int freq, t_time duration) {
  static VirtualTimer bvt;
  TC *tc = T1Base;

  chSysLock();

  if (chVTIsArmedI(&bvt)) {              // If a sound is already being played
    chVTResetI(&bvt);                   // then aborts it.
    StopCounter(tc);
  }

  tc->TC_MR0 = tc->TC_MR1 = (PCLK / (freq * 2));
  StartCounter(tc);
  chVTSetI(&bvt, duration, stop, NULL);

  chSysUnlock();
}

void PlaySoundWait(int freq, t_time duration) {
  TC *tc = T1Base;

  StopCounter(tc);
  tc->TC_MR0 = tc->TC_MR1 = (PCLK / (freq * 2));
  StartCounter(tc);
  chThdSleep(duration);
  StopCounter(tc);
}
