/*
    ChibiOS - Copyright (C) 2006..2015 Giovanni Di Sirio.

    This file is part of ChibiOS.

    ChibiOS is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    ChibiOS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/**
 * @file    hal_mmcsd.c
 * @brief   MMC/SD cards common code.
 *
 * @addtogroup MMCSD
 * @{
 */

#include "hal.h"

#if HAL_USE_MMC_SPI || HAL_USE_SDC || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Gets a bit field from a words array.
 * @note    The bit zero is the LSb of the first word.
 *
 * @param[in] data      pointer to the words array
 * @param[in] end       bit offset of the last bit of the field, inclusive
 * @param[in] start     bit offset of the first bit of the field, inclusive
 *
 * @return              The bits field value, left aligned.
 *
 * @notapi
 */
uint32_t mmcsd_get_slice(const uint32_t *data, uint32_t end, uint32_t start) {
  unsigned startidx, endidx, startoff;
  uint32_t endmask;

  osalDbgCheck((end >= start) && ((end - start) < 32));

  startidx = start / 32;
  startoff = start % 32;
  endidx   = end / 32;
  endmask  = (1 << ((end % 32) + 1)) - 1;

  /* One or two pieces?*/
  if (startidx < endidx)
    return (data[startidx] >> startoff) |               /* Two pieces case. */
           ((data[endidx] & endmask) << (32 - startoff));
  return (data[startidx] & endmask) >> startoff;        /* One piece case.  */
}

/**
 * @brief   Extract card capacity from a CSD.
 * @details The capacity is returned as number of available blocks.
 *
 * @param[in] csd       the CSD record
 *
 * @return              The card capacity.
 * @retval 0            CSD format error
 */
uint32_t mmcsdGetCapacity(const uint32_t *csd) {
  uint32_t a, b, c;

  osalDbgCheck(NULL != csd);

  switch (mmcsd_get_slice(csd, MMCSD_CSD_10_CSD_STRUCTURE_SLICE)) {
  case 0:
    /* CSD version 1.0 */
    a = mmcsd_get_slice(csd, MMCSD_CSD_10_C_SIZE_SLICE);
    b = mmcsd_get_slice(csd, MMCSD_CSD_10_C_SIZE_MULT_SLICE);
    c = mmcsd_get_slice(csd, MMCSD_CSD_10_READ_BL_LEN_SLICE);
    return (a + 1) << (b + 2) << (c - 9);       /* 2^9 == MMCSD_BLOCK_SIZE. */
  case 1:
    /* CSD version 2.0.*/
    return 1024 * (mmcsd_get_slice(csd, MMCSD_CSD_20_C_SIZE_SLICE) + 1);
  default:
    /* Reserved value detected.*/
    return 0;
  }
}

/**
 * @brief   Extract MMC card capacity from a CSD or EXT_CSD.
 * @details The capacity is returned as number of available blocks.
 *
 * @param[in] csd       the CSD record
 * @param[in] ext_csd   the extended CSD record
 *
 * @return              The card capacity.
 */
uint32_t mmcsdGetCapacityMMC(const uint32_t *csd, const uint8_t *ext_csd) {
  uint32_t a, b, c;

  osalDbgCheck(NULL != csd);

  a = mmcsd_get_slice(csd, MMCSD_CSD_MMC_C_SIZE_SLICE);
  if (0xFFF != a) {
    b = mmcsd_get_slice(csd, MMCSD_CSD_MMC_C_SIZE_MULT_SLICE);
    c = mmcsd_get_slice(csd, MMCSD_CSD_MMC_READ_BL_LEN_SLICE);
    return (a + 1) << (b + 2) << (c - 9);       /* 2^9 == MMCSD_BLOCK_SIZE. */
  }
  else if (NULL != ext_csd) {
    return (ext_csd[215] << 24) +
           (ext_csd[214] << 16) +
           (ext_csd[213] << 8)  +
            ext_csd[212];
  }
  else
    return 0;
}

#endif /* HAL_USE_MMC_SPI || HAL_USE_SDC */

/** @} */
