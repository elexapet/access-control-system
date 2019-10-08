/**
 *  @file
 *  @brief Terminal client for access control system (ACS).
 *
 *  @author Petr Elexa
 *  @see LICENSE
 *
 */

#ifndef APP_TERMINAL_H_
#define APP_TERMINAL_H_

#include <reader.h>
#include <stdint.h>

/**
* @brief Initialize terminal
*
* Will initialize terminal configuration and all readers and create terminal task.
*
* @param reader_cfg
* @param id
*/
void terminal_init(void);

/**
* @brief Reconfigure terminal's reader at runtime.
*
* @param reader_cfg
* @param id
*/
void terminal_reconfigure(reader_conf_t * reader_cfg, uint8_t id);

/**
* @brief CAN receive callback.
*
* Function is executed by the Callback handler after a CAN message has been received.
*
* @param msg_obj_num Contains the number of the message object that triggered
*                    the CAN receive callback.
*/
void term_can_recv(uint8_t msg_obj_num);

/**
* @brief CAN transmit callback.
*
* Function is executed by the Callback handler after a CAN message has been transmitted.
*
* @param msg_obj_num Contains the number of the message object that triggered
*                    the CAN transmit callback.
*/
void term_can_send(uint8_t msg_obj_num);

/**
* @brief CAN error callback.
*
* Function is executed by the Callback handler after an error has occurred on the CAN bus.
*
* @param error_info Contains the error code that triggered the CAN error callback.
*/
void term_can_error(uint32_t error_info);

#endif /* APP_TERMINAL_H_ */
