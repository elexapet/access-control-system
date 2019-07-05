/*
 *
 *  Created on: 24. 8. 2018
 *      Author: Petr
 */

#include "terminal.h"
#include "board.h"
#include "weigand.h"
#include "static_cache.h"
#include "FreeRTOS.h"
#include "task.h"
#include "stream_buffer.h"
#include "can/can_term_driver.h"
#include "can/can_term_protocol.h"
#include <stdio.h>
#include <string.h>


typedef cache_item_t term_cache_item_t; // 4B


static void terminal_user_authorized(uint8_t panel_id)
{
  // TODO
  DEBUGSTR("auth OK\n");
  panel_unlock(panel_id, BEEP_ON_SUCCESS, OK_LED_ON_SUCCESS);
}

static void terminal_user_not_authorized(uint8_t panel_id)
{
  //TODO
  (void)panel_id;
  DEBUGSTR("auth FAIL\n");
}

// This is called from ISR - do stuff quickly
void terminal_can_proto_handler(uint32_t msg_head, uint8_t * data, uint8_t dlc)
{

  if (AUTH_OK)
  {
    terminal_user_authorized(panel_id);
    #ifdef CACHING_ENABLED
      static_cache_insert(user);
    #endif
  }
  else if (AUTH_FAIL)
  {
    terminal_user_not_authorized(panel_id);
  }
}

static void terminal_register_user(uint32_t user_id, uint8_t panel_id)
{
  // TODO send request on CAN
}

static void terminal_request_auth(uint32_t user_id, uint8_t panel_id)
{
  // TODO send request on CAN
}


static void terminal_user_identified(uint32_t user_id, uint8_t panel_id)
{
  term_cache_item_t user;
  user.key = user_id;

  if (panel_id < DOOR_ACC_PANEL_COUNT)
  {
    if (panel_conf[panel_id].learn_mode)
    {
      user.value = panel_id;
      terminal_register_user(user_id, panel_id);
    }
    else if (static_cache_get(&user) && user.panel == panel_id)
    {
      terminal_user_authorized(panel_id);
    }
    else
    {
      terminal_request_auth(user_id, panel_id);
    }
  }
}

static void terminal_task(void *pvParameters)
{
  (void)pvParameters;

  while (true)
  {
    uint32_t user_id;
    uint8_t panel_id = panel_get_request_from_buffer(&user_id);
    if (panel_id < DOOR_ACC_PANEL_COUNT)
    {
      terminal_user_identified(user_id, panel_id);
      DEBUGSTR("user identified\n");
    }
  }
}

void terminal_init(void)
{
  //register handler
  CAN_frame_callback = terminal_can_proto_handler;
  CAN_receive_all_frames();

  for (size_t id = 0; id < DOOR_ACC_PANEL_COUNT; ++id)
  {
    if (panel_conf[id].acc_panel_on) terminal_reconfigure(NULL, id);
  }

  xTaskCreate(terminal_task, "term_tsk", configMINIMAL_STACK_SIZE + 128, NULL, (tskIDLE_PRIORITY + 1UL), NULL);

#ifdef DEVEL_BOARD
  static_cache_insert(static_cache_convert((814204 << 1) | 0));
  static_cache_insert(static_cache_convert((814199 << 1) | 1));
#endif
}

void terminal_reconfigure(panel_conf_t * panel_cfg, uint8_t panel_id)
{
  if (panel_id >= DOOR_ACC_PANEL_COUNT) return;

  portENTER_CRITICAL();

  if (panel_cfg != NULL)
  {
    memcpy(&panel_conf[panel_id], panel_cfg, sizeof(panel_conf_t));

    //reconfigure interface to card reader
    if (panel_conf[panel_id].acc_panel_on)
    {
      panel_init(panel_id);
      DEBUGSTR("panel enabled\n");
    }
    else
    {
      //disable interface
      panel_deinit(panel_id);
      DEBUGSTR("panel disabled\n");
    }
  }
  else
  {
    panel_init(panel_id);
  }

  portEXIT_CRITICAL();
}
