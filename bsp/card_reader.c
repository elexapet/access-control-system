/*
 *
 *  Created on: 24. 8. 2018
 *      Author: Petr
 */

#include "card_reader.h"
#include "FreeRTOS.h"
#include "task.h"

static weigand26_frame_t _pending_frame = {.value = 0};

void weigand_callback(uint8_t port, weigand26_frame_t frame)
{
	_pending_frame.value = frame.value;
}

static void card_reader_task(void *pvParameters)
{
	weigand26_frame_t frame;
	uint8_t port;

	 printf("started\n");

	while(1)
	{
		vTaskDelay(150 / portTICK_PERIOD_MS);

		if (_pending_frame.value == 0) continue;

		portENTER_CRITICAL();
		frame.value = _pending_frame.value;
		_pending_frame.value = 0;
		portEXIT_CRITICAL();

		//printf("Frame: 0x%X\n", ((frame & 0x1FFFFFE) >> 1));
		printf("Facility: %u Card: %u Parity: %u\n", frame.facility_code, frame.card_number, weigand_parity_ok(frame));
	}
}

void card_reader_init(void)
{
	weigand_init(CARD_READER1_PORT, CARD_READER1_D0_PIN, CARD_READER1_D1_PIN);
	xTaskCreate(card_reader_task, "card_task", configMINIMAL_STACK_SIZE + 64, NULL, (tskIDLE_PRIORITY + 1UL), NULL);
}
