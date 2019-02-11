/***********************************************************************
 * $Id:: CAN_Node_Def.h 1604 2012-04-24 11:34:47Z nxp31103     $
 *
 * Project: CANopen Application Example for LPC11Cxx (slave)
 *
 * Description:
 *   CANopen definition header file
 *
 * Copyright(C) 2012, NXP Semiconductor
 * All rights reserved.
 *
 ***********************************************************************
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * products. This software is supplied "AS IS" without any warranties.
 * NXP Semiconductors assumes no responsibility or liability for the
 * use of the software, conveys no license or title under any patent,
 * copyright, or mask work right to the product. NXP Semiconductors
 * reserves the right to make changes in the software without
 * notification. NXP Semiconductors also make no representation or
 * warranty that such application will be suitable for the specified
 * use without further testing or modification.
 **********************************************************************/

#ifndef _CAN_NODE_DEF_H
#define _CAN_NODE_DEF_H

#include <can/canopen_driver.h>
#include <stdint.h>

#define CAN_MASTER_NODE			0x7D		/* 125 */
#define CAN_SLAVE1_NODE			0x01		/* 1 */
#define CAN_SLAVE2_NODE			0x02		/* 2 */

#define CAN_NODE_ID				CAN_SLAVE1_NODE		/* make sure to change this when using 2 slaves */
#define CANOPEN_TIMEOUT_VAL		100					/* in ms */

/* Application variables used in variable OD */
extern uint8_t  error_register;							/* CANopen error register */
extern uint8_t  LEDArray;								/* LEDs of MCB11C14 board */
extern uint32_t CANopen_Heartbeat_Producer_Value;		/* heartbeat producer value */
extern volatile SDOS_Buffer_t SDOS_2200;				/* buffer structure associated with segmented entry 2200h */
extern volatile uint8_t SDOS_2200_Data[255];			/* buffer associated with segmented entry 2200h */

/* Watchlist */
extern WatchNode_t WatchList[];							/* for watching nodes */
extern uint8_t WatchListLength;							/* number of nodes in watchlist must be known */

/* OD */
extern CAN_ODCONSTENTRY myConstOD[];					/* constant OD entries */
extern uint32_t NumberOfmyConstODEntries;				/* required so that the number of entries in the constant OD is known */
extern CAN_ODENTRY myOD[];								/* non-constant OD entries */
extern uint32_t NumberOfmyODEntries;					/* required so that the number of entries in the non-constant OD is known */

#endif /* _CAN_NODE_DEF_H */
