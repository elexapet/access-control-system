/***********************************************************************
 * Project: CAN
 *
 **********************************************************************/

#ifndef CAN_DRIVER_H_
#define CAN_DRIVER_H_

#include <board.h>


void CAN_init(void);

/* General CCAN callback functions */
void CAN_IRQHandler(void);
void CAN_rx(uint8_t msg_obj_num);
void CAN_tx(uint8_t msg_obj_num);
void CAN_error(uint32_t error_info);


#endif /* CAN_DRIVER_H_ */
