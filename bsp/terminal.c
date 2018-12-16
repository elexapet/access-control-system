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
#include <stdio.h>

#define ACC_PANEL_0 0
#define ACC_PANEL_1 1
#define ACC_PANEL_2 2

static StreamBufferHandle_t _cr_stream[DOOR_ACC_PANEL_MAX_COUNT] = {0};
static panel_conf_t _acc_panel[DOOR_ACC_PANEL_MAX_COUNT] =
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
    .open_time_sec = DOOR_0_OPEN_TIME_SEC,
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
    .open_time_sec = DOOR_1_OPEN_TIME_SEC,
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
    .open_time_sec = DOOR_2_OPEN_TIME_SEC,
  }
};

static void terminal_process_card(uint8_t facility_code, uint16_t card_number, uint8_t reader_id)
{
  #ifdef DEVEL_BOARD
  printf("Facility: %u Card: %u Parity: %u\n", facility_code, card_number);
  Board_LED_Set(1, true);
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  Board_LED_Set(1, false);
  #endif
}

static void terminal_reader_task(void *pvParameters)
{
  //printf("test");

  (void)pvParameters;

  while (true)
  {
    weigand26_frame_t card_data;
    size_t bytes_got;

    if (_acc_panel[ACC_PANEL_0].acc_panel_on)
    {
      bytes_got = xStreamBufferReceive(_cr_stream[0], &card_data, sizeof card_data, pdMS_TO_TICKS(0));
      if (bytes_got == sizeof card_data && weigand_is_parity_ok(card_data))
      {
        terminal_process_card(card_data.facility_code, card_data.card_number, ACC_PANEL_0);
      }
    }
    if (_acc_panel[ACC_PANEL_1].acc_panel_on)
    {
      bytes_got = xStreamBufferReceive(_cr_stream[1], &card_data, sizeof card_data, pdMS_TO_TICKS(0));
      if (bytes_got == sizeof card_data && weigand_is_parity_ok(card_data))
      {
        terminal_process_card(card_data.facility_code, card_data.card_number, ACC_PANEL_1);
      }
    }
    if (_acc_panel[ACC_PANEL_2].acc_panel_on)
    {
      bytes_got = xStreamBufferReceive(_cr_stream[2], &card_data, sizeof card_data, pdMS_TO_TICKS(0));
      if (bytes_got == sizeof card_data && weigand_is_parity_ok(card_data))
      {
        terminal_process_card(card_data.facility_code, card_data.card_number, ACC_PANEL_2);
      }
    }
  }
}

void terminal_init(void)
{
  for (int i = 0; i < DOOR_ACC_PANEL_MAX_COUNT; ++i)
  {
    if (_acc_panel[i].acc_panel_on) terminal_reconfigure(NULL, i);
  }

  xTaskCreate(terminal_reader_task, "term_tsk", configMINIMAL_STACK_SIZE + 128, NULL, (tskIDLE_PRIORITY + 1UL), NULL);
}

void terminal_reconfigure(panel_conf_t * acc_panel, uint8_t id)
{
  if (id >= DOOR_ACC_PANEL_MAX_COUNT) return;

  portENTER_CRITICAL();
  if (acc_panel != NULL)
  {
    memcpy(&_acc_panel[id], acc_panel, sizeof(_acc_panel[id]));
  }
  //reconfigure interface to card reader
  if (acc_panel[id].acc_panel_on)
  {
    _cr_stream[id] = xStreamBufferCreate(CONSUMER_BUFF_SIZE, CONSUMER_BUFF_SIZE);
    configASSERT(_cr_stream[id]);
    weigand_init(_cr_stream[id], _acc_panel[id].acc_panel_port, _acc_panel[id].acc_panel_d0_pin, _acc_panel[id].acc_panel_d1_pin);
  }
  //disable interface
  else
  {
    vStreamBufferDelete(_cr_stream[id]);
    _cr_stream[id] = NULL;
    weigand_disable(_acc_panel[id].acc_panel_port, _acc_panel[id].acc_panel_d0_pin, _acc_panel[id].acc_panel_d1_pin);
  }
  portEXIT_CRITICAL();
}
