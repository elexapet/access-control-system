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

typedef union{
	struct{
		uint32_t odd_parity: 1;
		uint32_t card_number : 16;
		uint32_t facility_code : 8;
		uint32_t even_parity: 1;
		uint32_t : 6; //padding
	};
	uint32_t value;
}weigand26_frame_t;

#define CONSUMER_BUFF_SIZE sizeof(weigand26_frame_t)

typedef struct {
	weigand26_frame_t frame_buffer;
	uint8_t frame_buffer_ptr;
	uint8_t port;
	uint8_t pin_d0;
	uint8_t pin_d1;
	StreamBufferHandle_t consumer_buffer;
}weigand26_t;


void weigand_init(StreamBufferHandle_t buffer, uint8_t dx_port, uint8_t d0_pin, uint8_t d1_pin);

void weigand_disable(uint8_t dx_port, uint8_t d0_pin, uint8_t d1_pin);

bool weigand_pending_frame(weigand26_t * instance);

weigand26_frame_t weigand_get_frame(weigand26_t * instance);

uint32_t weigand_get_facility(weigand26_frame_t frame);

uint32_t weigand_get_card(weigand26_frame_t frame);

bool weigand_is_parity_ok(weigand26_frame_t frame);

#endif /* BSP_WEIGAND_H_ */
