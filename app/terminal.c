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
#include <stdio.h>
#include <string.h>

typedef union
{
#pragma pack(push,1)
  struct
  {
    uint32_t user_id : 24;
    uint32_t panel0 : 1;
    uint32_t panel1 : 1;
    uint32_t panel2 : 1;
    uint32_t : 5; // reserved for future
  };
#pragma pack(pop)
  cache_item_t as_cache_item;
  uint32_t as_bitvalue;
}term_cache_item_t; // 4B

static bool _learn_mode_panel[3] = {false,false,false};
static uint32_t _learn_enable_user_id = 7632370;

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

static void terminal_user_identified(uint32_t user_id, uint8_t panel_id)
{
  if (user_id == _learn_enable_user_id)
  {
    _learn_mode_panel[panel_id] = !_learn_mode_panel[panel_id];
    return;
  }

  term_cache_item_t user;
  user.user_id = user_id;

  if (_learn_mode_panel[panel_id])
  {
    user.as_bitvalue |= _BIT(panel_id) << 24;
    static_cache_insert(user.as_cache_item);
  }
  else if (static_cache_get(&user.as_cache_item))
  {
    if ((user.panel2 << 2 | user.panel1 << 1 | user.panel0) & _BIT(panel_id))
    {
      terminal_user_authorized(panel_id);
      return;
    }
  }
  else
  {
    terminal_user_not_authorized(panel_id);
  }
}

static void terminal_task(void *pvParameters)
{
  (void)pvParameters;

  while (true)
  {
    uint32_t user_id;
    uint8_t panel_id = panel_get_request_from_buffer(&user_id);
    if (panel_id > 0)
    {
      terminal_user_identified(user_id, panel_id);
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

#ifdef DEVEL_BOARD
  static_cache_insert(static_cache_convert(0x7 << 24 | 814204));
  static_cache_insert(static_cache_convert(0x7 << 24 | 814199));
  static_cache_insert(static_cache_convert(0x7 << 24 | 814185));
  static_cache_insert(static_cache_convert(0x7 << 24 | 814190));
  static_cache_insert(static_cache_convert(0x7 << 24 | 7577396));
  static_cache_insert(static_cache_convert(0x7 << 24 | 7575678));
  static_cache_insert(static_cache_convert(0x7 << 24 | 7573990));
  static_cache_insert(static_cache_convert(0x7 << 24 | 7627526));
#endif
}

void terminal_reconfigure(panel_conf_t * panel_cfg, uint8_t panel_id)
{
  if (panel_id >= DOOR_ACC_PANEL_MAX_COUNT) return;

  portENTER_CRITICAL();

  if (panel_cfg != NULL)
  {
    memcpy(&acc_panel[panel_id], panel_cfg, sizeof(panel_conf_t));

    //reconfigure interface to card reader
    if (acc_panel[panel_id].acc_panel_on)
    {
      panel_init(panel_id);
    }
    else
    {
      //disable interface
      panel_deinit(panel_id);
    }
  }
  else
  {
    panel_init(panel_id);
  }

  portEXIT_CRITICAL();
}
