/*
 * weigand.h
 *
 *  Created on: 24. 8. 2018
 *      Author: Petr
 */

#ifndef BSP_WEIGAND_H_
#define BSP_WEIGAND_H_

#include "stdbool.h"
#include "stdint.h"
#include "limits.h"
#include "chip.h"
#include "board_config.h"

typedef union{
	struct{
		uint32_t odd_parity: 1;
		uint32_t card_number : 16;
		uint32_t facility_code : 8;
		uint32_t even_parity: 1;
		uint32_t : 6;
	};
	uint32_t value;
}weigand26_frame_t;

typedef struct {
	weigand26_frame_t frame_buffer;
	uint8_t frame_buffer_ptr;
	uint8_t port;
	uint8_t pin_d0;
	uint8_t pin_d1;
}weigand_t;


weigand_t * weigand_init(uint8_t dx_port, uint8_t d0_pin, uint8_t d1_pin);

bool weigand_pending_frame(weigand_t * instance);

weigand26_frame_t weigand_get_frame(weigand_t * instance);

uint32_t weigand_get_facility(weigand26_frame_t frame);

uint32_t weigand_get_card(weigand26_frame_t frame);

bool weigand_parity_ok(weigand26_frame_t frame);

void weigand_callback(uint8_t port, weigand26_frame_t frame);

#endif /* BSP_WEIGAND_H_ */
