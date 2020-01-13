/**
 *  @file
 *  @brief Wiegand26 interface driver.
 *
 *
 *  ~490 b/s transfer rate
 *  data pulse nominal 40us (standard 20-100us)
 *  data interval nominal 2ms (standard 200us-20ms)
 *
 *  Only Wiegand 26bit format
 *  Frame:
 *  | even parity (1b) | facility code (8b) | card number (16b) | odd parity (1b) |
 *  transmission duration ~52ms
 *
 *  @author Petr Elexa
 *  @see LICENSE
 *
 */

#ifndef BSP_WEIGAND_H_
#define BSP_WEIGAND_H_

#include <stdbool.h>
#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"
#include "stream_buffer.h"

#define WEIGAND26_FRAME_SIZE 26
#define WEIGAND26_MAX_BIT_PERIOD 20  // Should be equal to upper limit of data interval.

typedef union
{
	struct
	{
		uint32_t odd_parity: 1;
		uint32_t card_number : 16;
		uint32_t facility_code : 8;
		uint32_t even_parity: 1;
		uint32_t : 6; //padding
	};
	uint32_t value;
} weigand26_frame_t; // 4B

// Buffer item type required from consumer of this driver to be used
typedef struct
{
  uint8_t source; // =interface identification
  weigand26_frame_t frame;
} weigand26_buff_item_t;

#define WEIGAND26_BUFF_ITEM_SIZE sizeof(weigand26_buff_item_t)

//
/**
 * @brief Initialize Wiegand driver.
 *
 * @note One buffer for each consumer is preferred.
 *
 * @param buffer ... Receive buffer for frames from RFID card/tags.
 *                   See weigand26_buff_item_t
 * @param id ... interface identification
 * @param dx_port ... Port number for data signals.
 * @param d0_pin ... Pin for 0's data signal.
 * @param d1_pin ... Pin for 1's data signal.
 */
void weigand_init(StreamBufferHandle_t buffer, uint8_t id, uint8_t dx_port, uint8_t d0_pin, uint8_t d1_pin);

/**
 * @brief Disable Wiegand driver.
 *
 * @param dx_port ... Port number for data signals.
 * @param d0_pin ... Pin for 0's data signal.
 * @param d1_pin ... Pin for 1's data signal.
 */
void weigand_disable(uint8_t dx_port, uint8_t d0_pin, uint8_t d1_pin);

/**
 * @brief Parse facility code from frame.
 *
 * @param frame ... Wiegand26 data frame.
 *
 * @return facility code
 */
uint32_t weigand_get_facility(weigand26_frame_t frame);

/**
 * @brief Parse card number from frame.
 *
 * @param frame ... Wiegand26 data frame.
 *
 * @return card number
 */
uint32_t weigand_get_card(weigand26_frame_t frame);

/**
 * @brief Check frame parity.
 *
 * @param frame ... Wiegand26 data frame.
 *
 * @return true if parity is valid
 */
bool weigand_is_parity_ok(weigand26_frame_t frame);

#endif /* BSP_WEIGAND_H_ */
