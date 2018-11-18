/*
 *
 *  Created on: 24. 8. 2018
 *      Author: Petr
 */

#include "terminal.h"
#include "weigand.h"
#include "board_config.h"
#include "FreeRTOS.h"
#include "task.h"
#include "stream_buffer.h"

static StreamBufferHandle_t cr_stream[CARD_READERS_MAX_COUNT];

static void terminal_process_card(uint16_t card_number, uint8_t facility_code, uint8_t reader_id)
{
	printf("Facility: %u Card: %u Parity: %u\n", facility_code, card_number);
}

static void card_reader_task(void *pvParameters)
{
	weigand26_frame_t card_data;
	size_t bytes_got;

	printf("card_task\n");

	while(true)
	{
		#if CARD_READER1
			bytes_got = xStreamBufferReceive(cr_stream[CARD_READER1_ID], &card_data, sizeof card_data, pdMS_TO_TICKS(0));
			if (bytes_got == sizeof card_data && weigand_is_parity_ok(card_data))
			{
				terminal_process_card(card_data.facility_code, card_data.card_number, CARD_READER1_ID);
			}
		#endif
		#if CARD_READER2
			bytes_got = xStreamBufferReceive(cr_stream[CARD_READER2_ID], &card_data, sizeof card_data, pdMS_TO_TICKS(0));
			if (bytes_got == sizeof card_data && weigand_is_parity_ok(card_data))
			{
				terminal_process_card(card_data.facility_code, card_data.card_number, CARD_READER2_ID);
			}
		#endif
		#if CARD_READER3
			bytes_got = xStreamBufferReceive(cr_stream[CARD_READER3_ID], &card_data, sizeof card_data, pdMS_TO_TICKS(0));
			if (bytes_got == sizeof card_data && weigand_is_parity_ok(card_data))
			{
				terminal_process_card(card_data.facility_code, card_data.card_number, CARD_READER3_ID);
			}
		#endif
	}
}

void card_reader_init(void)
{
	#if CARD_READER1
		cr_stream[CARD_READER1_ID] = weigand_init(CARD_READER1_PORT, CARD_READER1_D0_PIN, CARD_READER1_D1_PIN);
	#endif
	#if CARD_READER2
		cr_stream[CARD_READER2_ID] = weigand_init(CARD_READER2_PORT, CARD_READER2_D0_PIN, CARD_READER2_D1_PIN);
	#endif
	#if CARD_READER3
		cr_stream[CARD_READER3_ID] = weigand_init(CARD_READER3_PORT, CARD_READER3_D0_PIN, CARD_READER3_D1_PIN);
	#endif

	xTaskCreate(card_reader_task, "crd_tsk", configMINIMAL_STACK_SIZE + 64, NULL, (tskIDLE_PRIORITY + 1UL), NULL);
}
