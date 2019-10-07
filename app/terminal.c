/*
 *  Access control system terminal
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
#include "acs_can_protocol.h"
#include <stdio.h>
#include <string.h>

// Cache entry type mapping our type
typedef cache_item_t term_cache_item_t; // 4B

// Used cache values
enum term_cache_reader
{
  cache_reader_none = 0,
  cache_reader_A = 1,
  cache_reader_B = 2,
  cache_reader_all = 3
};

// Address for currently active master
static uint16_t _act_master = ACS_RESERVED_ADDR;

// Flag for master alive broadcast timeout
static bool _master_timeout = true;

// Timer for master timeout
// Timer handle
static TimerHandle_t _act_timer = NULL;
// Timer ID
static const uint32_t _act_timer_id = TERMINAL_TIMER_ID;


static inline uint8_t map_reader_idx_to_cache(uint8_t reader_idx)
{
  return (reader_idx == ACS_READER_A_IDX ? cache_reader_A : cache_reader_B);
}

static inline void terminal_user_authorized(uint8_t reader_idx)
{
  DEBUGSTR("auth OK\n");
  reader_unlock(reader_idx, BEEP_ON_SUCCESS, OK_LED_ON_SUCCESS);
}

static inline void terminal_user_not_authorized(uint8_t reader_id)
{
  (void)reader_id;
  DEBUGSTR("auth FAIL\n");
}

static void _timer_callback(TimerHandle_t pxTimer)
{
  configASSERT(pxTimer);

  // Which timer expired
  uint32_t id = (uint32_t) pvTimerGetTimerID(pxTimer);

  if (id == _act_timer_id)
  {
    // Active master timeout after T = 2 * MASTER_ALIVE_TIMEOUT
    portENTER_CRITICAL();
    if (_master_timeout == true)
    {
      _act_master = ACS_RESERVED_ADDR;
      Board_LED_Set(BOARD_LED_STATUS, false);
    }
    _master_timeout = true;
    portEXIT_CRITICAL();
  }
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

  acs_msg_head_t head;
  head.scalar = msg_obj.mode_id;

  uint8_t reader_idx;

  // get target door if message is for us
  if (msg_obj.msgobj == ACS_MSGOBJ_RECV_DOOR_A)
  {
    reader_idx = ACS_READER_A_IDX;
    DEBUGSTR("for door A\n");
  }
  else if (msg_obj.msgobj == ACS_MSGOBJ_RECV_DOOR_B)
  {
    reader_idx = ACS_READER_B_IDX;
    DEBUGSTR("for door B\n");
  }
  else if (msg_obj.msgobj == ACS_MSGOBJ_RECV_BCAST)
  {
    //broadcast message
    if (head.fc == FC_ALIVE &&
        head.src >= ACS_MSTR_FIRST_ADDR &&
        head.src <= ACS_MSTR_LAST_ADDR)
    {
      // update master address if timeout occurred
      portENTER_CRITICAL();
      if (_master_timeout == true)
      {
        _act_master = head.src;
        Board_LED_Set(BOARD_LED_STATUS, true);
      }
      _master_timeout = false;
      portEXIT_CRITICAL();
      DEBUGSTR("master alive\n");
    }
    return;
  }
  else return;

  // stop processing if card reader not configured
  if (reader_idx >= ACS_READER_COUNT) return;

  // continue deducing action and execute it
  if (head.fc == FC_USER_AUTH_RESP)
  {
    terminal_user_authorized(reader_idx);

    #if CACHING_ENABLED
      uint32_t user_id;
      uint8_t len = msg_obj.dlc > sizeof(user_id) ? sizeof(user_id) : msg_obj.dlc;
      memcpy(&user_id, msg_obj.data, len);
      term_cache_item_t user;
      user.key = user_id;
      user.value = map_reader_idx_to_cache(reader_idx);
      static_cache_insert(user);
    #endif
  }
  else if (head.fc == FC_USER_NOT_AUTH_RESP)
  {
    terminal_user_not_authorized(reader_idx);

    #if CACHING_ENABLED
      uint32_t user_id;
      uint8_t len = msg_obj.dlc >= sizeof(user_id) ? sizeof(user_id) : msg_obj.dlc;
      memcpy(&user_id, msg_obj.data, len);
      term_cache_item_t user;
      user.key = user_id;
      user.value = cache_reader_none;
      static_cache_insert(user);
    #endif
  }
  else if (head.fc == FC_DOOR_CTRL)
  {
    switch (msg_obj.data[0])
    {
      case DATA_DOOR_CTRL_MODE_DEF:
        DEBUGSTR("mode DEF\n");
        reader_conf[reader_idx].mode = READER_MODE_DEF;
        break;
      case DATA_DOOR_CTRL_UNLCK:
        DEBUGSTR("cmd UNLOCK\n");
        terminal_user_authorized(reader_idx);
        break;
      case DATA_DOOR_CTRL_LOCK:
        DEBUGSTR("mode LOCK\n");
        reader_conf[reader_idx].mode = READER_MODE_LOCKED;
        break;
      case DATA_DOOR_CTRL_LEARN:
        DEBUGSTR("mode LEARN\n");
        reader_conf[reader_idx].mode = READER_MODE_LEARN;
        break;
      case DATA_DOOR_CTRL_CLR_CACHE:
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

static void terminal_register_user(uint32_t user_id, uint8_t reader_idx)
{
  //check if master online
  if (_act_master == ACS_RESERVED_ADDR)
  {
    DEBUGSTR("master offline\n");
    return;
  }

  // Prepare msg head to send request on CAN
  acs_msg_head_t head;
  head.scalar = CAN_MSGOBJ_EXT;
  head.prio = PRIO_NEW_USER;
  head.fc = FC_NEW_USER;
  head.dst = _act_master;

  if (reader_idx == ACS_READER_A_IDX)
  {
    head.src = get_reader_a_addr();
    CAN_send_once(ACS_MSGOBJ_SEND_DOOR_A, head.scalar, (void *)&user_id, sizeof(user_id));
  }
  else if (reader_idx == ACS_READER_B_IDX)
  {
    head.src = get_reader_b_addr();
    CAN_send_once(ACS_MSGOBJ_SEND_DOOR_B, head.scalar, (void *)&user_id, sizeof(user_id));
  }
}

static void terminal_request_auth(uint32_t user_id, uint8_t reader_idx)
{
  //check if master online
  if (_act_master == ACS_RESERVED_ADDR)
  {
    DEBUGSTR("master off-line\n");
    return;
  }

  // Prepare msg head to send request on CAN
  acs_msg_head_t head;
  head.scalar = CAN_MSGOBJ_EXT;
  head.prio = PRIO_USER_AUTH_REQ;
  head.fc = FC_USER_AUTH_REQ;
  head.dst = _act_master;

  if (reader_idx == ACS_READER_A_IDX)
  {
    head.src = get_reader_a_addr();
    CAN_send_once(ACS_MSGOBJ_SEND_DOOR_A, head.scalar, (void *)&user_id, sizeof(user_id));
  }
  else if (reader_idx == ACS_READER_B_IDX)
  {
    head.src = get_reader_b_addr();
    CAN_send_once(ACS_MSGOBJ_SEND_DOOR_B, head.scalar, (void *)&user_id, sizeof(user_id));
  }
}


static void terminal_user_identified(uint32_t user_id, uint8_t reader_idx)
{
#if CACHING_ENABLED
  term_cache_item_t user = {.key = user_id};
#endif

  if (reader_idx < ACS_READER_COUNT)
  {
    if (reader_conf[reader_idx].mode == READER_MODE_LOCKED)
    {
      terminal_user_not_authorized(reader_idx);
    }
    else if (reader_conf[reader_idx].mode == READER_MODE_LEARN)
    {
      terminal_register_user(user_id, reader_idx);
    }
#if CACHING_ENABLED
    else if (static_cache_get(&user))
    {
      if (map_reader_idx_to_cache(reader_idx) & user.value)
      {
        terminal_user_authorized(reader_idx);
      }
    }
#endif
    else
    {
      terminal_request_auth(user_id, reader_idx);
    }
  }
}

static void terminal_task(void *pvParameters)
{
  (void)pvParameters;

  configASSERT(xTimerStart(_act_timer, 0));

  while (true)
  {
    uint32_t user_id;
    uint8_t reader_idx = reader_get_request_from_buffer(&user_id);
    if (reader_idx < ACS_READER_COUNT)
    {
      DEBUGSTR("user identified\n");
      terminal_user_identified(user_id, reader_idx);
    }
  }
}

void terminal_init(void)
{
  // init config for terminal
  configASSERT(terminal_config_init());

  // assign CAN callback functions of on-chip drivers
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

  // init CAN driver
  CAN_init(&term_can_callbacks, CAN_BAUD_RATE);

  // CAN msg filter for door A
  CAN_recv_filter(ACS_MSGOBJ_RECV_DOOR_A,
                  get_reader_a_addr() << ACS_DST_ADDR_OFFSET,
                  ACS_DST_ADDR_MASK, true);
  // CAN msg filter for door B
  CAN_recv_filter(ACS_MSGOBJ_RECV_DOOR_B,
                  get_reader_b_addr() << ACS_DST_ADDR_OFFSET,
                  ACS_DST_ADDR_MASK, true);
  // CAN msg filter for broadcast
  CAN_recv_filter(ACS_MSGOBJ_RECV_BCAST,
                  ACS_BROADCAST_ADDR << ACS_DST_ADDR_OFFSET,
                  ACS_DST_ADDR_MASK, true);

  // initialize card readers
  for (size_t id = 0; id < ACS_READER_COUNT; ++id)
  {
    if (reader_conf[id].enabled) terminal_reconfigure(NULL, id);
  }

  // create timer for master alive status timeout
  _act_timer = xTimerCreate("MAT", (ACS_MASTER_ALIVE_TIMEOUT_MS / portTICK_PERIOD_MS),
               pdTRUE, (void *)_act_timer_id, _timer_callback);
  configASSERT(_act_timer);

  // create task for terminal loop
  xTaskCreate(terminal_task, "term_tsk", configMINIMAL_STACK_SIZE + 128, NULL, (tskIDLE_PRIORITY + 1UL), NULL);
}

void terminal_reconfigure(reader_conf_t * reader_cfg, uint8_t reader_idx)
{
  if (reader_idx >= ACS_READER_COUNT) return;

  portENTER_CRITICAL();

  if (reader_cfg != NULL)
  {
    memcpy(&reader_conf[reader_idx], reader_cfg, sizeof(reader_conf_t));

    //reconfigure interface to card reader
    if (reader_conf[reader_idx].enabled)
    {
      reader_init(reader_idx);
      DEBUGSTR("reader enabled\n");
    }
    else
    {
      //disable interface
      reader_deinit(reader_idx);
      DEBUGSTR("reader disabled\n");
    }
  }
  else
  {
    reader_init(reader_idx);
  }

  portEXIT_CRITICAL();
}
