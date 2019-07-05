/***********************************************************************
 * CAN interface driver
 *
 **********************************************************************/

#ifndef CAN_DRIVER_H_
#define CAN_DRIVER_H_

#include "board.h"
#include <stdint.h>

void CAN_init(void);
void CAN_receive_all_frames(void);
void CAN_send_frame_once(uint32_t id, uint8_t * data, uint8_t size);
void CAN_send_test(void);

void (*CAN_frame_callback)(uint32_t id, uint8_t * data, uint8_t size);

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
