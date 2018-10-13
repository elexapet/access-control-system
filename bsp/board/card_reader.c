/*
 *
 *  Created on: 24. 8. 2018
 *      Author: Petr
 */

#include "weigand.h"



void weigand_callback(uint8_t port, uint32_t frame)
{
	//0x3FFFFFF
	printf("Facility: %u\n", weigand_get_facility(frame));
	printf("Card: %u\n", weigand_get_card(frame));
	//printf("Frame: 0x%X\n", ((frame & 0x1FFFFFE) >> 1));
}


