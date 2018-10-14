/*
 * weigand.c
 *
 *  Created on: 24. 8. 2018
 *      Author: Petr
 *
 *  W26 frame | even parity (1b) | facility code (8b) | card number (16b) | odd parity (1b) | takes up-to ~52ms
 *
 *	data pulse 50us
 *	data interval 2ms
 */

#include "weigand.h"


static weigand_t instance[4];


weigand_t * weigand_init(uint8_t dx_port, uint8_t d0_pin, uint8_t d1_pin)
{
	if (dx_port > 3) return NULL;

	instance[dx_port].port = dx_port;
	instance[dx_port].pin_d0 = d0_pin;
	instance[dx_port].pin_d1 = d1_pin;
	instance[dx_port].frame_buffer.value = 0;
	instance[dx_port].frame_buffer_ptr = WEIGAND_TRANS_BITS;

	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO3_1, IOCON_MODE_PULLUP);
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO3_2, IOCON_MODE_PULLUP);

	Chip_GPIO_SetPinDIRInput(LPC_GPIO, dx_port, d0_pin);
	Chip_GPIO_SetPinDIRInput(LPC_GPIO, dx_port, d1_pin);

	Chip_GPIO_SetupPinInt(LPC_GPIO, dx_port, d0_pin, GPIO_INT_FALLING_EDGE);
	Chip_GPIO_SetupPinInt(LPC_GPIO, dx_port, d1_pin, GPIO_INT_FALLING_EDGE);

	Chip_GPIO_ClearInts(LPC_GPIO, dx_port, (1 << d0_pin));
	Chip_GPIO_ClearInts(LPC_GPIO, dx_port, (1 << d1_pin));

	Chip_GPIO_EnableInt(LPC_GPIO, dx_port, (1 << d0_pin) | (1 << d1_pin));

	return &instance[dx_port];
}

bool weigand_pending_frame(weigand_t * instance)
{
	return instance->frame_buffer_ptr == 0;
}

weigand26_frame_t weigand_get_frame(weigand_t * instance)
{
	return instance->frame_buffer;
}

uint32_t weigand_get_facility(weigand26_frame_t frame)
{
	return (frame.value & 0x1FE0000) >> 17;
}

uint32_t weigand_get_card(weigand26_frame_t frame)
{
	return (frame.value & 0x1FFFE) >> 1;
}

bool weigand_parity_ok(weigand26_frame_t frame)
{
	uint8_t even_parity = __builtin_parity(frame.value & 0x1FFE000);
	uint8_t odd_parity = __builtin_parity(frame.value & 0x1FFE) ^ 1;
	return even_parity == ((frame.value >> 25) & 0x1) && odd_parity == (frame.value & 0x1);
}

void weigand_int_handler(weigand_t * instance)
{
	uint32_t int_states = Chip_GPIO_GetMaskedInts(LPC_GPIO, instance->port);
	Chip_GPIO_ClearInts(LPC_GPIO, instance->port, (1 << instance->pin_d1));
	Chip_GPIO_ClearInts(LPC_GPIO, instance->port, (1 << instance->pin_d0));

	if (int_states & (1 << instance->pin_d1))
	{
		if (Chip_GPIO_ReadPortBit(LPC_GPIO, instance->port, instance->pin_d1) == 0)
		{
			instance->frame_buffer_ptr--;
			instance->frame_buffer.value |= (1 << instance->frame_buffer_ptr);
		}
	}
	else if (int_states & (1 << instance->pin_d0))
	{
		if (Chip_GPIO_ReadPortBit(LPC_GPIO, instance->port, instance->pin_d0) == 0)
		{
			instance->frame_buffer_ptr--;
			instance->frame_buffer.value &= ~(1 << instance->frame_buffer_ptr);
		}
	}
	else
	{
		Chip_GPIO_ClearInts(LPC_GPIO, instance->port, 0xFFFFFFFF);
	}
	if (instance->frame_buffer_ptr == 0)
	{
		//TODO move from int handler
		//TODO check parity (not here)
		weigand_callback(instance->port, instance->frame_buffer);
		instance->frame_buffer_ptr = WEIGAND_TRANS_BITS;
		instance->frame_buffer.value = 0;
	}
}

void PIOINT1_IRQHandler(void)
{
	NVIC_ClearPendingIRQ(EINT1_IRQn);
	weigand_int_handler(&instance[1]);
}

void PIOINT2_IRQHandler(void)
{
	NVIC_ClearPendingIRQ(EINT2_IRQn);
	weigand_int_handler(&instance[2]);
}

void PIOINT3_IRQHandler(void)
{
	NVIC_ClearPendingIRQ(EINT3_IRQn);
	weigand_int_handler(&instance[3]);
}

