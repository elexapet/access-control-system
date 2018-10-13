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


typedef struct {
	uint32_t frame_buffer;
	uint8_t frame_buffer_ptr;
	uint8_t port;
	uint8_t pin_d0;
	uint8_t pin_d1;
}WEIGAND_T;


WEIGAND_T * weigand_init(uint8_t dx_port, uint8_t d0_pin, uint8_t d1_pin);

bool weigand_pending_frame(WEIGAND_T * instance);

uint32_t weigand_get_frame(WEIGAND_T * instance);

uint32_t weigand_get_facility(uint32_t frame);

uint32_t weigand_get_card(uint32_t frame);

bool weigand_parity_ok(uint32_t frame);

void weigand_callback(uint8_t port, uint32_t frame);

#endif /* BSP_WEIGAND_H_ */
