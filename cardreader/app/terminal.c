/**
 *  @file
 *  @brief Terminal client for access control system (ACS).
 *
 *				 Contains main processing loop.
 *
 *  @author Petr Elexa
 *  @see LICENSE
 *
 */

#include "terminal.h"
#include "board.h"
#include "weigand.h"
#include "watchdog.h"
#include "static_cache.h"
#include "FreeRTOS.h"
#include "task.h"
#include "stream_buffer.h"
#include "can/can_term_driver.h"
#include "acs_can_protocol.h"
#include <stdio.h>
#include <string.h>

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

// How long to wait for user request.
static const uint16_t USER_REQUEST_WAIT_MS = 400;
// This is minimal period between each request.
// Also controls intensity of door status messages (sent each SEND_DOOR_STATUS_RATE*period).
static const uint16_t USER_REQUEST_MIN_PERIOD_MS = 1000;
// Door status update rate (period multiplier)
static const uint16_t SEND_DOOR_STATUS_RATE = 5;

// Cache entry type mapping to our type.
typedef cache_item_t term_cache_item_t;  // 4 bytes

// Available cache values.
enum term_cache_reader
{
  cache_reader_none = 0,
  cache_reader_A = 1,
  cache_reader_B = 2,
  cache_reader_all = 3
};

// Address for currently active master.
static uint16_t _act_master = ACS_RESERVED_ADDR;

// Flag for master alive broadcast timeout.
static bool _master_timeout = true;

// Timer handle for master timeout.
static TimerHandle_t _act_timer = NULL;

// Timer ID for master timeout.
static const uint32_t _act_timer_id = TERMINAL_TIMER_ID;

// Last door open(true) / close(false) status
static bool _last_door_state[ACS_READER_MAXCOUNT] = {false, false};

// Signal request to clear cache.
static bool _cache_clear_req = false;

/*****************************************************************************
 * Private functions
 ****************************************************************************/

// Map reader index to correct cache value.
static inline uint8_t map_reader_idx_to_cache(uint8_t reader_idx)
{
  return (reader_idx == ACS_READER_A_IDX ? cache_reader_A : cache_reader_B);
}

static inline void _terminal_user_authorized(uint8_t reader_idx)
{
  DEBUGSTR("auth OK\n");
  reader_unlock(reader_idx, BEEP_ON_SUCCESS, OK_LED_ON_SUCCESS);
}

static inline void __terminal_user_not_authorized(uint8_t reader_idx)
{
  (void)reader_idx;
  DEBUGSTR("auth FAIL\n");
}

// Callback for timer dedicated to master master activity.
static void _timer_callback(TimerHandle_t pxTimer)
{
  configASSERT(pxTimer);

  // Which timer expired.
  uint32_t id = (uint32_t) pvTimerGetTimerID(pxTimer);

  if (id == _act_timer_id)
  {
    // Active master timeout after T = 2 * MASTER_ALIVE_TIMEOUT.
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

/*****************************************************************************
 * Public functions
 ****************************************************************************/

void term_can_error(uint32_t error_info)
{
  if (error_info == CAN_ERROR_BOFF) // if bus off do reset by WDG timeout
  {
  	WDT_Feed();
    configASSERT(false);
  }
}

void term_can_send(uint8_t msg_obj_num)
{
  (void)msg_obj_num;
}

// This is called from interrupt. We must not block.
void term_can_recv(uint8_t msg_obj_num)
{
  CCAN_MSG_OBJ_T msg_obj;
  // Determine which CAN message has been received.
  msg_obj.msgobj = msg_obj_num;
  // Now load up the msg_obj structure with the CAN message.
  LPC_CCAN_API->can_receive(&msg_obj);

  acs_msg_head_t head;
  head.scalar = msg_obj.mode_id;

  uint8_t reader_idx;

  // Get target door if message is for us.
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
    // Broadcast message.
    if (head.fc == FC_ALIVE &&
        head.src >= ACS_MSTR_FIRST_ADDR &&
        head.src <= ACS_MSTR_LAST_ADDR)
    {
      // Update master address if timeout occurred.
      portENTER_CRITICAL();
      if (_master_timeout == true)
      {
        _act_master = head.src;
        Board_LED_Set(BOARD_LED_STATUS, true);
        _cache_clear_req = true;
      }
      _master_timeout = false;
      portEXIT_CRITICAL();
      DEBUGSTR("master alive\n");
    }
    return;
  }
  else return;

  // Stop processing if card reader not configured.
  if (reader_idx >= ACS_READER_MAXCOUNT || !reader_conf[reader_idx].enabled) return;

  // Continue deducing action and execute it.
  if (head.fc == FC_USER_AUTH_RESP)
  {
    _terminal_user_authorized(reader_idx);

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
    __terminal_user_not_authorized(reader_idx);

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
  else if (head.fc == FC_LEARN_USER_OK)
  {
      reader_signal_to_user(reader_idx, BEEP_ON_SUCCESS);
  }
  else if (head.fc == FC_DOOR_CTRL)
  {
    switch (msg_obj.data[0])
    {
      case DATA_DOOR_CTRL_REMOTE_UNLCK:
        DEBUGSTR("cmd UNLOCK\n");
        reader_unlock(reader_idx, BEEP_ON_SUCCESS, OK_LED_ON_SUCCESS);
        break;
      case DATA_DOOR_CTRL_CLR_CACHE:
        DEBUGSTR("cmd CLR CACHE\n");
#if CACHING_ENABLED
        _cache_clear_req = true;
#endif
        break;
      case DATA_DOOR_CTRL_NORMAL_MODE:
        DEBUGSTR("cmd NORMAL MODE\n");
        reader_conf[reader_idx].learn_mode = false;
        reader_signal_to_user(reader_idx, BEEP_ON_SUCCESS);
        break;
      case DATA_DOOR_CTRL_LEARN_MODE:
        DEBUGSTR("cmd LEARN MODE\n");
        reader_conf[reader_idx].learn_mode = true;
        reader_signal_to_user(reader_idx, BEEP_ON_SUCCESS);
        break;
      default:
        break;
    }
  }
  else return;
}

// Send door status update to server
static void terminal_send_door_status(uint8_t reader_idx, bool is_open)
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
  head.prio = PRIO_DOOR_STATUS;
  head.fc = FC_DOOR_STATUS;
  head.dst = _act_master;

  uint8_t status = (is_open ? DATA_DOOR_STATUS_OPEN : DATA_DOOR_STATUS_CLOSED);

  if (reader_idx == ACS_READER_A_IDX)
  {
    head.src = get_reader_a_addr();
    CAN_send_once(ACS_MSGOBJ_SEND_DOOR_A, head.scalar, (void *)&status, sizeof(status));
  }
  else if (reader_idx == ACS_READER_B_IDX)
  {
    head.src = get_reader_b_addr();
    CAN_send_once(ACS_MSGOBJ_SEND_DOOR_B, head.scalar, (void *)&status, sizeof(status));
  }
}

// Send command to server that request authorization of user for given reader.
static void terminal_request_auth(uint32_t user_id, uint8_t reader_idx)
{
  // Check if master online.
  if (_act_master == ACS_RESERVED_ADDR)
  {
    DEBUGSTR("master off-line\n");
    return;
  }

  // Prepare message head to send request on CAN.
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

// Send command to server that request to learn a user for given reader.
static void terminal_request_user_learn(uint32_t user_id, uint8_t reader_idx)
{
  // Check if master online.
  if (_act_master == ACS_RESERVED_ADDR)
  {
    DEBUGSTR("master off-line\n");
    return;
  }

  // Prepare message head to send request on CAN.
  acs_msg_head_t head;
  head.scalar = CAN_MSGOBJ_EXT;
  head.prio = PRIO_LEARN_USER;
  head.fc = FC_LEARN_USER;
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

// Process user identification on a reader.
static void terminal_user_identified(uint32_t user_id, uint8_t reader_idx)
{
#if CACHING_ENABLED
  term_cache_item_t user = {.key = user_id};
#endif

  if (reader_idx < ACS_READER_MAXCOUNT && reader_conf[reader_idx].enabled)
  {
    if (reader_conf[reader_idx].learn_mode)
    {
      terminal_request_user_learn(user_id, reader_idx);
    }
#if CACHING_ENABLED
  	// Read from cache online if master is offline.
    else if ((_act_master == ACS_RESERVED_ADDR) && static_cache_get(&user))
    {
      if (map_reader_idx_to_cache(reader_idx) & user.value)
      {
        _terminal_user_authorized(reader_idx);
      }
    }
#else
    else
    {
      terminal_request_auth(user_id, reader_idx);
    }
#endif
  }
}

// Main loop in terminal processing task.
//
// Waked only when request is in the reader buffer or after timeout.
static void terminal_task(void *pvParameters)
{
  (void)pvParameters;

  // start timer for detecting master timeout
  configASSERT(xTimerStart(_act_timer, 0));

  uint8_t send_door_status = SEND_DOOR_STATUS_RATE;

  WDT_Feed(); // Feed HW watchdog

  while (true)
  {
    TickType_t begin_time = xTaskGetTickCount();

    // Service pending user requests for all readers
    for (size_t i = 0; i < ACS_READER_MAXCOUNT; ++i)
    {
      if (!reader_conf[i].enabled) continue;

      uint32_t user_id;
      uint8_t reader_idx = reader_get_request_from_buffer(&user_id, USER_REQUEST_WAIT_MS);

      if (reader_idx < ACS_READER_MAXCOUNT && reader_conf[reader_idx].enabled)
      {
        DEBUGSTR("user req\n");
        terminal_user_identified(user_id, reader_idx);
      }
    }

#if CACHING_ENABLED
    if (_cache_clear_req)
    {
    	_cache_clear_req = false;
      static_cache_reset();
    }
#endif

    // Calculate actual processing time.
		TickType_t proces_time = xTaskGetTickCount() - begin_time;

    // Protect against brute-force attack by limiting processing frequency.
    // Also controls frequency of door status update.
    if (proces_time < pdMS_TO_TICKS(USER_REQUEST_MIN_PERIOD_MS))
		{
      vTaskDelay(pdMS_TO_TICKS(USER_REQUEST_MIN_PERIOD_MS) - proces_time);
		}

    WDT_Feed(); // Feed HW watchdog

    if (send_door_status == SEND_DOOR_STATUS_RATE)
    {
      send_door_status = 0;

			// Check if door open/close state changed
			for (size_t idx = 0; idx < ACS_READER_MAXCOUNT; ++idx)
			{
				if (reader_conf[idx].enabled && (reader_is_door_open(idx) != _last_door_state[idx]))
				{
					DEBUGSTR("new door state\n");
					_last_door_state[idx] = !_last_door_state[idx];
					terminal_send_door_status(idx, _last_door_state[idx]);
				}
			}
    }
    send_door_status++;
  }
}

void terminal_init(void)
{
  // Initialize configuration for terminal.
  configASSERT(terminal_config_init());

  // Assign CAN callback functions of on-chip drivers.
  CCAN_CALLBACKS_T term_can_callbacks =
  {
    term_can_recv,  // Callback for any message received CAN frame which ID
                    // matches with any of the message objects' masks.
    term_can_send,  // Callback for every transmitted CAN frame.
    term_can_error, // Callback for CAN errors.
    NULL,           // Not used.
    NULL,           // Not used.
    NULL,           // Not used.
    NULL,           // Not used.
    NULL,           // Not used.
  };

  // Init CAN driver.
  CAN_init(&term_can_callbacks, CAN_BAUD_RATE);

  // CAN msg filter for door A.
  CAN_recv_filter(ACS_MSGOBJ_RECV_DOOR_A,
                  get_reader_a_addr() << ACS_DST_ADDR_OFFSET,
                  ACS_DST_ADDR_MASK, true);
  // CAN msg filter for door B.
  CAN_recv_filter(ACS_MSGOBJ_RECV_DOOR_B,
                  get_reader_b_addr() << ACS_DST_ADDR_OFFSET,
                  ACS_DST_ADDR_MASK, true);
  // CAN msg filter for broadcast.
  CAN_recv_filter(ACS_MSGOBJ_RECV_BCAST,
                  ACS_BROADCAST_ADDR << ACS_DST_ADDR_OFFSET,
                  ACS_DST_ADDR_MASK, true);

  // Initialize card readers.
  for (size_t id = 0; id < ACS_READER_MAXCOUNT; ++id)
  {
  	// Config only present CR
    if (reader_conf[id].enabled) terminal_reconfigure(NULL, id);
  }

  // Create timer for master alive status timeout.
  _act_timer = xTimerCreate("MAT", (ACS_MASTER_ALIVE_TIMEOUT_MS / portTICK_PERIOD_MS),
               pdTRUE, (void *)_act_timer_id, _timer_callback);
  configASSERT(_act_timer);

  // Create task for terminal loop.
  xTaskCreate(terminal_task, "term_tsk", configMINIMAL_STACK_SIZE + 128, NULL, (tskIDLE_PRIORITY + 1UL), NULL);
}

void terminal_reconfigure(reader_conf_t * reader_cfg, uint8_t reader_idx)
{
  if (reader_idx >= ACS_READER_MAXCOUNT) return;

  portENTER_CRITICAL(); // Effectively disables interrupts.

  if (reader_cfg != NULL)
  {
    memcpy(&reader_conf[reader_idx], reader_cfg, sizeof(reader_conf_t));

    // Reconfigure the reader instance.
    if (reader_conf[reader_idx].enabled)
    {
      reader_init(reader_idx);
      DEBUGSTR("reader enabled\n");
    }
    else
    {
      // Disable interface.
      reader_deinit(reader_idx);
      DEBUGSTR("reader disabled\n");
    }
  }
  else
  {
    // No new configuration given -- just init.
    reader_init(reader_idx);
  }

  portEXIT_CRITICAL();
}
