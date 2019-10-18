/**
 *  @file
 *  @brief Interface for LPC11C24's on-board CCAN driver in ROM.
 *
 *
 *  @author Petr Elexa
 *  @see LICENSE
 *
 */

#ifndef CAN_DRIVER_H_
#define CAN_DRIVER_H_

#include "board.h"
#include <stdint.h>

#define CCAN_MSG_OBJ_FIRST 0
#define CCAN_MSG_OBJ_LAST  31
// Extended CAN frame ID mask.
#define CAN_EXT_ID_BIT_MASK 0x1FFFFFFFUL
// CAN message data maximal length.
#define CAN_DLC_MAX 8

/*
 * CCAN_MSG_OBJ_T (Message object) cheat sheet:
 *
 * Message object is "slot" in hardware for sending and receiving messages.
 *
 * mode_id ... Frame ID + control bits (STD/EXT frame, DATA/RTR frame)
 * mask ... Mask for arbitration bits (RX only)
 * data[8] ... Frame data up to 0 to 8 bytes
 * dlc ... Data length code
 * msgobj ... Number of message object to use (0-31, 0 has highest priority)
 */


/**
 * @brief Initializes CAN periphery.
 *
 *        Function should be executed before using the CAN bus.
 *        Initializes the CAN controller, on-chip drivers.
 *
 * @param ptr_callbacks ... pointer to callback structure
 * @param baud_rate ... CAN baud rate to use
 */
void CAN_init(CCAN_CALLBACKS_T * ptr_callbacks, uint32_t baud_rate);


/**
 * @brief Setup HW filter for received CAN messages.
 *
 *        The filter matches when <recieved_id> & mask == id & mask.
 *        Non-matching messages are dropped.
 *
 * @param msgobj_num ... number of message object (0-31) to setup.
 * @param id ... CAN message ID
 * @param mask ... CAN ID mask
 * @param extended ... use extended frame ID (29bit) if true
 */
void CAN_recv_filter(uint8_t msgobj_num, uint32_t id, uint32_t mask, bool extended);


/**
 * @brief Setup HW filter to receive all extended frames ID (0-0x1FFFFFFF).
 *
 *        This setups HW filter for received CAN message.
 *        The filter matches when <recieved_id> & mask == id & mask.
 *        Non-matching messages are dropped.
 *
 * @param msgobj_num ... number of message object (0-31).
 */
void CAN_recv_filter_all_ext(uint8_t msgobj_num);


/**
 * @brief Send one time CAN message.
 *
 * @param msgobj_num ... number of message object (0-31).
 * @param id ... CAN arbitration ID.
 * @param data ... pointer to data to send.
 * @param size ... size of data.
 */
void CAN_send_once(uint8_t msgobj_num, uint32_t id, uint8_t * data, uint8_t size);


/**
* @brief Send test message on CAN.
*
*/
void CAN_send_test(void);


/**
* @brief CAN interrupt handler.
*
*        The CAN interrupt handler must be provided by the user application.
*        It's function is to call the handler located in the ROM.
*/
void CAN_IRQHandler(void);


#endif /* CAN_DRIVER_H_ */
