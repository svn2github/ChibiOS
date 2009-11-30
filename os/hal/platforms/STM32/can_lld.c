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

/**
 * @file STM32/can_lld.c
 * @brief STM32 CAN subsystem low level driver source
 * @addtogroup STM32_CAN
 * @{
 */

#include "ch.h"
#include "hal.h"

#if CH_HAL_USE_CAN || defined(__DOXYGEN__)

/*===========================================================================*/
/* Low Level Driver exported variables.                                      */
/*===========================================================================*/

/** @brief ADC1 driver identifier.*/
#if USE_STM32_CAN1 || defined(__DOXYGEN__)
CANDriver CAND1;
#endif

/*===========================================================================*/
/* Low Level Driver local variables.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Low Level Driver local functions.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Low Level Driver interrupt handlers.                                      */
/*===========================================================================*/

/*
 * CAN1 TX interrupt handler.
 */
CH_IRQ_HANDLER(Vector8C) {

  CH_IRQ_PROLOGUE();

  /* No more events until a message is transmitted.*/
  CAN1->IER &= ~CAN_IER_TMEIE;
  chEvtBroadcastI(&CAND1.cd_txempty_event);

  CH_IRQ_EPILOGUE();
}

/*
 * CAN1 RX0 interrupt handler.
 */
CH_IRQ_HANDLER(Vector90) {

  CH_IRQ_PROLOGUE();

  /* No more events until the incoming messages queues are emptied.*/
  CAN1->IER &= ~(CAN_IER_FMPIE0 | CAN_IER_FMPIE1);
  chEvtBroadcastI(&CAND1.cd_rxfull_event);

  CH_IRQ_EPILOGUE();
}

/*
 * CAN1 RX1 interrupt handler.
 */
CH_IRQ_HANDLER(Vector94) {

  CH_IRQ_PROLOGUE();

  /* No more events until the incoming messages queues are emptied.*/
  CAN1->IER &= ~(CAN_IER_FMPIE0 | CAN_IER_FMPIE1);
  chEvtBroadcastI(&CAND1.cd_rxfull_event);

  CH_IRQ_EPILOGUE();
}

/*
 * CAN1 SCE interrupt handler.
 */
CH_IRQ_HANDLER(Vector98) {

  CH_IRQ_PROLOGUE();

  canAddFlagsI(&CAND1, 1);
  chEvtBroadcastI(&CAND1.cd_error_event);
  CAN1->MSR = CAN_MSR_ERRI;

  CH_IRQ_EPILOGUE();
}

/*===========================================================================*/
/* Low Level Driver exported functions.                                      */
/*===========================================================================*/

/**
 * @brief Low level CAN driver initialization.
 */
void can_lld_init(void) {

}

/**
 * @brief Configures and activates the CAN peripheral.
 *
 * @param[in] canp pointer to the @p CANDriver object
 */
void can_lld_start(CANDriver *canp) {

  if (canp->cd_state == CAN_STOP) {
    /* Clock activation.*/
  }
  /* Configuration.*/
}

/**
 * @brief Deactivates the CAN peripheral.
 *
 * @param[in] canp pointer to the @p CANDriver object
 */
void can_lld_stop(CANDriver *canp) {

}

/**
 * @brief Determines whether a frame can be transmitted.
 *
 * @param[in] canp pointer to the @p CANDriver object
 *
 * @return The queue space availability.
 * @retval FALSE no space in the transmit queue.
 * @retval TRUE transmit slot available.
 */
bool_t can_lld_can_transmit(CANDriver *canp) {

  return FALSE;
}

/**
 * @brief Inserts a frame into the transmit queue.
 *
 * @param[in] canp      pointer to the @p CANDriver object
 * @param[in] cfp       pointer to the CAN frame to be transmitted
 *
 * @return The operation status.
 * @retval RDY_OK frame transmitted.
 */
msg_t can_lld_transmit(CANDriver *canp, const CANFrame *cfp) {

  return RDY_OK;
}

/**
 * @brief Determines whether a frame has been received.
 *
 * @param[in] canp pointer to the @p CANDriver object
 *
 * @return The queue space availability.
 * @retval FALSE no space in the transmit queue.
 * @retval TRUE transmit slot available.
 */
bool_t can_lld_can_receive(CANDriver *canp) {

  return FALSE;
}

/**
 * @brief Receives a frame from the input queue.
 *
 * @param[in] canp      pointer to the @p CANDriver object
 * @param[out] cfp      pointer to the buffer where the CAN frame is copied
 *
 * @return The operation status.
 * @retval RDY_OK frame received.
 */
msg_t can_lld_receive(CANDriver *canp, CANFrame *cfp) {

  return RDY_OK;
}

#if CAN_USE_SLEEP_MODE || defined(__DOXYGEN__)
/**
 * @brief Enters the sleep mode.
 *
 * @param[in] canp      pointer to the @p CANDriver object
 */
void can_lld_sleep(CANDriver *canp) {

}

/**
 * @brief Enforces leaving the sleep mode.
 *
 * @param[in] canp      pointer to the @p CANDriver object
 */
void can_lld_wakeup(CANDriver *canp) {

}
#endif /* CAN_USE_SLEEP_MODE */

#endif /* CH_HAL_USE_CAN */

/** @} */
