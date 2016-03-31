/*
    ChibiOS - Copyright (C) 2006..2016 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include "hal.h"
#include "ch_test.h"
#include "test_root.h"

/**
 * @page test_sequence_007 [7] Event Sources and Event Flags
 *
 * File: @ref test_sequence_007.c
 *
 * <h2>Description</h2>
 * This module implements the test sequence for the Events subsystem.
 *
 * <h2>Conditions</h2>
 * This sequence is only executed if the following preprocessor condition
 * evaluates to true:
 * - CH_CFG_USE_EVENTS
 * .
 *
 * <h2>Test Cases</h2>
 * - @subpage test_007_001
 * - @subpage test_007_002
 * - @subpage test_007_003
 * - @subpage test_007_004
 * - @subpage test_007_005
 * .
 */

#if (CH_CFG_USE_EVENTS) || defined(__DOXYGEN__)

/****************************************************************************
 * Shared code.
 ****************************************************************************/

static EVENTSOURCE_DECL(es1);
static EVENTSOURCE_DECL(es2);

static void h1(eventid_t id) {(void)id;test_emit_token('A');}
static void h2(eventid_t id) {(void)id;test_emit_token('B');}
static void h3(eventid_t id) {(void)id;test_emit_token('C');}
static ROMCONST evhandler_t evhndl[] = {h1, h2, h3};

static THD_FUNCTION(evt_thread3, p) {

  chThdSleepMilliseconds(50);
  chEvtSignal((thread_t *)p, 1);
}

static THD_FUNCTION(evt_thread4, p) {

  (void)p;
  chEvtBroadcast(&es1);
  chThdSleepMilliseconds(50);
  chEvtBroadcast(&es2);
}

/****************************************************************************
 * Test cases.
 ****************************************************************************/

/**
 * @page test_007_001 [7.1] Events registration
 *
 * <h2>Description</h2>
 * Two event listeners are registered on an event source and then
 * unregistered in the same order.<br> The test expects that the even
 * source has listeners after the registrations and after the first
 * unregistration, then, after the second unegistration, the test
 * expects no more listeners.
 *
 * <h2>Test Steps</h2>
 * - [7.1.1] An Event Source is initialized.
 * - [7.1.2] Two Event Listeners are registered on the Event Source,
 *   the Event Source is tested to have listeners.
 * - [7.1.3] An Event Listener is unregistered, the Event Source must
 *   still have listeners.
 * - [7.1.4] An Event Listener is unregistered, the Event Source must
 *   not have listeners.
 * .
 */

static void test_007_001_execute(void) {
  event_listener_t el1, el2;

  /* [7.1.1] An Event Source is initialized.*/
  test_set_step(1);
  {
    chEvtObjectInit(&es1);
  }

  /* [7.1.2] Two Event Listeners are registered on the Event Source,
     the Event Source is tested to have listeners.*/
  test_set_step(2);
  {
    chEvtRegisterMask(&es1, &el1, 1);
    chEvtRegisterMask(&es1, &el2, 2);
    test_assert_lock(chEvtIsListeningI(&es1), "no listener");
  }

  /* [7.1.3] An Event Listener is unregistered, the Event Source must
     still have listeners.*/
  test_set_step(3);
  {
    chEvtUnregister(&es1, &el1);
    test_assert_lock(chEvtIsListeningI(&es1), "no listener");
  }

  /* [7.1.4] An Event Listener is unregistered, the Event Source must
     not have listeners.*/
  test_set_step(4);
  {
    chEvtUnregister(&es1, &el2);
    test_assert_lock(!chEvtIsListeningI(&es1), "stuck listener");
  }
}

static const testcase_t test_007_001 = {
  "Events registration",
  NULL,
  NULL,
  test_007_001_execute
};

/**
 * @page test_007_002 [7.2] Event Flags dispatching
 *
 * <h2>Description</h2>
 * The test dispatches three event flags and verifies that the
 * associated event handlers are invoked in LSb-first order.
 *
 * <h2>Test Steps</h2>
 * - [7.2.1] Three evenf flag bits are raised then chEvtDispatch() is
 *   invoked, the sequence of handlers calls is tested.
 * .
 */

static void test_007_002_setup(void) {
  chEvtGetAndClearEvents(ALL_EVENTS);
}

static void test_007_002_execute(void) {

  /* [7.2.1] Three evenf flag bits are raised then chEvtDispatch() is
     invoked, the sequence of handlers calls is tested.*/
  test_set_step(1);
  {
    chEvtDispatch(evhndl, 7);
    test_assert_sequence("ABC", "invalid sequence");
  }
}

static const testcase_t test_007_002 = {
  "Event Flags dispatching",
  test_007_002_setup,
  NULL,
  test_007_002_execute
};

/**
 * @page test_007_003 [7.3] Events Flags wait using chEvtWaitOne()
 *
 * <h2>Description</h2>
 * Functionality of chEvtWaitOne() is tested under various scenarios.
 *
 * <h2>Test Steps</h2>
 * - [7.3.1] Setting three event flags.
 * - [7.3.2] Calling chEvtWaitOne() three times, each time a single
 *   flag must be returned in order of priority.
 * - [7.3.3] Getting current time and starting a signaler thread, the
 *   thread will set an event flag after 50mS.
 * - [7.3.4] Calling chEvtWaitOne() then verifying that the event has
 *   been received after 50mS and that the event flags mask has been
 *   emptied.
 * .
 */

static void test_007_003_setup(void) {
  chEvtGetAndClearEvents(ALL_EVENTS);
}

static void test_007_003_execute(void) {
  eventmask_t m;
  systime_t target_time;

  /* [7.3.1] Setting three event flags.*/
  test_set_step(1);
  {
    chEvtAddEvents(7);
  }

  /* [7.3.2] Calling chEvtWaitOne() three times, each time a single
     flag must be returned in order of priority.*/
  test_set_step(2);
  {
    m = chEvtWaitOne(ALL_EVENTS);
    test_assert(m == 1, "single event error");
    m = chEvtWaitOne(ALL_EVENTS);
    test_assert(m == 2, "single event error");
    m = chEvtWaitOne(ALL_EVENTS);
    test_assert(m == 4, "single event error");
    m = chEvtGetAndClearEvents(ALL_EVENTS);
    test_assert(m == 0, "stuck event");
  }

  /* [7.3.3] Getting current time and starting a signaler thread, the
     thread will set an event flag after 50mS.*/
  test_set_step(3);
  {
    target_time = test_wait_tick() + MS2ST(50);
    threads[0] = chThdCreateStatic(wa[0], WA_SIZE, chThdGetPriorityX() - 1,
                                   evt_thread3, chThdGetSelfX());
  }

  /* [7.3.4] Calling chEvtWaitOne() then verifying that the event has
     been received after 50mS and that the event flags mask has been
     emptied.*/
  test_set_step(4);
  {
    m = chEvtWaitOne(ALL_EVENTS);
    test_assert_time_window(target_time, target_time + ALLOWED_DELAY,
                            "out of time window");
    test_assert(m == 1, "event flag error");
    m = chEvtGetAndClearEvents(ALL_EVENTS);
    test_assert(m == 0, "stuck event");
    test_wait_threads();
  }
}

static const testcase_t test_007_003 = {
  "Events Flags wait using chEvtWaitOne()",
  test_007_003_setup,
  NULL,
  test_007_003_execute
};

/**
 * @page test_007_004 [7.4] Events Flags wait using chEvtWaitAny()
 *
 * <h2>Description</h2>
 * Functionality of chEvtWaitAny() is tested under various scenarios.
 *
 * <h2>Test Steps</h2>
 * - [7.4.1] Setting two, non contiguous, event flags.
 * - [7.4.2] Calling chEvtWaitAny() one time, the two flags must be
 *   returned.
 * - [7.4.3] Getting current time and starting a signaler thread, the
 *   thread will set an event flag after 50mS.
 * - [7.4.4] Calling chEvtWaitAny() then verifying that the event has
 *   been received after 50mS and that the event flags mask has been
 *   emptied.
 * .
 */

static void test_007_004_setup(void) {
  chEvtGetAndClearEvents(ALL_EVENTS);
}

static void test_007_004_execute(void) {
  eventmask_t m;
  systime_t target_time;

  /* [7.4.1] Setting two, non contiguous, event flags.*/
  test_set_step(1);
  {
    chEvtAddEvents(5);
  }

  /* [7.4.2] Calling chEvtWaitAny() one time, the two flags must be
     returned.*/
  test_set_step(2);
  {
    m = chEvtWaitAny(ALL_EVENTS);
    test_assert(m == 5, "unexpected pending bit");
    m = chEvtGetAndClearEvents(ALL_EVENTS);
    test_assert(m == 0, "stuck event");
  }

  /* [7.4.3] Getting current time and starting a signaler thread, the
     thread will set an event flag after 50mS.*/
  test_set_step(3);
  {
    target_time = test_wait_tick() + MS2ST(50);
    threads[0] = chThdCreateStatic(wa[0], WA_SIZE, chThdGetPriorityX() - 1,
                                   evt_thread3, chThdGetSelfX());
  }

  /* [7.4.4] Calling chEvtWaitAny() then verifying that the event has
     been received after 50mS and that the event flags mask has been
     emptied.*/
  test_set_step(4);
  {
    m = chEvtWaitAny(ALL_EVENTS);
    test_assert_time_window(target_time, target_time + ALLOWED_DELAY,
                            "out of time window");
    test_assert(m == 1, "event flag error");
    m = chEvtGetAndClearEvents(ALL_EVENTS);
    test_assert(m == 0, "stuck event");
    test_wait_threads();
  }
}

static const testcase_t test_007_004 = {
  "Events Flags wait using chEvtWaitAny()",
  test_007_004_setup,
  NULL,
  test_007_004_execute
};

/**
 * @page test_007_005 [7.5] Events Flags wait using chEvtWaitAll()
 *
 * <h2>Description</h2>
 * Functionality of chEvtWaitAll() is tested under various scenarios.
 *
 * <h2>Test Steps</h2>
 * - [7.5.1] Setting two, non contiguous, event flags.
 * - [7.5.2] Calling chEvtWaitAll() one time, the two flags must be
 *   returned.
 * - [7.5.3] Setting one event flag.
 * - [7.5.4] Getting current time and starting a signaler thread, the
 *   thread will set another event flag after 50mS.
 * - [7.5.5] Calling chEvtWaitAll() then verifying that both event
 *   flags have been received after 50mS and that the event flags mask
 *   has been emptied.
 * .
 */

static void test_007_005_setup(void) {
  chEvtGetAndClearEvents(ALL_EVENTS);
}

static void test_007_005_execute(void) {
  eventmask_t m;
  systime_t target_time;

  /* [7.5.1] Setting two, non contiguous, event flags.*/
  test_set_step(1);
  {
    chEvtAddEvents(5);
  }

  /* [7.5.2] Calling chEvtWaitAll() one time, the two flags must be
     returned.*/
  test_set_step(2);
  {
    m = chEvtWaitAll(5);
    test_assert(m == 5, "unexpected pending bit");
    m = chEvtGetAndClearEvents(ALL_EVENTS);
    test_assert(m == 0, "stuck event");
  }

  /* [7.5.3] Setting one event flag.*/
  test_set_step(3);
  {
    chEvtAddEvents(4);
  }

  /* [7.5.4] Getting current time and starting a signaler thread, the
     thread will set another event flag after 50mS.*/
  test_set_step(4);
  {
    target_time = test_wait_tick() + MS2ST(50);
    threads[0] = chThdCreateStatic(wa[0], WA_SIZE, chThdGetPriorityX() - 1,
                                   evt_thread3, chThdGetSelfX());
  }

  /* [7.5.5] Calling chEvtWaitAll() then verifying that both event
     flags have been received after 50mS and that the event flags mask
     has been emptied.*/
  test_set_step(5);
  {
    m = chEvtWaitAll(5);
    test_assert_time_window(target_time, target_time + ALLOWED_DELAY,
                            "out of time window");
    test_assert(m == 5, "event flags error");
    m = chEvtGetAndClearEvents(ALL_EVENTS);
    test_assert(m == 0, "stuck event");
    test_wait_threads();
  }
}

static const testcase_t test_007_005 = {
  "Events Flags wait using chEvtWaitAll()",
  test_007_005_setup,
  NULL,
  test_007_005_execute
};

/****************************************************************************
 * Exported data.
 ****************************************************************************/

/**
 * @brief   Event Sources and Event Flags.
 */
const testcase_t * const test_sequence_007[] = {
  &test_007_001,
  &test_007_002,
  &test_007_003,
  &test_007_004,
  &test_007_005,
  NULL
};

#endif /* CH_CFG_USE_EVENTS */