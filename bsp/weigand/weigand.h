/*
 * weigand.h
 *
 *  Created on: 24. 8. 2018
 *      Author: Petr
 */

#ifndef BSP_WEIGAND_H_
#define BSP_WEIGAND_H_

#include <stdbool.h>
#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"
#include "stream_buffer.h"

#define WEIGAND26_FRAME_SIZE 26
#define WEIGAND26_FRAME_TIME_LIMIT 100 //ms

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
} weigand26_frame_t;

// Buffer item type required from consumer of this driver to be used
typedef struct
{
  uint8_t source;
  weigand26_frame_t frame;
} weigand26_buff_item_t;

#define WEIGAND26_BUFF_ITEM_SIZE sizeof(weigand26_buff_item_t)

//One buffer for each receiving component is preferred
void weigand_init(StreamBufferHandle_t buffer, uint8_t id, uint8_t dx_port, uint8_t d0_pin, uint8_t d1_pin);

void weigand_disable(uint8_t dx_port, uint8_t d0_pin, uint8_t d1_pin);

uint32_t weigand_get_facility(weigand26_frame_t frame);

uint32_t weigand_get_card(weigand26_frame_t frame);

bool weigand_is_parity_ok(weigand26_frame_t frame);

#endif /* BSP_WEIGAND_H_ */
