/*
 *
 *  Created on: 24. 8. 2018
 *      Author: Petr
 */

#include "terminal.h"
#include "board.h"
#include "weigand.h"
#include "FreeRTOS.h"
#include "task.h"
#include "stream_buffer.h"
#include <stdio.h>
#include <string.h>


panel_conf_t acc_panel[DOOR_ACC_PANEL_MAX_COUNT] =
{
  {
    .acc_panel_on = DOOR_0_ACC_PANEL_ON,
    .acc_panel_port = DOOR_0_ACC_PANEL_PORT,
    .acc_panel_d0_pin = DOOR_0_ACC_PANEL_D0_PIN,
    .acc_panel_d1_pin = DOOR_0_ACC_PANEL_D1_PIN,
    .acc_panel_beep_port = DOOR_0_ACC_PANEL_BEEP_PORT,
    .acc_panel_beep_pin = DOOR_0_ACC_PANEL_BEEP_PIN,
    .acc_panel_gled_port = DOOR_0_ACC_PANEL_GLED_PORT,
    .acc_panel_gled_pin = DOOR_0_ACC_PANEL_GLED_PIN,
    .relay_port = DOOR_0_RELAY_PORT,
    .relay_pin = DOOR_0_RELAY_PIN,
    .open_time_sec = DOOR_0_OPEN_TIME_MS,
    .card_stream = NULL,
    .timer = NULL,
  },
  {
    .acc_panel_on = DOOR_1_ACC_PANEL_ON,
    .acc_panel_port = DOOR_1_ACC_PANEL_PORT,
    .acc_panel_d0_pin = DOOR_1_ACC_PANEL_D0_PIN,
    .acc_panel_d1_pin = DOOR_1_ACC_PANEL_D1_PIN,
    .acc_panel_beep_port = DOOR_1_ACC_PANEL_BEEP_PORT,
    .acc_panel_beep_pin = DOOR_1_ACC_PANEL_BEEP_PIN,
    .acc_panel_gled_port = DOOR_1_ACC_PANEL_GLED_PORT,
    .acc_panel_gled_pin = DOOR_1_ACC_PANEL_GLED_PIN,
    .relay_port = DOOR_1_RELAY_PORT,
    .relay_pin = DOOR_1_RELAY_PIN,
    .open_time_sec = DOOR_1_OPEN_TIME_MS,
    .card_stream = NULL,
    .timer = NULL,
  },
  {
    .acc_panel_on = DOOR_2_ACC_PANEL_ON,
    .acc_panel_port = DOOR_2_ACC_PANEL_PORT,
    .acc_panel_d0_pin = DOOR_2_ACC_PANEL_D0_PIN,
    .acc_panel_d1_pin = DOOR_2_ACC_PANEL_D1_PIN,
    .acc_panel_beep_port = DOOR_2_ACC_PANEL_BEEP_PORT,
    .acc_panel_beep_pin = DOOR_2_ACC_PANEL_BEEP_PIN,
    .acc_panel_gled_port = DOOR_2_ACC_PANEL_GLED_PORT,
    .acc_panel_gled_pin = DOOR_2_ACC_PANEL_GLED_PIN,
    .relay_port = DOOR_2_RELAY_PORT,
    .relay_pin = DOOR_2_RELAY_PIN,
    .open_time_sec = DOOR_2_OPEN_TIME_MS,
    .card_stream = NULL,
    .timer = NULL,
  }
};

static void terminal_user_authorized(uint8_t panel_id)
{
  panel_unlock(panel_id);
}

static void terminal_user_not_authorized(uint8_t panel_id)
{
  (void)panel_id;
}

static void terminal_process_card(uint8_t facility_code, uint16_t user_number, uint8_t panel_id)
{
  #ifdef DEVEL_BOARD
  //printf("Facility: %u Card: %u Parity: %u\n", facility_code, user_number);
  Board_LED_Set(1, true);
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  Board_LED_Set(1, false);

  if (facility_code == 12 && user_number == 27762) terminal_user_authorized(panel_id);
  #endif
}

static void terminal_task(void *pvParameters)
{
  (void)pvParameters;

  while (true)
  {
    weigand26_frame_t card_data;
    size_t bytes_got;

    if (acc_panel[ACC_PANEL_0].acc_panel_on)
    {
      bytes_got = xStreamBufferReceive(acc_panel[ACC_PANEL_0].card_stream, &card_data, sizeof card_data, pdMS_TO_TICKS(0));
      if (bytes_got == sizeof card_data && weigand_is_parity_ok(card_data))
      {
        terminal_process_card(card_data.facility_code, card_data.card_number, ACC_PANEL_0);
      }
    }
    if (acc_panel[ACC_PANEL_1].acc_panel_on)
    {
      bytes_got = xStreamBufferReceive(acc_panel[ACC_PANEL_1].card_stream, &card_data, sizeof card_data, pdMS_TO_TICKS(0));
      if (bytes_got == sizeof card_data && weigand_is_parity_ok(card_data))
      {
        terminal_process_card(card_data.facility_code, card_data.card_number, ACC_PANEL_1);
      }
    }
    if (acc_panel[ACC_PANEL_2].acc_panel_on)
    {
      bytes_got = xStreamBufferReceive(acc_panel[ACC_PANEL_2].card_stream, &card_data, sizeof card_data, pdMS_TO_TICKS(0));
      if (bytes_got == sizeof card_data && weigand_is_parity_ok(card_data))
      {
        terminal_process_card(card_data.facility_code, card_data.card_number, ACC_PANEL_2);
      }
    }
  }
}

void terminal_init(void)
{
  for (size_t id = 0; id < DOOR_ACC_PANEL_MAX_COUNT; ++id)
  {
    if (acc_panel[id].acc_panel_on) terminal_reconfigure(NULL, id);
  }

  xTaskCreate(terminal_task, "term_tsk", configMINIMAL_STACK_SIZE + 128, NULL, (tskIDLE_PRIORITY + 1UL), NULL);
}

void terminal_reconfigure(panel_conf_t * panel_cfg, uint8_t panel_id)
{
  if (panel_id >= DOOR_ACC_PANEL_MAX_COUNT) return;

  portENTER_CRITICAL();

  if (panel_cfg != NULL)
  {
    memcpy(&acc_panel[panel_id], panel_cfg, sizeof(acc_panel[panel_id]));

    //disable interface
    panel_deinit(panel_id);
  }

  portEXIT_CRITICAL();

  //reconfigure interface to card reader
  if (acc_panel[panel_id].acc_panel_on)
  {
    panel_init(panel_id);
  }
}
