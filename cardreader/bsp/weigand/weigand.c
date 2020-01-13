/**
 *  @file
 *  @brief Wiegand26 interface driver.
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

#include "weigand.h"
#include "board.h"
#include "timers.h"
#include <limits.h>

typedef struct
{
  weigand26_frame_t frame_buffer;
  StreamBufferHandle_t consumer_buffer;
  TimerHandle_t timer;
  uint8_t frame_buffer_ptr;
  uint8_t port;
  uint8_t pin_d0;
  uint8_t pin_d1;
  uint8_t id;
} weigand26_t;

static weigand26_t device[WEIGAND_DEVICE_LIMIT] = {0};

static void weigand_frame_timeout(TimerHandle_t pxTimer)
{
  configASSERT(pxTimer);

  // Which timer expired
  uint32_t id = (int32_t) pvTimerGetTimerID(pxTimer);

  // Atomicity is guaranteed because when weigand_int_handler is invoked
  // weigand_frame_timeout will not be called. But we need to prevent
  // weigand_int_handler from being invoked.
  portENTER_CRITICAL();
	// Empty the frame buffer because next bit period expired
	device[id].frame_buffer.value = 0;
	device[id].frame_buffer_ptr = WEIGAND26_FRAME_SIZE;
  portEXIT_CRITICAL();
}

void weigand_init(StreamBufferHandle_t buffer, uint8_t id, uint8_t dx_port, uint8_t d0_pin, uint8_t d1_pin)
{
	configASSERT(dx_port == 2 || dx_port == 3);

	//Save device information
	device[dx_port].consumer_buffer = buffer;
	device[dx_port].port = dx_port;
	device[dx_port].pin_d0 = d0_pin;
	device[dx_port].pin_d1 = d1_pin;
	device[dx_port].frame_buffer.value = 0;
	device[dx_port].frame_buffer_ptr = WEIGAND26_FRAME_SIZE;
	device[dx_port].id = id;

	//Normal function
	Chip_IOCON_PinMux(LPC_IOCON, CHIP_IOCON_PIO[dx_port][d0_pin], IOCON_MODE_INACT, IOCON_FUNC0);
	Chip_IOCON_PinMux(LPC_IOCON, CHIP_IOCON_PIO[dx_port][d1_pin], IOCON_MODE_INACT, IOCON_FUNC0);

	// LPC_GPIO is initialized in board.c

	//Input mode
	Chip_GPIO_SetPinDIRInput(LPC_GPIO, dx_port, d0_pin);
	Chip_GPIO_SetPinDIRInput(LPC_GPIO, dx_port, d1_pin);

	//Trigger on falling edge
	Chip_GPIO_SetupPinInt(LPC_GPIO, dx_port, d0_pin, GPIO_INT_FALLING_EDGE);
	Chip_GPIO_SetupPinInt(LPC_GPIO, dx_port, d1_pin, GPIO_INT_FALLING_EDGE);

	//Clear INTs
	Chip_GPIO_ClearInts(LPC_GPIO, dx_port, (1 << d0_pin));
	Chip_GPIO_ClearInts(LPC_GPIO, dx_port, (1 << d1_pin));

	//Create timer
	if (device[dx_port].timer == NULL)
  {
	  device[dx_port].timer = xTimerCreate("WG", (WEIGAND26_MAX_BIT_PERIOD / portTICK_PERIOD_MS), pdFALSE, (void *)(uint32_t) dx_port, weigand_frame_timeout);
  }
  configASSERT(device[dx_port].timer);

	//Enable INT for both pins
	Chip_GPIO_EnableInt(LPC_GPIO, dx_port, (1 << d0_pin) | (1 << d1_pin));
}

void weigand_disable(uint8_t dx_port, uint8_t d0_pin, uint8_t d1_pin)
{
  configASSERT(dx_port == 2 || dx_port == 3);

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

static inline void _refresh_frame_time(weigand26_t * device)
{
  configASSERT(xTimerStart(device->timer, 0)); // Restart frame expiration timer
}

static inline void _stop_frame_time(weigand26_t * device)
{
  configASSERT(xTimerStop(device->timer, 0)); // Stop frame expiration timer
}

void weigand_int_handler(weigand26_t * device)
{
	uint32_t int_states = Chip_GPIO_GetMaskedInts(LPC_GPIO, device->port);
	//Clear int flag on all pins
	Chip_GPIO_ClearInts(LPC_GPIO, device->port, 0xFFFFFFFF);

	//Resolve pin
	if (int_states & (1 << device->pin_d1)) // 1's
	{
		if (Chip_GPIO_ReadPortBit(LPC_GPIO, device->port, device->pin_d1) == 0)
		{
		  _refresh_frame_time(device);
			device->frame_buffer_ptr--;
			device->frame_buffer.value |= (1 << device->frame_buffer_ptr);
		}
	}
	else if (int_states & (1 << device->pin_d0)) // 0's
	{
		if (Chip_GPIO_ReadPortBit(LPC_GPIO, device->port, device->pin_d0) == 0)
		{
			_refresh_frame_time(device);
			device->frame_buffer_ptr--;
			device->frame_buffer.value &= ~(1 << device->frame_buffer_ptr);
		}
	}
	else
	{
		return;
	}
	//Whole frame received
	if (device->frame_buffer_ptr == 0)
	{
		_stop_frame_time(device);

	  BaseType_t pxHigherPriorityTaskWoken = pdFALSE;

	  // Check if not full
		if (xStreamBufferIsFull(device->consumer_buffer) == pdFALSE)
		{
		  weigand26_buff_item_t item_to_send = {
		      device->id,
		      device->frame_buffer
		  };

		  // Send data
      size_t bytes_sent = xStreamBufferSendFromISR(
          device->consumer_buffer,
          &item_to_send,
          WEIGAND26_BUFF_ITEM_SIZE,
          &pxHigherPriorityTaskWoken);

      //Stream buffer should have had enough space (we checked)
      configASSERT(bytes_sent == WEIGAND26_BUFF_ITEM_SIZE);
		}
		// Empty the frame buffer
		device->frame_buffer_ptr = WEIGAND26_FRAME_SIZE;
		device->frame_buffer.value = 0;

		// Wake potentially blocked higher priority task
		portYIELD_FROM_ISR(pxHigherPriorityTaskWoken);
	}
}



//GPIO port 2 handler
void PIOINT2_IRQHandler(void)
{
	NVIC_ClearPendingIRQ(EINT2_IRQn);
  if (device[2].consumer_buffer != NULL)
  {
    weigand_int_handler(&device[2]);
  }
}

//GPIO port 3 handler
void PIOINT3_IRQHandler(void)
{
	NVIC_ClearPendingIRQ(EINT3_IRQn);
  if (device[3].consumer_buffer != NULL)
  {
    weigand_int_handler(&device[3]);
  }
}

