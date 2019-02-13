/***********************************************************************
 * $Id:: canopen_driver.h 1604 2012-04-24 11:34:47Z nxp31103     $
 *
 * Project: CANopen Application Example for LPC11Cxx
 *
 * Description:
 *   CANopen driver header file
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

#ifndef CANOPEN_DRIVER_H_
#define CANOPEN_DRIVER_H_

#include <board.h>

/* Message buffers used by CANopen library:
CANopen driver RX		1
CANopen driver TX		2
heartbeat RX			3
heartbeat TX			4
NMT RX					5
NMT TX					6
SDO Client RX			7
SDO Client TX			8
*/

/* General CANopen functions */
void CANterm_init(void);

/* General CAN functions */
void CAN_IRQHandler (void);
void CAN_RX(uint8_t msg_obj_num);
void CAN_TX(uint8_t msg_obj_num);
void CAN_Error(uint32_t error_info);

/* Global Variables */

#endif /* CANOPEN_DRIVER_H_ */
