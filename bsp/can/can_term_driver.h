/***********************************************************************
 * CAN interface driver
 * for only one user
 *
 **********************************************************************/

#ifndef CAN_DRIVER_H_
#define CAN_DRIVER_H_

#include "board.h"
#include <stdint.h>

#define CCAN_MSG_OBJ_FIRST 0
#define CCAN_MSG_OBJ_LAST  31
#define CAN_EXT_ID_BIT_MASK 0x1FFFFFFFUL
#define CAN_DLC_MAX 8

void CAN_init(CCAN_CALLBACKS_T * ptr_callbacks, uint32_t baud_rate);
void CAN_recv_filter_set(uint8_t msgobj_num, uint32_t id, uint32_t mask);
void CAN_recv_filter_set_eff(uint8_t msgobj_num);
void CAN_send_once(uint8_t msgobj_num, uint32_t id, uint8_t * data, uint8_t size);
void CAN_send_test(void);

/*
 * CCAN_MSG_OBJ_T cheat sheet:
 *
 * mode_id ... Frame ID + control bits (STD/EXT frame, DATA/RTR frame)
 * mask ... Mask for arbitration bits (RX only)
 * data[8] ... Frame data up to 0 to 8 bytes
 * dlc ... Data length code
 * msgobj ... Number of message object to use (0-31, 0 has highest priority)
 */


#endif /* CAN_DRIVER_H_ */
