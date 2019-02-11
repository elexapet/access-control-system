/***********************************************************************
 * $Id:: CAN_Node_Def.c 1604 2012-04-24 11:34:47Z nxp31103     $
 *
 * Project: CANopen Application Example for LPC11Cxx (slave)
 *
 * Description:
 *   CANopen definition source file
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

#include <can/CAN_Node_Def.h>
#include <stdint.h>

/* Application variables used in variable OD */
uint8_t  error_register;
uint8_t  LEDArray;
volatile SDOS_Buffer_t SDOS_2200 =
{
	(uint8_t*)SDOS_2200_Data,
	sizeof(SDOS_2200_Data),
};
volatile uint8_t SDOS_2200_Data[255] = "Boot-up value of SDO 2200h";
uint32_t CANopen_Heartbeat_Producer_Value;
WatchNode_t WatchList[1];
uint8_t WatchListLength = sizeof(WatchList)/sizeof(WatchList[0]);



/*	CANopen read-only (constant) Object Dictionary (OD) entries
	Used with Expedited SDO only. Lengths = 1/2/4 bytes */
CAN_ODCONSTENTRY myConstOD [] =
{
	/* index, subindex,	length,	value */
	{ 0x1000, 0x00, 	4, 		0x00000000UL },
	{ 0x1018, 0x00, 	1, 		0x00000001UL },		/* only vendor ID is specified */
	{ 0x1018, 0x01, 	4, 		0x000002DCUL },		/* NXP vendor ID for CANopen */
};
uint32_t NumberOfmyConstODEntries = sizeof(myConstOD)/sizeof(myConstOD[0]);

/*	CANopen list of variable Object Dictionary (OD) entries
	Expedited SDO with length=1/2/4 bytes and segmented SDO */
CAN_ODENTRY myOD [] =
{
	/* index, subindex,	access_type | length,	value_pointer */
	{ 0x1001, 0x00, 	OD_EXP_RO | 1,			(uint8_t *)&error_register },
	{ 0x1016, 0x00,     OD_EXP_RO | 1,          (uint8_t *)&WatchListLength},
	{ 0x1016, 0x01,     OD_EXP_RW | 4,          (uint8_t *)&WatchList[0].value},
	{ 0x1017, 0x00,     OD_EXP_RW | 2,          (uint8_t *)&CANopen_Heartbeat_Producer_Value},
	{ 0x2000, 0x00,     OD_EXP_RW | 1,          (uint8_t *)&LEDArray},
	{ 0x2200, 0x00,		OD_SEG_RW,				(uint8_t *)&SDOS_2200},
};
uint32_t NumberOfmyODEntries = sizeof(myOD)/sizeof(myOD[0]);
