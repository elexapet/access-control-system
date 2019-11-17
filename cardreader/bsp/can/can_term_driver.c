/**
 *  @file
 *  @brief Interface for LPC11C24's on-board CCAN driver in ROM.
 *
 *  Based on LPCOpen CCAN on-chip driver example.
 *
 *  @author Petr Elexa
 *
 * @note
 * Copyright(C) NXP Semiconductors, 2012
 * All rights reserved.
 *
 * @par
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * LPC products.  This software is supplied "AS IS" without any warranties of
 * any kind, and NXP Semiconductors and its licensor disclaim any and
 * all warranties, express or implied, including all implied warranties of
 * merchantability, fitness for a particular purpose and non-infringement of
 * intellectual property rights.  NXP Semiconductors assumes no responsibility
 * or liability for the use of the software, conveys no license or rights under any
 * patent, copyright, mask work right, or any other intellectual property rights in
 * or to any products. NXP Semiconductors reserves the right to make changes
 * in the software without notification. NXP Semiconductors also makes no
 * representation or warranty that such application will be suitable for the
 * specified use without further testing or modification.
 *
 * @par
 * Permission to use, copy, modify, and distribute this software and its
 * documentation is hereby granted, under NXP Semiconductors' and its
 * licensor's relevant copyrights in the software, without fee, provided that it
 * is used in conjunction with NXP Semiconductors microcontrollers.  This
 * copyright, permission, and disclaimer notice must appear in all copies of
 * this code.
 */

#include "can/can_term_driver.h"
#include <string.h>

#if defined (  __GNUC__  )
#include "cr_section_macros.h"
__BSS(RESERVED) char CAN_driver_memory[184];  /* reserve 184 bytes for CAN driver */
#endif

// Time quanta (Bit-rate prescaler)
#define CCAN_BCR_QUANTA(x) ((x) & 0x3F)
// Synchronization jump width
#define CCAN_BCR_SJW(x) (((x) & 0x3) << 6)
// Time segment 1
#define CCAN_BCR_TSEG1(x) (((x) & 0x0F) << 8)
// Time segment 2
#define CCAN_BCR_TSEG2(x) (((x) & 0x07) << 12)

#define CAN_CALC_SYNC_SEG 1


// Sample point = 100 * (tseg1 + CAN_CALC_SYNC_SEG) / (tseg1 + tseg2 + CAN_CALC_SYNC_SEG)

// HW constants
#define	TSEG1_MIN 1
#define	TSEG1_MAX 13
#define TSEG2_MIN 1
#define TSEG2_MAX 8
#define SJW_MAX   4
#define BRP_MIN   1
#define BRP_MAX   32


/*****************************************************************************
 * Private functions
 ****************************************************************************/

/*
 * Integrated CCAN hardware timing calculation from LPC11Cxx CCAN on-chip driver example.
 * Satisfies chapter 8 "BIT TIMING REQUIREMENTS" of the "Bosch CAN Specification version 2.0"
 * at http://www.semiconductors.bosch.de/pdf/can2spec.pdf.
 *
 * Hard coded sample point 50%
 */
static void _timing_calculate(uint32_t baud_rate, uint32_t * can_api_timing_cfg)
{
  uint32_t pClk, div, quanta, segs, seg1, seg2, clk_per_bit, can_sjw;
  pClk = Chip_Clock_GetMainClockRate();

  clk_per_bit = pClk / baud_rate;

  for (div = 0; div <= 15; div++)
  {
    for (quanta = 1; quanta <= 32; quanta++)
    {
      for (segs = 3; segs <= 17; segs++)
      {
        if (clk_per_bit == (segs * quanta * (div + 1)))
        {
          segs -= 3;
          seg1 = segs / 2;
          seg2 = segs - seg1;
          can_sjw = seg1 > 3 ? 3 : seg1;
          can_api_timing_cfg[0] = div;
          can_api_timing_cfg[1] =
            CCAN_BCR_QUANTA(quanta - 1) | CCAN_BCR_SJW(can_sjw) | CCAN_BCR_TSEG1(seg1) | CCAN_BCR_TSEG2(seg2);
          return;
        }
      }
    }
  }
}

/*
 * Integrated CCAN hardware timing calculation from LPC11Cxx CCAN on-chip driver example.
 * Satisfies chapter 8 "BIT TIMING REQUIREMENTS" of the "Bosch CAN Specification version 2.0"
 * at http://www.semiconductors.bosch.de/pdf/can2spec.pdf.
 *
 * Uses CiA recommended sample points.
 */
static void _timing_calculate_sp(uint32_t baud_rate, uint32_t * can_api_timing_cfg)
{
  uint32_t pClk, div, quanta, segs, seg1, seg2, clk_per_bit, can_sjw;
  pClk = Chip_Clock_GetMainClockRate();

  uint8_t spt_nominal;

  if (baud_rate > 800000)
  {
	spt_nominal = 75;
  }
  else if (baud_rate > 500000)
  {
	spt_nominal = 80;
  }
  else
  {
	spt_nominal = 87;
  }

  clk_per_bit = pClk / baud_rate;

  for (div = 0; div <= 15; div++)
  {
    for (quanta = 1; quanta <= 32; quanta++)
    {
      for (segs = 3; segs <= 17; segs++)
      {
        if (clk_per_bit == (segs * quanta * (div + 1)))
        {
          segs -= 3;

          // Sample point
          seg2 = segs - (spt_nominal * segs) / 100;
          seg2 = seg2 > TSEG2_MIN ? (seg2 < TSEG2_MAX ? seg2 : TSEG2_MAX) : TSEG2_MIN; // Clamp the value
          seg1 = segs - seg2;

          can_sjw = seg1 > 3 ? 3 : seg1;

          can_api_timing_cfg[0] = div;
          can_api_timing_cfg[1] =
            CCAN_BCR_QUANTA(quanta - 1) | CCAN_BCR_SJW(can_sjw) | CCAN_BCR_TSEG1(seg1) | CCAN_BCR_TSEG2(seg2);
          return;
        }
      }
    }
  }
}

static inline void _125_kbaud_75sp(uint32_t * can_api_timing_cfg)
{
  /* 125kb 75%, CAN_CLK tolerance 5% */
  /* Propagation time for UTP copper cable (0.64c) with maximum distance of 100 meter is 0.55us */
  /* CANCLKDIV: CAN_CLK=48MHz */
  /* CANBT register: TSEG1=11, TSEG2=4, SJW=4, BRP=4 (actual value written is -1) */
  can_api_timing_cfg[0] = 5;
  can_api_timing_cfg[1] = CCAN_BCR_QUANTA(3) | CCAN_BCR_SJW(3) | CCAN_BCR_TSEG1(10) | CCAN_BCR_TSEG2(3);
}

/*****************************************************************************
 * Public functions
 ****************************************************************************/

void CAN_init(CCAN_CALLBACKS_T * ptr_callbacks, uint32_t baud_rate)
{
  // Power up CAN
  Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_CAN);

  // Deassert reset
  Chip_SYSCTL_DeassertPeriphReset(RESET_CAN0);

  /* Initialize CAN Controller structure*/
  uint32_t ClkInitTable[2];
  _timing_calculate(baud_rate, &ClkInitTable[0]);

  /* Initialize the CAN controller */
  LPC_CCAN_API->init_can(&ClkInitTable[0], TRUE);

  /* Configure the CAN callback functions */
  LPC_CCAN_API->config_calb(ptr_callbacks);

  /* Enable the CAN Interrupt */
  NVIC_EnableIRQ(CAN_IRQn);
}

void CAN_recv_filter(uint8_t msgobj_num, uint32_t id, uint32_t mask, bool extended)
{
  CCAN_MSG_OBJ_T msg_obj = {0, };

  if (extended == true)
  {
    id |= CAN_MSGOBJ_EXT;
    mask |= CAN_MSGOBJ_EXT;
  }

  msg_obj.msgobj = msgobj_num;
  msg_obj.mode_id = id;
  msg_obj.mask = mask;
  LPC_CCAN_API->config_rxmsgobj(&msg_obj);
}

void CAN_recv_filter_all_ext(uint8_t msgobj_num)
{
  CCAN_MSG_OBJ_T msg_obj = {0, };

  msg_obj.msgobj = msgobj_num;
  msg_obj.mode_id = CAN_MSGOBJ_EXT;
  msg_obj.mask = CAN_MSGOBJ_EXT;
  LPC_CCAN_API->config_rxmsgobj(&msg_obj);
}

void CAN_send_once(uint8_t msgobj_num, uint32_t id, uint8_t * data, uint8_t size)
{
  if (size > CAN_DLC_MAX) size = CAN_DLC_MAX;

  CCAN_MSG_OBJ_T msg_obj;

  msg_obj.msgobj  = msgobj_num;
  msg_obj.mode_id = id;
  msg_obj.mask    = 0x0;
  msg_obj.dlc     = size;

  memcpy(msg_obj.data, data, size);

  LPC_CCAN_API->can_transmit(&msg_obj);
}

void CAN_send_test(void)
{
  CCAN_MSG_OBJ_T msg_obj;
  /* Send a simple one time CAN message */
  msg_obj.msgobj  = CCAN_MSG_OBJ_LAST;
  msg_obj.mode_id = CAN_MSGOBJ_EXT | 0x123456;
  msg_obj.mask    = 0x0;
  msg_obj.dlc     = 4;
  msg_obj.data[0] = 0;
  msg_obj.data[1] = 1;
  msg_obj.data[2] = 2;
  msg_obj.data[3] = 3;
  LPC_CCAN_API->can_transmit(&msg_obj);
}

void CAN_IRQHandler(void)
{
  LPC_CCAN_API->isr();
}

