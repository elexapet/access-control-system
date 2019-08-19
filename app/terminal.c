/*
 *  Access control system panel
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

// Cache entry mapping
typedef cache_item_t term_cache_item_t; // 4B

enum term_cache_panel
{
  cache_panel_none = 0,
  cache_panel_door_A = 1,
  cache_panel_door_B = 2,
  cache_panel_all = 3
};


static inline uint8_t map_panel_id_to_cache(uint8_t panel_id)
{
  return (panel_id == ACC_PANEL_A ? cache_panel_door_A : cache_panel_door_B);
}

static inline void terminal_user_authorized(uint8_t panel_id)
{
  DEBUGSTR("auth OK\n");
  panel_unlock(panel_id, BEEP_ON_SUCCESS, OK_LED_ON_SUCCESS);
}

static inline void terminal_user_not_authorized(uint8_t panel_id)
{
  (void)panel_id;
  DEBUGSTR("auth FAIL\n");
}

/* ROM CCAN driver callback functions prototypes */
/*****************************************************************************
**
** Description:     CAN receive callback.
**            Function is executed by the Callback handler after
**            a CAN message has been received
**
** Parameters:      msg_obj_num. Contains the number of the message object
**            that triggered the CAN receive callback.
** Returned value:    None
*****************************************************************************/
void term_can_recv(uint8_t msg_obj_num);
/*****************************************************************************
**
** Description:     CAN transmit callback.
**            Function is executed by the Callback handler after
**            a CAN message has been transmitted
**
** Parameters:      msg_obj_num. Contains the number of the message object
**            that triggered the CAN transmit callback.
** Returned value:    None
*****************************************************************************/
void term_can_send(uint8_t msg_obj_num)
{
  (void)msg_obj_num;
}
/*****************************************************************************
**
** Description:     CAN error callback.
**            Function is executed by the Callback handler after
**            an error has occured on the CAN bus
**
** Parameters:      error_info. Contains the error code
**            that triggered the CAN error callback.
** Returned value:    None
*****************************************************************************/
void term_can_error(uint32_t error_info)
{
  (void)error_info;
}

// This is called from ISR -> do not block
void term_can_recv(uint8_t msg_obj_num)
{
  CCAN_MSG_OBJ_T msg_obj;
  /* Determine which CAN message has been received */
  msg_obj.msgobj = msg_obj_num;
  /* Now load up the msg_obj structure with the CAN message */
  LPC_CCAN_API->can_receive(&msg_obj);

  msg_head_t head;
  head.scalar = msg_obj.mode_id;

  uint8_t panel_id;

  // get target door if message is for us
  if (msg_obj.msgobj == ACS_MSGOBJ_RECV_DOOR_A) //TODO pouzit dva MSG OBJ pro A a B
  {
    DEBUGSTR("got CAN msg");

    if (head.dst == ACC_PANEL_A_ADDR)
    {
      panel_id = ACC_PANEL_A;
      DEBUGSTR("for door A\n");
    }
    else if (head.dst == ACC_PANEL_B_ADDR)
    {
      panel_id = ACC_PANEL_B;
      DEBUGSTR("for door B\n");
    }
    else return;
  }
  else return;

  // continue deducing action and execute it
  if (head.fc == FC_USER_AUTH_RESP)
  {
    terminal_user_authorized(panel_id);

    #if CACHING_ENABLED
      uint32_t user_id;
      uint8_t len = msg_obj.dlc > sizeof(user_id) ? sizeof(user_id) : msg_obj.dlc;
      memcpy(&user_id, msg_obj.data, len);
      term_cache_item_t user;
      user.key = user_id;
      user.value = map_panel_id_to_cache(panel_id);
      static_cache_insert(user);
    #endif
  }
  else if (head.fc == FC_USER_NOT_AUTH_RESP)
  {
    terminal_user_not_authorized(panel_id);

    #if CACHING_ENABLED
      uint32_t user_id;
      uint8_t len = msg_obj.dlc >= sizeof(user_id) ? sizeof(user_id) : msg_obj.dlc;
      memcpy(&user_id, msg_obj.data, len);
      term_cache_item_t user;
      user.key = user_id;
      user.value = cache_panel_none;
      static_cache_insert(user);
    #endif
  }
  else if (head.fc == FC_PANEL_CTRL)
  {
    switch (msg_obj.data[0])
    {
      case PANEL_CTRL_DATA_DEF:
        DEBUGSTR("mode DEF\n");
        panel_conf[panel_id].mode = PANEL_MODE_DEF;
        break;
      case PANEL_CTRL_DATA_UNLCK:
        DEBUGSTR("cmd UNLOCK\n");
        terminal_user_authorized(panel_id);
        break;
      case PANEL_CTRL_DATA_LOCK:
        DEBUGSTR("mode LOCK\n");
        panel_conf[panel_id].mode = PANEL_MODE_LOCKED;
        break;
      case PANEL_CTRL_DATA_LEARN:
        DEBUGSTR("mode LEARN\n");
        panel_conf[panel_id].mode = PANEL_MODE_LEARN;
        break;
      case PANEL_CTRL_DATA_CLR_CACHE:
        DEBUGSTR("cmd CLR CACHE\n");
#if CACHING_ENABLED
        static_cache_reset();
#endif
        break;
      default:
        break;
    }
  }
  else return;
}

static void terminal_register_user(uint32_t user_id, uint8_t panel_id)
{
  // Prepare msg head to send request on CAN
  msg_head_t head;
  head.scalar = CAN_MSGOBJ_EXT;
  head.prio = PRIO_NEW_USER;
  head.fc = FC_NEW_USER;
  head.dst = ACS_MSTR_FIRST_ADDR;

  if (panel_id == ACC_PANEL_A)
  {
    head.src = ACC_PANEL_A_ADDR;
    CAN_send_once(ACS_MSGOBJ_SEND_DOOR_A, head.scalar, (void *)&user_id, sizeof(user_id));
  }
  else if (panel_id == ACC_PANEL_B)
  {
    head.src = ACC_PANEL_B_ADDR;
    CAN_send_once(ACS_MSGOBJ_SEND_DOOR_B, head.scalar, (void *)&user_id, sizeof(user_id));
  }
}

static void terminal_request_auth(uint32_t user_id, uint8_t panel_id)
{
  // Prepare msg head to send request on CAN
  msg_head_t head;
  head.scalar = CAN_MSGOBJ_EXT;
  head.prio = PRIO_USER_AUTH_REQ;
  head.fc = FC_USER_AUTH_REQ;
  head.dst = ACS_MSTR_FIRST_ADDR;

  if (panel_id == ACC_PANEL_A)
  {
    head.src = ACC_PANEL_A_ADDR;
    CAN_send_once(ACS_MSGOBJ_SEND_DOOR_A, head.scalar, (void *)&user_id, sizeof(user_id));
  }
  else if (panel_id == ACC_PANEL_B)
  {
    head.src = ACC_PANEL_B_ADDR;
    CAN_send_once(ACS_MSGOBJ_SEND_DOOR_B, head.scalar, (void *)&user_id, sizeof(user_id));
  }
}


static void terminal_user_identified(uint32_t user_id, uint8_t panel_id)
{
#if CACHING_ENABLED
  term_cache_item_t user = {.key = user_id};
#endif

  if (panel_id < DOOR_ACC_PANEL_COUNT)
  {
    if (panel_conf[panel_id].mode == PANEL_MODE_LOCKED)
    {
      terminal_user_not_authorized(panel_id);
    }
    else if (panel_conf[panel_id].mode == PANEL_MODE_LEARN)
    {
      terminal_register_user(user_id, panel_id);
    }
#if CACHING_ENABLED
    else if (static_cache_get(&user))
    {
      if (map_panel_id_to_cache(panel_id) & user.value)
      {
        terminal_user_authorized(panel_id);
      }
    }
#endif
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
  //init CAN comm
  /* CAN Callback Functions of on-chip drivers */
  CCAN_CALLBACKS_T term_can_callbacks =
  {
    term_can_recv,    /* callback for any message received CAN frame which ID matches with any of the message objects' masks */
    term_can_send,    /* callback for every transmitted CAN frame */
    term_can_error,   /* callback for CAN errors */
    NULL,           /* callback for expedited read access (not used) */
    NULL,           /* callback for expedited write access (not used) */
    NULL,           /* callback for segmented read access (not used) */
    NULL,           /* callback for segmented write access (not used) */
    NULL,           /* callback for fall-back SDO handler (not used) */
  };

  CAN_init(&term_can_callbacks, CAN_BAUD_RATE);
  CAN_recv_filter_set_eff(ACS_MSGOBJ_RECV_DOOR_A);
  //CAN_recv_filter_set(ACS_MSGOBJ_RECV_DOOR_A, ACC_PANEL_A_ADDR << ACS_DST_ADDR_OFFSET, ACS_DST_ADDR_MASK);
  //CAN_recv_filter_set(ACS_MSGOBJ_RECV_DOOR_B, ACC_PANEL_B_ADDR << ACS_DST_ADDR_OFFSET, ACS_DST_ADDR_MASK);

  for (size_t id = 0; id < DOOR_ACC_PANEL_COUNT; ++id)
  {
    if (panel_conf[id].enabled) terminal_reconfigure(NULL, id);
  }

  xTaskCreate(terminal_task, "term_tsk", configMINIMAL_STACK_SIZE + 128, NULL, (tskIDLE_PRIORITY + 1UL), NULL);

#ifdef DEVEL_BOARD
  static_cache_insert(static_cache_convert(7632370, 3));
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
    if (panel_conf[panel_id].enabled)
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
