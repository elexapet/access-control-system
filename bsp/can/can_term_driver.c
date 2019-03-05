/***********************************************************************
 *  CCAN driver
 **********************************************************************/

#include <can/can_term_driver.h>
#include <can/can_term_protocol.h>

#if defined (  __GNUC__  )
#include "cr_section_macros.h"
__BSS(RESERVED) char CAN_driver_memory[184];  /* reserve 184 bytes for CAN driver */
#endif

#define CCAN_BCR_QUANTA(x) ((x) & 0x3F)
#define CCAN_BCR_SJW(x) (((x) & 0x3) << 6)
#define CCAN_BCR_TSEG1(x) (((x) & 0x0F) << 8)
#define CCAN_BCR_TSEG2(x) (((x) & 0x07) << 12)

#define CAN_EXT_ID_BIT_MASK 0x1FFFFFFFUL
#define CAN_DLC_MAX 8

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

/* ROM CCAN driver callback functions prototypes */
void CAN_rx(uint8_t msg_obj_num);
void CAN_tx(uint8_t msg_obj_num);
void CAN_error(uint32_t error_info);

/* IRQ handler prototype */
void CAN_IRQHandler(void);

/* Publish CAN Callback Functions of on-chip drivers */
CCAN_CALLBACKS_T _callbacks =
{
	CAN_rx,					/* callback for any message received CAN frame which ID matches with any of the message objects' masks */
	CAN_tx,					/* callback for every transmitted CAN frame */
	CAN_error,			/* callback for CAN errors */
	NULL,						/* callback for expedited read access (not used) */
	NULL,						/* callback for expedited write access (not used) */
	NULL,						/* callback for segmented read access (not used) */
	NULL,						/* callback for segmented write access (not used) */
	NULL,						/* callback for fall-back SDO handler (not used) */
};

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Private functions
 ****************************************************************************/


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

inline static void _100kbaud(uint32_t * can_api_timing_cfg)
{
  /* CANCLKDIV: CAN_CLK=48MHz */
  can_api_timing_cfg[0] = 0x00000000UL;
  /* Propagation time for UTP copper cable (0.64c) with maximum distance of 100 meter is 0.55us */
  /* CANBT register: TSEG1=4, TSEG2=5, SJW=4, BRP=48 (actual value written is -1) */
  /* Equals to 100KBit/s and CAN_CLK tolerance 1.58% */
  can_api_timing_cfg[1] = 0x000034EFUL;
}

/*****************************************************************************
 * Public functions
 ****************************************************************************/


/*****************************************************************************
** Function name:   CAN_init
**
** Description:     Initializes CAN
**            Function should be executed before using the CAN bus.
**            Initializes the CAN controller, on-chip drivers.
**
**
** Parameters:      None
** Returned value:    None
*****************************************************************************/
void CAN_init(void)
{
  // Power up CAN
  Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_CAN);

  // Deassert reset
  Chip_SYSCTL_DeassertPeriphReset(RESET_CAN0);

  /* Initialize CAN Controller structure*/
  uint32_t ClkInitTable[2];
  //_100kbaud(&ClkInitTable[0]);
  _timing_calculate(125000, &ClkInitTable[0]);

  /* Initialize the CAN controller */
  LPC_CCAN_API->init_can(&ClkInitTable[0], TRUE);

  /* Configure the CAN callback functions */
  LPC_CCAN_API->config_calb(&_callbacks);

  /* Enable the CAN Interrupt */
  NVIC_EnableIRQ(CAN_IRQn);
}

void CAN_receive_all_frames(void)
{
  CCAN_MSG_OBJ_T msg_obj;

  /* Configure message object 1 to receive all 29-bit messages 0x0-0x1FFFFFFF */
  msg_obj.msgobj = 1;
  msg_obj.mode_id = CAN_MSGOBJ_EXT;
  msg_obj.mask = 0x0;
  LPC_CCAN_API->config_rxmsgobj(&msg_obj);
}

void CAN_send_frame_once(uint32_t id, uint8_t * data, uint8_t size)
{
  if (size > CAN_DLC_MAX) size = CAN_DLC_MAX;

  CCAN_MSG_OBJ_T msg_obj;

  msg_obj.msgobj  = 0;
  msg_obj.mode_id = CAN_MSGOBJ_EXT | (id & CAN_EXT_ID_BIT_MASK);
  msg_obj.mask    = 0x0;
  msg_obj.dlc     = size;
  for (int i = 0; i < size; ++i)
  {
    msg_obj.data[i] = data[i];
  }

  LPC_CCAN_API->can_transmit(&msg_obj);
}

void CAN_send_test(void)
{
  CCAN_MSG_OBJ_T msg_obj;
  /* Send a simple one time CAN message */
  msg_obj.msgobj  = 0;
  msg_obj.mode_id = CAN_MSGOBJ_EXT | 0x3456;
  msg_obj.mask    = 0x0;
  msg_obj.dlc     = 4;
  msg_obj.data[0] = 0;
  msg_obj.data[1] = 1;
  msg_obj.data[2] = 2;
  msg_obj.data[3] = 3;
  LPC_CCAN_API->can_transmit(&msg_obj);
}

/*****************************************************************************
** Function name:   CAN_IRQHandler
**
** Description:     CAN interrupt handler.
**            The CAN interrupt handler must be provided by the user application.
**            It's function is to call the isr() API located in the ROM
**
** Parameters:      None
** Returned value:    None
*****************************************************************************/
void CAN_IRQHandler(void)
{
  LPC_CCAN_API->isr();
}

/*****************************************************************************
** Function name:   CAN_RX
**
** Description:     CAN receive callback.
**            Function is executed by the Callback handler after
**            a CAN message has been received
**
** Parameters:      msg_obj_num. Contains the number of the message object
**            that triggered the CAN receive callback.
** Returned value:    None
*****************************************************************************/
void CAN_rx(uint8_t msg_obj_num)
{
  CCAN_MSG_OBJ_T msg_obj;

  /* Determine which CAN message has been received */
  msg_obj.msgobj = msg_obj_num;

  /* Now load up the msg_obj structure with the CAN message */
  LPC_CCAN_API->can_receive(&msg_obj);
  if (msg_obj_num == 1)
  {
    CAN_frame_callback(msg_obj.mode_id & CAN_EXT_ID_BIT_MASK, msg_obj.data, msg_obj.dlc);
  }
}

/*****************************************************************************
** Function name:   CAN_TX
**
** Description:     CAN transmit callback.
**            Function is executed by the Callback handler after
**            a CAN message has been transmitted
**
** Parameters:      msg_obj_num. Contains the number of the message object
**            that triggered the CAN transmit callback.
** Returned value:    None
*****************************************************************************/
void CAN_tx(uint8_t msg_obj_num)
{
  return;
}

/*****************************************************************************
** Function name:   CAN_Error
**
** Description:     CAN error callback.
**            Function is executed by the Callback handler after
**            an error has occured on the CAN bus
**
** Parameters:      error_info. Contains the error code
**            that triggered the CAN error callback.
** Returned value:    None
*****************************************************************************/
void CAN_error(uint32_t error_info)
{
  return;
}
