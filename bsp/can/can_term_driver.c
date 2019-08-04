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


/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

/* IRQ handler prototype */
void CAN_IRQHandler(void);

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
void CAN_init(CCAN_CALLBACKS_T * ptr_callbacks, uint32_t baud_rate)
{
  // Power up CAN
  Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_CAN);

  // Deassert reset
  Chip_SYSCTL_DeassertPeriphReset(RESET_CAN0);

  /* Initialize CAN Controller structure*/
  uint32_t ClkInitTable[2];
  //_100kbaud(&ClkInitTable[0]);
  _timing_calculate(baud_rate, &ClkInitTable[0]);

  /* Initialize the CAN controller */
  LPC_CCAN_API->init_can(&ClkInitTable[0], TRUE);

  /* Configure the CAN callback functions */
  LPC_CCAN_API->config_calb(ptr_callbacks);

  /* Enable the CAN Interrupt */
  NVIC_EnableIRQ(CAN_IRQn);
}

void CAN_recv_filter_set(uint8_t msgobj_num, uint32_t id, uint32_t mask)
{
  CCAN_MSG_OBJ_T msg_obj = {0, };

  /* Configure Message object filter */
  msg_obj.msgobj = msgobj_num;
  msg_obj.mode_id = id;
  msg_obj.mask = mask;
  LPC_CCAN_API->config_rxmsgobj(&msg_obj);
}

void CAN_recv_filter_set_eff(uint8_t msgobj_num)
{
  CCAN_MSG_OBJ_T msg_obj = {0, };

  /* Configure Message object to receive all extended frames 0-0x1FFFFFFF */
  msg_obj.msgobj = msgobj_num;
  msg_obj.mode_id = CAN_MSGOBJ_EXT;
  msg_obj.mask = 0x0;
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

