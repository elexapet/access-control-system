/***********************************************************************
 * $Id:: canopen_driver.c 1604 2012-04-24 11:34:47Z nxp31103     $
 *
 * Project: CANopen Application Example for LPC11Cxx
 *
 * Description:
 *   CANopen driver source file
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

#include <can/canterm_driver.h>

#if defined (  __GNUC__  )						/* only for LPCXpresso, for keil/IAR this is done by linker */
#include "cr_section_macros.h"
__BSS(RESERVED) char CAN_driver_memory[184]; 	/* reserve 184 bytes for CAN driver */
#endif


/* Publish CAN Callback Functions of onchip drivers */
CCAN_CALLBACKS_T callbacks =
{
	CAN_RX,					/* callback for any message received CAN frame which ID matches with any of the message objects' masks */
	CAN_TX,					/* callback for every transmitted CAN frame */
	CAN_Error,			/* callback for CAN errors */
	NULL,						/* callback for expedited read access (not used) */
	NULL,						/* callback for expedited write access (not used) */
	NULL,						/* callback for segmented read access (not used) */
	NULL,						/* callback for segmented write access (not used) */
	NULL,						/* callback for fall-back SDO handler (not used) */
};

/*****************************************************************************
** Function name:		CANterm_init
**
** Description:			Initializes CAN
** 						Function should be executed before using the CAN bus.
** 						Initializes the CAN controller, on-chip drivers.
**
**
** Parameters:			None
** Returned value:		None
*****************************************************************************/
void CANterm_init(void)
{
  // Power up CAN
  Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_CAN);

  // Deassert reset
  Chip_SYSCTL_DeassertPeriphReset(RESET_CAN0);

  CCAN_MSG_OBJ_T CANopen_Msg_Obj;
  /* Initialize CAN Controller structure*/
	uint32_t ClkInitTable[2] =
	{
	  /* CANCLKDIV: CAN_CLK=48MHz */
	  0x00000000UL,
	  /* Propagation time for UTP copper cable (0.64c) with maximum distance of 100 meter is 0.55us */
	  /* CANBT register: TSEG1=4, TSEG2=5, SJW=4, BRP=48 (actual value written is -1) */
	  /* Equals to 100KBit/s and CAN_CLK tolerance 1.58% */
	  0x000034EFUL
	};

	/* Initialize the CAN controller */
	LPC_CCAN_API->init_can(&ClkInitTable[0], TRUE);

	/* Configure the CAN callback functions */
	LPC_CCAN_API->config_calb(&callbacks);

	/* Enable the CAN Interrupt */
	NVIC_EnableIRQ(CAN_IRQn);

	/* Configure message object 3 to receive all 11-bit messages 0x700-0x77F */
	CANopen_Msg_Obj.msgobj = 3;
	CANopen_Msg_Obj.mode_id = 0x700;
	CANopen_Msg_Obj.mask = 0x780;
	LPC_CCAN_API->config_rxmsgobj(&CANopen_Msg_Obj);
}


/*****************************************************************************
** Function name:		CAN_IRQHandler
**
** Description:			CAN interrupt handler.
** 						The CAN interrupt handler must be provided by the user application.
**						It's function is to call the isr() API located in the ROM
**
** Parameters:			None
** Returned value:		None
*****************************************************************************/
void CAN_IRQHandler (void)
{
  LPC_CCAN_API->isr();
}

/*****************************************************************************
** Function name:		CAN_RX
**
** Description:			CAN receive callback.
** 						Function is executed by the Callback handler after
**						a CAN message has been received
**
** Parameters:			msg_obj_num. Contains the number of the message object
** 						that triggered the CAN receive callback.
** Returned value:		None
*****************************************************************************/
void CAN_RX(uint8_t msg_obj_num)
{

//	uint32_t i;
//	CCAN_MSG_OBJ_T CANopen_Msg_Obj;
//
//	/* Determine which CAN message has been received */
//	CANopen_Msg_Obj.msgobj = msg_obj_num;
//
//	/* Now load up the CANopen_Msg_Obj structure with the CAN message */
//	LPC_CCAN_API->can_receive(&CANopen_Msg_Obj);
//
//	if(msg_obj_num == 3)
//	{
//		/* message object used for heartbeat / bootup */
//		for(i=0; i<WatchListLength; i++)
//		{
//			if((CANopen_Msg_Obj.mode_id & 0x7F) == WatchList[i].NodeID || (CANopen_Msg_Obj.mode_id & 0x7F) == ((WatchList[i].value>>16) & 0x007F))
//			{
//				/* Node ID of received message is listed in watchlist */
//				WatchList[i].counter = 0;
//				WatchList[i].status = CANopen_Msg_Obj.data[0];
//				if(CANopen_Msg_Obj.data[0] == 0x00)
//				{
//					/* received message is bootup */
//					WatchList[i].status = NMT_STATE_PRE_OPERATIONAL;			/* Received bootup, thus state is pre-op */
//					if(WatchList[i].heartbeatFail)
//						WatchList[i].BootupAfterHBF = 1;
//					CANopen_NMT_Consumer_Bootup_Received(CANopen_Msg_Obj.mode_id & 0x7F);
//				}
//			}
//		}
//	}
//
//	if (msg_obj_num == 5)
//	{
//		/* message object used for NMT */
//		if(CANopen_Msg_Obj.data[1] == CAN_NODE_ID || CANopen_Msg_Obj.data[1] == 0x00)
//			CANopen_NMT_Change_MyState(CANopen_Msg_Obj.data[0]);			/* change NMT state both on broadcast and on my ID */
//	}
//
//	if (msg_obj_num == 7)
//	{
//		/* message object used for SDO client */
//		if(CANopen_SDOC_State == CANopen_SDOC_Exp_Read_Busy)
//		{
//			/* Expedited read was initiated */
//			if((CANopen_Msg_Obj.data[0] & (7<<5)) == 0x40)
//			{
//				/* received data from server */
//				i = 4-((CANopen_Msg_Obj.data[0]>>2) & 0x03);				/* i now contains number of valid data bytes */
//				CANopen_SDOC_InBuff = 0;
//
//				while(i--)
//					CANopen_SDOC_Buff[i] = CANopen_Msg_Obj.data[CANopen_SDOC_InBuff++ + 4];	/* save valid databytes to memory */
//				CANopen_SDOC_State = CANopen_SDOC_Succes;					/* expedited read completed successfully */
//				if(CANopen_SDOC_Exp_ValidBytes)
//					*CANopen_SDOC_Exp_ValidBytes = CANopen_SDOC_InBuff;		/* save number of valid bytes */
//			}
//		}
//		else if(CANopen_SDOC_State == CANopen_SDOC_Exp_Write_Busy)
//		{
//			/* expedited write was initiated */
//			if(CANopen_Msg_Obj.data[0] == 0x60)
//				CANopen_SDOC_State = CANopen_SDOC_Succes;					/* received confirmation */
//		}
//		else if(CANopen_SDOC_State == CANopen_SDOC_Seg_Read_Busy)
//		{
//			/* segmented read was initiated */
//			if(((CANopen_Msg_Obj.data[0] & (7<<5)) == 0x40) && ((CANopen_Msg_Obj.data[0] & (1<<1)) == 0x00))
//			{
//				/* Received reply on initiate command, send first segment request */
//				CANopen_Msg_Obj.msgobj = 8;
//				CANopen_Msg_Obj.mode_id = 0x600 + CANopen_SDOC_Seg_ID;
//				CANopen_Msg_Obj.data[0] = 0x60;
//				CANopen_Msg_Obj.data[1] = 0x00;
//				CANopen_Msg_Obj.data[2] = 0x00;
//				CANopen_Msg_Obj.data[3] = 0x00;
//				CANopen_Msg_Obj.data[4] = 0x00;
//				CANopen_Msg_Obj.data[5] = 0x00;
//				CANopen_Msg_Obj.data[6] = 0x00;
//				CANopen_Msg_Obj.data[7] = 0x00;
//				LPC_CCAN_API->can_transmit(&CANopen_Msg_Obj);
//
//				CANopen_SDOC_InBuff = 0;
//			}
//			else if((CANopen_Msg_Obj.data[0] & (7<<5)) == 0x00)
//			{
//				/* Received response on request */
//				for(i=0; i < (7 - ((CANopen_Msg_Obj.data[0]>>1) & 0x07)); i++)
//				{
//					/* get all data from frame and save it to memory */
//					if(CANopen_SDOC_InBuff < CANopen_SDOC_Seg_BuffSize)
//						CANopen_SDOC_Buff[CANopen_SDOC_InBuff++] = CANopen_Msg_Obj.data[i+1];
//					else
//					{
//						/* SDO segment too big for buffer, abort */
//						CANopen_SDOC_State = CANopen_SDOC_Fail;
//					}
//				}
//
//				if(CANopen_Msg_Obj.data[0] & 0x01)
//				{
//					/* Last frame, change status to success */
//					CANopen_SDOC_State = CANopen_SDOC_Succes;
//				}
//				else
//				{
//					/* not last frame, send acknowledge */
//					CANopen_Msg_Obj.msgobj = 8;
//					CANopen_Msg_Obj.mode_id = 0x600 + CANopen_SDOC_Seg_ID;
//					CANopen_Msg_Obj.data[0] = 0x60 | ((CANopen_Msg_Obj.data[0] & (1<<4)) ^ (1<<4));		/* toggle */
//					CANopen_Msg_Obj.data[1] = 0x00;
//					CANopen_Msg_Obj.data[2] = 0x00;
//					CANopen_Msg_Obj.data[3] = 0x00;
//					CANopen_Msg_Obj.data[4] = 0x00;
//					CANopen_Msg_Obj.data[5] = 0x00;
//					CANopen_Msg_Obj.data[6] = 0x00;
//					CANopen_Msg_Obj.data[7] = 0x00;
//					LPC_CCAN_API->can_transmit(&CANopen_Msg_Obj);
//				}
//			}
//		}
//		else if(CANopen_SDOC_State == CANopen_SDOC_Seg_Write_Busy)
//		{
//			/* segmented write was initiated */
//			if((((CANopen_Msg_Obj.data[0] & (7<<5)) == 0x60) && ((CANopen_Msg_Obj.data[0] & (1<<1)) == 0x00)) || ((CANopen_Msg_Obj.data[0] & (7<<5)) == 0x20))
//			{
//				/* received acknowledge */
//				CANopen_Msg_Obj.msgobj = 8;
//				CANopen_Msg_Obj.mode_id = 0x600 + CANopen_SDOC_Seg_ID;
//				if((CANopen_Msg_Obj.data[0] & (7<<5)) == 0x60)
//				{
//					/* first frame */
//					CANopen_SDOC_InBuff = 0;			/* Clear buffer */
//					CANopen_Msg_Obj.data[0] = 1<<4;		/* initialize for toggle */
//				}
//				CANopen_Msg_Obj.data[0] = ((CANopen_Msg_Obj.data[0] & (1<<4)) ^ (1<<4));		/* toggle */
//
//				/* fill frame data */
//				for(i=0; i<7; i++)
//				{
//					if(CANopen_SDOC_InBuff < CANopen_SDOC_Seg_BuffSize)
//						CANopen_Msg_Obj.data[i+1] = CANopen_SDOC_Buff[CANopen_SDOC_InBuff++];
//					else
//						CANopen_Msg_Obj.data[i+1] = 0x00;
//				}
//
//				/* if end of buffer has been reached, then this is the last frame */
//				if(CANopen_SDOC_InBuff == CANopen_SDOC_Seg_BuffSize)
//				{
//					CANopen_Msg_Obj.data[0] |= ((7-(CANopen_SDOC_Seg_BuffSize%7))<<1) | 0x01;		/* save length */
//					CANopen_SDOC_State = CANopen_SDOC_Succes;										/* set state to succes */
//				}
//
//				LPC_CCAN_API->can_transmit(&CANopen_Msg_Obj);
//			}
//		}
//	}
//	return;
}

/*****************************************************************************
** Function name:		CAN_TX
**
** Description:			CAN transmit callback.
** 						Function is executed by the Callback handler after
**						a CAN message has been transmitted
**
** Parameters:			msg_obj_num. Contains the number of the message object
** 						that triggered the CAN transmit callback.
** Returned value:		None
*****************************************************************************/
void CAN_TX(uint8_t msg_obj_num){
  return;
}

/*****************************************************************************
** Function name:		CAN_Error
**
** Description:			CAN error callback.
** 						Function is executed by the Callback handler after
**						an error has occured on the CAN bus
**
** Parameters:			error_info. Contains the error code
** 						that triggered the CAN error callback.
** Returned value:		None
*****************************************************************************/
void CAN_Error(uint32_t error_info){
  return;
}


