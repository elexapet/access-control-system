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
#include "board.h"
#include <limits.h>
#include <assert.h>


static weigand26_t device[WEIGAND_DEVICE_LIMIT];


void weigand_init(StreamBufferHandle_t buffer, uint8_t dx_port, uint8_t d0_pin, uint8_t d1_pin)
{
	if (dx_port > 3) return;

	//Save device information
	device[dx_port].port = dx_port;
	device[dx_port].pin_d0 = d0_pin;
	device[dx_port].pin_d1 = d1_pin;
	device[dx_port].frame_buffer.value = 0;
	device[dx_port].frame_buffer_ptr = WEIGAND26_FRAME_SIZE;

	//Enable pull-ups
	Chip_IOCON_PinMux(LPC_IOCON, CHIP_IOCON_PIO[dx_port][d0_pin], IOCON_MODE_PULLUP, IOCON_FUNC0);
	Chip_IOCON_PinMux(LPC_IOCON, CHIP_IOCON_PIO[dx_port][d1_pin], IOCON_MODE_PULLUP, IOCON_FUNC0);

	//Input mode
	Chip_GPIO_SetPinDIRInput(LPC_GPIO, dx_port, d0_pin);
	Chip_GPIO_SetPinDIRInput(LPC_GPIO, dx_port, d1_pin);

	//Trigger on falling edge
	Chip_GPIO_SetupPinInt(LPC_GPIO, dx_port, d0_pin, GPIO_INT_FALLING_EDGE);
	Chip_GPIO_SetupPinInt(LPC_GPIO, dx_port, d1_pin, GPIO_INT_FALLING_EDGE);

	//Clear INTs
	Chip_GPIO_ClearInts(LPC_GPIO, dx_port, (1 << d0_pin));
	Chip_GPIO_ClearInts(LPC_GPIO, dx_port, (1 << d1_pin));

	//Create stream
	device[dx_port].consumer_buffer = buffer;

	//Enable INT for both pins
	Chip_GPIO_EnableInt(LPC_GPIO, dx_port, (1 << d0_pin) | (1 << d1_pin));
}

void weigand_disable(uint8_t dx_port, uint8_t d0_pin, uint8_t d1_pin)
{
  Chip_GPIO_DisableInt(LPC_GPIO, dx_port, (1 << d0_pin) | (1 << d1_pin));
}

bool weigand_pending_frame(weigand26_t * device)
{
	return device->frame_buffer_ptr == 0;
}

weigand26_frame_t weigand_get_frame(weigand26_t * device)
{
	return device->frame_buffer;
}

uint32_t weigand_get_facility(weigand26_frame_t frame)
{
	return (frame.value & 0x1FE0000) >> 17;
}

uint32_t weigand_get_card(weigand26_frame_t frame)
{
	return (frame.value & 0x1FFFE) >> 1;
}

bool weigand_is_parity_ok(weigand26_frame_t frame)
{
	uint8_t even_parity = __builtin_parity(frame.value & 0x1FFE000);
	uint8_t odd_parity = __builtin_parity(frame.value & 0x1FFE) ^ 1;
	return even_parity == ((frame.value >> 25) & 0x1) && odd_parity == (frame.value & 0x1);
}

void weigand_int_handler(weigand26_t * device)
{
	uint32_t int_states = Chip_GPIO_GetMaskedInts(LPC_GPIO, device->port);
	//Clear int flag on each pin
	Chip_GPIO_ClearInts(LPC_GPIO, device->port, (1 << device->pin_d1));
	Chip_GPIO_ClearInts(LPC_GPIO, device->port, (1 << device->pin_d0));

	//Resolve bit value
	if (int_states & (1 << device->pin_d1))
	{
		if (Chip_GPIO_ReadPortBit(LPC_GPIO, device->port, device->pin_d1) == 0)
		{
			device->frame_buffer_ptr--;
			device->frame_buffer.value |= (1 << device->frame_buffer_ptr);
		}
	}
	else if (int_states & (1 << device->pin_d0))
	{
		if (Chip_GPIO_ReadPortBit(LPC_GPIO, device->port, device->pin_d0) == 0)
		{
			device->frame_buffer_ptr--;
			device->frame_buffer.value &= ~(1 << device->frame_buffer_ptr);
		}
	}
	else
	{
		Chip_GPIO_ClearInts(LPC_GPIO, device->port, 0xFFFFFFFF);
	}
	//Whole frame received
	if (device->frame_buffer_ptr == 0)
	{
		xStreamBufferReset(device->consumer_buffer);

		BaseType_t pxHigherPriorityTaskWoken = pdFALSE;
		size_t bytes_sent = xStreamBufferSendFromISR(
				device->consumer_buffer,
				&device->frame_buffer,
				sizeof(weigand26_frame_t),
				&pxHigherPriorityTaskWoken);

		//Stream buffer should be empty because we reset it
		assert(bytes_sent == sizeof(weigand26_frame_t));

		device->frame_buffer_ptr = WEIGAND26_FRAME_SIZE;
		device->frame_buffer.value = 0;

		portYIELD_FROM_ISR(pxHigherPriorityTaskWoken);
	}
}

//GPIO port 1 handler
void PIOINT0_IRQHandler(void)
{
	NVIC_ClearPendingIRQ(EINT1_IRQn);
	weigand_int_handler(&device[0]);
}

//GPIO port 1 handler
void PIOINT1_IRQHandler(void)
{
	NVIC_ClearPendingIRQ(EINT1_IRQn);
	weigand_int_handler(&device[1]);
}

//GPIO port 2 handler
void PIOINT2_IRQHandler(void)
{
	NVIC_ClearPendingIRQ(EINT2_IRQn);
	weigand_int_handler(&device[2]);
}

//GPIO port 3 handler
void PIOINT3_IRQHandler(void)
{
	NVIC_ClearPendingIRQ(EINT3_IRQn);
	weigand_int_handler(&device[3]);
}

